#ifndef EXPIRYENTRY_HPP
#define EXPIRYENTRY_HPP

#include "order.hpp"

class ExpiryEntry {
public:
	ExpiryEntry(uint32_t _expiryTime, const Order& _order) : expiryTime(_expiryTime), order(_order) {};
	
	// Copy constructor
	ExpiryEntry(const ExpiryEntry& other) = default;
	
	// Copy assignment operator
	ExpiryEntry& operator=(const ExpiryEntry& other) = default;
	
	// Move constructor
	ExpiryEntry(ExpiryEntry&& other) noexcept = default;
	
	// Move assignment operator
	ExpiryEntry& operator=(ExpiryEntry&& other) noexcept = default;
	
	// Destructor
	~ExpiryEntry() = default;
	
	// Getters
	uint32_t getExpiryTime() const { return expiryTime; }
	const Order& getOrder() const { return order; }
	
	// Setters
	void setExpiryTime(uint32_t _expiryTime) { expiryTime = _expiryTime; }
	void setOrder(const Order& _order) { order = _order; }
	
	bool operator>(const ExpiryEntry& other) const {
		return expiryTime > other.expiryTime;
	}

private:
	uint32_t expiryTime;
	Order order;
};

#endif 
