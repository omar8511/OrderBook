#ifndef PRICELEVEL_H
#define PRICELEVEL_H

#include "order.hpp"
#include <list>
#include <unordered_map>
#include <vector>
#include <mutex>

// Make quantity exposed via a getter

class PriceLevel {

public:
	PriceLevel(uint32_t _price) : price(_price), quantity(0) {};
	
	// Copy constructor
	PriceLevel(const PriceLevel& other) = default;
	
	// Copy assignment operator
	PriceLevel& operator=(const PriceLevel& other) = default;
	
	// Move constructor
	PriceLevel(PriceLevel&& other) noexcept = default;
	
	// Move assignment operator
	PriceLevel& operator=(PriceLevel&& other) noexcept = default;
	
	// Destructor
	~PriceLevel() = default;
	
	// Getters
	uint32_t getPrice() const { return price; }
	uint32_t getQuantity() const { return quantity; }
	
	// Setters
	void setPrice(uint32_t _price) { price = _price; }
	void setQuantity(uint32_t _quantity) { quantity = _quantity; }
	
	// Methods
	uint32_t placeOrder(const Order &order, std::vector<uint64_t> &removedOrders);
	void addOrder(const Order &order);
	void removeOrder(uint64_t orderId);
	bool isEmpty();	

private:
	uint32_t price;
	uint32_t quantity;
	std::list<Order> orders;
	std::unordered_map<uint64_t, std::list<Order>::iterator> orderIdMap;
};

#endif

