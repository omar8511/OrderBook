#include "orderbook.hpp"

#include <iostream>
#include <chrono>


OrderBook::OrderBook(const std::string& assetName) {
	std::cout << "Orderbook opened for asset: " << assetName << std::endl;	
	expiryThread = std::thread(&OrderBook::expiryHandler, this);
}

OrderBook::~OrderBook(){
	{
		std::lock_guard<std::mutex> lock(expiryMutex);
		shutdownFlag = true;	
	} // To avoid waking up the thread and then blocking it we keep the notify outside the scope of lock_guard
	expiryCV.notify_one();
	if(expiryThread.joinable()) expiryThread.join();
}

// Move constructor
OrderBook::OrderBook(OrderBook&& other) noexcept
	: level2Buy(std::move(other.level2Buy)),
	  level2Sell(std::move(other.level2Sell)),
	  metaDataMap(std::move(other.metaDataMap)),
	  expiryQueue(std::move(other.expiryQueue)),
	  shutdownFlag(other.shutdownFlag.load()),
	  expiryThread(std::move(other.expiryThread)) {
	// Reset the moved-from object
	other.shutdownFlag = true;
}

// Move assignment operator
OrderBook& OrderBook::operator=(OrderBook&& other) noexcept {
	if (this != &other) {
		// Clean up current resources
		{
			std::lock_guard<std::mutex> lock(expiryMutex);
			shutdownFlag = true;
		}
		expiryCV.notify_one();
		if(expiryThread.joinable()) expiryThread.join();
		
		// Move resources from other
		level2Buy = std::move(other.level2Buy);
		level2Sell = std::move(other.level2Sell);
		metaDataMap = std::move(other.metaDataMap);
		expiryQueue = std::move(other.expiryQueue);
		shutdownFlag = other.shutdownFlag.load();
		expiryThread = std::move(other.expiryThread);
		
		// Reset the moved-from object
		other.shutdownFlag = true;
	}
	return *this;
}

// Runs the thread until the next expiry or every 8 ms if queue is empty 

void OrderBook::expiryHandler(){ 
	std::unique_lock<std::mutex> lock(expiryMutex); // unique lock needed for wait since we can manually lock and unlock
	while(!shutdownFlag){
		std::vector<Order> expiredEntries;	
		uint32_t currentTime = static_cast<uint32_t>(
			std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::system_clock::now().time_since_epoch()
			).count()
			);
		while(!expiryQueue.empty() && expiryQueue.top().getExpiryTime() <= currentTime){
			const Order& order = expiryQueue.top().getOrder();
			expiryQueue.pop();
			expiredEntries.push_back(order);			
		}
		lock.unlock();
		for(const Order& order : expiredEntries){
			this->cancelOrder(order.getOrderId());
		}
		lock.lock();
		if (!expiryQueue.empty()) {
    			auto nextExpiry = std::chrono::seconds(expiryQueue.top().getExpiryTime());
    			expiryCV.wait_until(lock, std::chrono::system_clock::time_point(nextExpiry));
		} else {
    			expiryCV.wait_for(lock, std::chrono::milliseconds(8));
		}	
		if(shutdownFlag) break;	
	}
}



void OrderBook::cancelOrder(uint64_t orderId) {
	std::lock_guard lock(metaDataMutex);	
	auto metaIterator = metaDataMap.find(orderId);
	if(metaIterator == metaDataMap.end()) return;

	
	const Order& order = metaIterator->second;
	uint32_t price = order.getPrice();
	bool isBuy = order.getIsBuy();

	metaDataMap.erase(metaIterator);	

	if(isBuy){
		std::unique_lock lock(level2BuyMutex);			
		auto it = level2Buy.find(price);
		if (it == level2Buy.end()) {
    			// std::cout << "Order not found";
			return;
		}
		PriceLevel& buyLevel = it->second;
		buyLevel.removeOrder(orderId);
		if(buyLevel.isEmpty()){
			level2Buy.erase(it);
		}
	} else {
		std::unique_lock lock(level2SellMutex);	
		auto it = level2Sell.find(price);
		if (it == level2Sell.end()) {
    			// std::cout << "Order not found";
			return;
		}
		PriceLevel& sellLevel = it->second;
		sellLevel.removeOrder(orderId);
		if(sellLevel.isEmpty()){
			level2Sell.erase(it);
		}
	}
	// std::cout << "Cancelled Order: " <<  << std::endl;
}

