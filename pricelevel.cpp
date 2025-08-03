#include "pricelevel.hpp"



// This function returns the unfilled quantity

uint32_t PriceLevel::placeOrder(const Order &order, std::vector<uint64_t> &removedOrders){
	uint32_t totalQuantityFilled = 0;
	uint32_t quantityToFulfill = order.getQuantity();
	
	if(orders.empty()) return quantityToFulfill;

	while(!orders.empty() && totalQuantityFilled < quantityToFulfill){
		Order& front = orders.front();
		uint32_t quantityProvided = front.getQuantity();
		if(quantityProvided > quantityToFulfill - totalQuantityFilled){
			uint32_t remaining = quantityToFulfill - totalQuantityFilled;
			front.setQuantity(front.getQuantity() - remaining);
			totalQuantityFilled = quantityToFulfill;
			this->setQuantity(this->getQuantity() - remaining);
			return 0; // Whole Order done;
		} else {
			totalQuantityFilled += quantityProvided;
			removedOrders.push_back(front.getOrderId());
    			orderIdMap.erase(front.getOrderId()); 		
			orders.pop_front();
		}	
	}
	this->setQuantity(this->getQuantity() - totalQuantityFilled);
	return quantityToFulfill - totalQuantityFilled;
}

void PriceLevel::addOrder(const Order& order){
	auto it = orders.insert(orders.end(), order);
	orderIdMap[order.getOrderId()] = it;	
	this->setQuantity(this->getQuantity() + order.getQuantity());
}

void PriceLevel::removeOrder(uint64_t orderId){
	auto mapIt = orderIdMap.find(orderId);
    	if (mapIt == orderIdMap.end()) {
        	return; // Order not found, nothing to remove
    	}
	auto orderIt = mapIt->second;
	this->setQuantity(this->getQuantity() - orderIt->getQuantity());
    	orders.erase(orderIt);
    	orderIdMap.erase(mapIt);
}

bool PriceLevel::isEmpty(){
	return orders.empty();
}


