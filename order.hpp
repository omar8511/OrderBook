#ifndef ORDER_HPP
#define ORDER_HPP

#include <cstdint>
#include <vector>
#include <string>

// Use unsigned ints over signed to prevent wraparounds and all of our values are positive

// In C++ structs and classes act the same but default struct is public default for class is private



enum OrderType {
	GTC, // Good till cancelled
	IOC, // Immediate or cancel
	FOK, // Fill or Kill
	MKT, // Market
	GTD, // Good till date
};

class Order {
public:
	Order(const uint64_t _orderId, const OrderType _orderType, uint32_t _quantity, const uint32_t _price, const uint32_t _expiryTime, const bool _isBuy) : 
	orderId(_orderId), orderType(_orderType), originalQuantity(_quantity), quantity(_quantity), price(_price), expiryTime(_expiryTime), isBuy(_isBuy), priceImprovement(0) {};
	Order() = default;
	
	// Copy constructor
	Order(const Order& other) = default;
	
	// Copy assignment operator
	Order& operator=(const Order& other) = default;
	
	// Move constructor
	Order(Order&& other) noexcept = default;
	
	// Move assignment operator
	Order& operator=(Order&& other) noexcept = default;
	
	~Order() = default;
	
	// Getters
	uint64_t getOrderId() const { return orderId; }
	OrderType getOrderType() const { return orderType; }
	uint32_t getOriginalQuantity() const { return originalQuantity; }
	uint32_t getQuantity() const { return quantity; }
	uint32_t getPrice() const { return price; }
	uint32_t getExpiryTime() const { return expiryTime; }
	uint32_t getPriceImprovement() const { return priceImprovement; }
	bool getIsBuy() const { return isBuy; }
	
	// Setters
	void setOrderType(OrderType _orderType) { orderType = _orderType; }
	void setQuantity(uint32_t _quantity) { quantity = _quantity; }
	void setPrice(uint32_t _price) { price = _price; }
	void setExpiryTime(uint32_t _expiryTime) { expiryTime = _expiryTime; }
	void setPriceImprovement(uint32_t _priceImprovement) { priceImprovement = _priceImprovement; }
	void setIsBuy(bool _isBuy) { isBuy = _isBuy; }

private:
	uint64_t orderId;
	OrderType orderType;
	uint32_t originalQuantity;
	uint32_t quantity;
	uint32_t price;
	uint32_t expiryTime; // make this optional
	uint32_t priceImprovement;
	bool isBuy; // Switch to ENUM
};


#endif 