std::string OrderBook::trackOrder(uint64_t orderId) {
	std::lock_guard lock(metaDataMutex);	
	auto metaIterator = metaDataMap.find(orderId);
	if(metaIterator == metaDataMap.end()) return "Order not found";
	const Order& order = metaIterator->second;
	return std::format("Fulfilled {} / {} @ {} USD", order.getOriginalQuantity() - order.getQuantity(), order.getOriginalQuantity(), order.getPrice());	
	
}

void OrderBook::modifyQuantity(uint64_t orderId, uint32_t newQuantity) {
	Order orderCopy;
	{
		std::lock_guard lock(metaDataMutex);	
		auto metaIterator = metaDataMap.find(orderId);
		if(metaIterator == metaDataMap.end()) std::cerr << "Order not found";
		orderCopy = metaIterator->second;
	}
	cancelOrder(orderCopy.getOrderId());
	orderCopy.setQuantity(newQuantity);
	placeOrder(orderCopy);
}

void OrderBook::modifyPrice(uint64_t orderId, uint32_t newPrice) {
	Order orderCopy;
	{
		std::lock_guard lock(metaDataMutex);	
		auto metaIterator = metaDataMap.find(orderId);
		if(metaIterator == metaDataMap.end()) std::cerr << "Order not found";
		orderCopy = metaIterator->second;
	}
	cancelOrder(orderCopy.getOrderId());
	orderCopy.setPrice(newPrice);
	placeOrder(orderCopy);
}


void OrderBook::modifyExpiryTime(uint64_t orderId, uint32_t newExpiryTime) {
	Order orderCopy;
	{
		std::lock_guard lock(metaDataMutex);	
		auto metaIterator = metaDataMap.find(orderId);
		if(metaIterator == metaDataMap.end()) std::cerr << "Order not found";
		orderCopy = metaIterator->second;
		if (orderCopy.getOrderType() != GTD) std::cerr << "Incorrect Order Type, cancel the order and place a new one";
	}
	cancelOrder(orderCopy.getOrderId());
	orderCopy.setExpiryTime(newExpiryTime);
	placeOrder(orderCopy);
}

void OrderBook::displayStats() {
	// Use separate read locks for thread safety
	
	std::cout << "\n=== Order Book Statistics ===\n";
	
	// Level 1 Data (Best Bid/Ask)
	std::shared_lock<std::shared_mutex> buyLock(level2BuyMutex);
	auto buyBegin = level2Buy.begin();
	auto buyEnd = level2Buy.end();
	buyLock.unlock();
	
	std::shared_lock<std::shared_mutex> sellLock(level2SellMutex);
	auto sellBegin = level2Sell.begin();
	auto sellEnd = level2Sell.end();
	sellLock.unlock();
	
	std::cout << "\n--- Level 1 Data ---\n";
	if (buyBegin != buyEnd) {
		// Get best bid (highest price in buy side)
		auto bestBid = level2Buy.rbegin();
		std::cout << "Best Bid: $" << bestBid->first << " (Qty: " << bestBid->second.getQuantity() << ")\n";
	} else {
		std::cout << "Best Bid: No buy orders\n";
	}
	
	if (sellBegin != sellEnd) {
		// Get best ask (lowest price in sell side)
		std::cout << "Best Ask: $" << sellBegin->first << " (Qty: " << sellBegin->second.getQuantity() << ")\n";
	} else {
		std::cout << "Best Ask: No sell orders\n";
	}
	
	// Calculate spread
	if (buyBegin != buyEnd && sellBegin != sellEnd) {
		auto bestBid = level2Buy.rbegin();
		uint32_t spread = sellBegin->first - bestBid->first;
		std::cout << "Spread: $" << spread << "\n";
	}
	
	// Level 2 Data (Order Book Depth)
	std::cout << "\n--- Level 2 Data ---\n";
	
	// Buy side depth
	std::cout << "Buy Side Levels: " << std::distance(buyBegin, buyEnd) << "\n";
	uint32_t totalBuyQuantity = 0;
	for (auto it = buyBegin; it != buyEnd; ++it) {
		totalBuyQuantity += it->second.getQuantity();
	}
	std::cout << "Total Buy Quantity: " << totalBuyQuantity << "\n";
	
	// Sell side depth  
	std::cout << "Sell Side Levels: " << std::distance(sellBegin, sellEnd) << "\n";
	uint32_t totalSellQuantity = 0;
	for (auto it = sellBegin; it != sellEnd; ++it) {
		totalSellQuantity += it->second.getQuantity();
	}
	std::cout << "Total Sell Quantity: " << totalSellQuantity << "\n";
	
	// Order metadata
	std::cout << "\n--- Order Metadata ---\n";
	std::lock_guard<std::mutex> metaLock(metaDataMutex);
	std::cout << "Total Active Orders: " << metaDataMap.size() << "\n";
	
	std::cout << "\n==========================================\n";
}







