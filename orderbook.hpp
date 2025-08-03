#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <map>
#include <unordered_map>
#include <vector>
#include <utility>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <stdatomic.h>
#include <thread>
#include "pricelevel.hpp"
#include "expiryentry.hpp"
#include "order.hpp"
#include "orderedpricelevel.hpp"

/*
My previous locking strategy was to hold one orderbookMutex which blocks everything so only 1 thread can operate on the book at a time now we maintain a shared lock so many threads can read the maap at once and only one can write to it which increases throughput. unique_lock when modifying and shared_lock when reading. Note this also protects the corresponding Level 1 pointers when they are being reassigned.
*/

class OrderBook{

public: 
	OrderBook(const std::string& assetName);
	~OrderBook();
	
	// Delete copy operations (OrderBook should not be copied)
	OrderBook(const OrderBook&) = delete;
	OrderBook& operator=(const OrderBook&) = delete;
	
	// Allow move operations
	OrderBook(OrderBook&& other) noexcept;
	OrderBook& operator=(OrderBook&& other) noexcept;
	void cancelOrder(uint64_t orderId);	
	void placeOrder(Order &order);
	std::string trackOrder(uint64_t orderId);
	void modifyQuantity(uint64_t orderId, uint32_t newQuantity);
	void modifyPrice(uint64_t orderId, uint32_t newPrice);
	void modifyExpiryTime(uint64_t orderId, uint32_t newExpiryTime);
	void displayStats();
	

private:	
	std::pair<uint32_t, std::vector<uint64_t>> fulfillBuy(Order &order);
	std::pair<uint32_t, std::vector<uint64_t>> fulfillSell(Order &order);	
	OrderedPriceLevel level2Buy;
	OrderedPriceLevel level2Sell;
	std::unordered_map<uint32_t, Order> metaDataMap; // How can we get rid of copying each time 
	std::mutex metaDataMutex;	
	std::shared_mutex level2BuyMutex;
	std::shared_mutex level2SellMutex;	

	std::priority_queue<ExpiryEntry,std::vector<ExpiryEntry>, std::greater<ExpiryEntry>> expiryQueue;
	std::mutex expiryMutex;
	std::condition_variable expiryCV;
	std::atomic_bool shutdownFlag{false};
	std::thread expiryThread;
	void expiryHandler();

};


#endif