std::pair<uint32_t, std::vector<uint64_t>> OrderBook::fulfillBuy(Order &order){
	std::vector<uint64_t> removedOrders;
	removedOrders.reserve(100);  // Pre-allocate to prevent reallocation			
	std::unique_lock lock(level2SellMutex);
	if(order.getOrderType() == FOK){
		auto itSim = level2Sell.begin();		
		uint32_t quantityAvailable = 0;
		while(itSim != level2Sell.end()){
			uint32_t price = itSim->first;
			PriceLevel &sellLevel = itSim->second;
			if(price > order.getPrice()) {
				break; // std::map stores in order so if this one is bigger then so is the next one
			}
			quantityAvailable += sellLevel.getQuantity();
			++itSim;
	}
		if(quantityAvailable < order.getQuantity()) return { order.getQuantity(), {}};
	}
	auto it = level2Sell.begin();
	while(it != level2Sell.end()){
		uint32_t price = it->first;
		PriceLevel &sellLevel = it->second;
		if(price > order.getPrice() && order.getOrderType() != MKT){
			break;
		}
		uint32_t fulfilledQuantity = sellLevel.placeOrder(order, removedOrders);
	
		if(fulfilledQuantity > 0){
			order.setQuantity(order.getQuantity() - fulfilledQuantity);
			order.setPriceImprovement(order.getPriceImprovement() + ((order.getOrderType() != MKT) ? (order.getPrice() - price) * fulfilledQuantity : 0));

		}
		if(sellLevel.isEmpty()){
			it = level2Sell.erase(it); // Removes it and returns next valid iterator
		} else {
			++it;
		}
		if(order.getQuantity() == 0){
			// std::cout << "Fulfilled order number: " << order.; // We update order tags here then i guess 
			break;			
		} 	
	}
	return {order.getQuantity(), removedOrders};
}


std::pair<uint32_t, std::vector<uint64_t>> OrderBook::fulfillSell(Order &order){
	std::vector<uint64_t> removedOrders;
	removedOrders.reserve(100);  // Pre-allocate to prevent reallocation	
	std::unique_lock lock(level2BuyMutex);	
	if(order.getOrderType() == FOK){
		auto itSim = level2Buy.rbegin();	
		uint32_t quantityAvailable = 0;
		while(itSim != level2Buy.rend()){
			uint32_t price = itSim->first;
			PriceLevel &buyLevel = itSim->second;
			if(price < order.getPrice()) {
				break; 			
			}
			quantityAvailable += buyLevel.getQuantity();
			++itSim;
		
	}
		if(quantityAvailable < order.getQuantity()) return { order.getQuantity(), {}};
	}
	auto it = level2Buy.rbegin();
	while(it != level2Buy.rend()){
		uint32_t price = it->first;
		PriceLevel &buyLevel = it->second;
		if(price < order.getPrice() && order.getOrderType() != MKT){
			break;
		}
		uint32_t fulfilledQuantity = buyLevel.placeOrder(order, removedOrders);
		if(fulfilledQuantity > 0){
			order.setQuantity(order.getQuantity() - fulfilledQuantity);
			order.setPriceImprovement(order.getPriceImprovement() + ((order.getOrderType() != MKT) ? (price - order.getPrice()) * fulfilledQuantity : 0));
		}
		if(buyLevel.isEmpty()){
			auto eraseIt = std::prev(it.base()); // it.base gets the forward iterator pointing to the next level
			it = std::reverse_iterator(level2Buy.erase(eraseIt));
		} else {
			++it; // Moves to next but follows the reverse order
		}
		if(order.getQuantity() == 0){
			// std::cout << "Fulfilled order number: " << order.;
			break;			
		} 	
	}
	return {order.getQuantity(), removedOrders};
}




void OrderBook::placeOrder(Order &order) {
	if(order.getOrderType() == GTD){
		{
			std::lock_guard<std::mutex> lock(expiryMutex);
			expiryQueue.push(ExpiryEntry(order.getExpiryTime(), order));
		}
		expiryCV.notify_one();
	}
	if(order.getIsBuy()){ 
		auto [remainingQuantity, removedOrders] = this->fulfillBuy(order);
		bool unfulfilledFOK = (order.getOrderType() == FOK && remainingQuantity > 0);
		{
            		std::unique_lock metaLock(metaDataMutex);
           		if(order.getOrderType() != IOC && !unfulfilledFOK) metaDataMap[order.getOrderId()] = order;
			for (uint64_t orderId : removedOrders){
				metaDataMap.erase(orderId);
			} 
        	}

		//std::cout << "Fulfilled buy order: " << order.getOrderId() << "unfilled quantity: " << order.getOriginalQuantity() - remainingQuantity << std::endl;
		if(order.getOrderType() == IOC) return;
		if(unfulfilledFOK) return;
		if(remainingQuantity > 0) {
			// std::cout << "Not enough liquidity to fulfil order ID: " << order. << std::endl;
			// std::cout << "Unfulfilled quantity: " << remainingQuantity << std::endl;
			if(order.getOrderType() != MKT){
				std::unique_lock lock(level2BuyMutex);
				auto it = level2Buy.find(order.getPrice());
				if (it == level2Buy.end()) {
    					auto [newIt, inserted] = level2Buy.emplace(order.getPrice(), PriceLevel(order.getPrice()));
    					it = newIt;
				}
				PriceLevel& buyLevel = it->second;
				buyLevel.addOrder(order);
			
			}
		}	
	} else {
		auto [remainingQuantity, removedOrders] = this->fulfillSell(order);
		bool unfulfilledFOK = (order.getOrderType() == FOK && remainingQuantity > 0);		
		{
            		std::unique_lock metaLock(metaDataMutex);
           		if(order.getOrderType() != IOC && !unfulfilledFOK) metaDataMap[order.getOrderId()] = order;
			for (uint64_t orderId : removedOrders){
				metaDataMap.erase(orderId);
			} 
        	}		
		//std::cout << "Fulfilled sell order: " << order.getOrderId() << " quantity: " << remainingQuantity << std::endl;			
		if(order.getOrderType() == IOC) return;	
		if(unfulfilledFOK) return;
		if(remainingQuantity > 0) {
			// std::cout << "Not enough liquidity to fulfill order ID: " << order. << std::endl;
			// std::cout << "Unfulfilled quantity: " << remainingQuantity << std::endl;
			if(order.getOrderType() != MKT){
				std::unique_lock lock(level2SellMutex);	
				auto it = level2Sell.find(order.getPrice());
				if (it == level2Sell.end()) {
    					auto [newIt, inserted] = level2Sell.emplace(order.getPrice(), PriceLevel(order.getPrice()));
    					it = newIt;
				}
				PriceLevel& sellLevel = it->second;
				sellLevel.addOrder(order);
			}
		}			
	}	
}

