#include "orderbook.hpp"
#include "order.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main() {

	OrderBook orderBook("AAPL");

	// Test GTC orders
	std::cout << "Testing GTC orders...\n";
	Order gtcBuy(1, GTC, 100, 105, 0, true);
	Order gtcSell(2, GTC, 50, 95, 0, false);
	orderBook.placeOrder(gtcBuy);
	orderBook.placeOrder(gtcSell);
	std::cout << "GTC orders placed successfully\n";

	// Test IOC orders
	std::cout << "Testing IOC orders...\n";
	Order iocBuy(3, IOC, 25, 96, 0, true);
	Order iocSell(4, IOC, 25, 104, 0, false);
	orderBook.placeOrder(iocBuy);
	orderBook.placeOrder(iocSell);
	std::cout << "IOC orders placed successfully\n";

	// Test FOK orders
	std::cout << "Testing FOK orders...\n";
	Order fokBuy(5, FOK, 10, 96, 0, true);
	Order fokSell(6, FOK, 10, 104, 0, false);
	orderBook.placeOrder(fokBuy);
	orderBook.placeOrder(fokSell);
	std::cout << "FOK orders placed successfully\n";

	// Test MKT orders
	std::cout << "Testing MKT orders...\n";
	Order mktBuy(7, MKT, 20, 0, 0, true);
	Order mktSell(8, MKT, 20, 0, 0, false);
	orderBook.placeOrder(mktBuy);
	orderBook.placeOrder(mktSell);
	std::cout << "MKT orders placed successfully\n";

	// Test GTD orders
	std::cout << "Testing GTD orders...\n";
	uint32_t currentTime = static_cast<uint32_t>(
			std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::system_clock::now().time_since_epoch()
				).count()
			);
	uint32_t expiryTime = currentTime + 5;

	Order gtdBuy(9, GTD, 30, 107, expiryTime, true);
	Order gtdSell(10, GTD, 30, 93, expiryTime, false);
	orderBook.placeOrder(gtdBuy);
	orderBook.placeOrder(gtdSell);
	std::cout << "GTD orders placed successfully\n";

	std::cout << "Waiting 6 seconds for GTD expiry...\n";
	std::this_thread::sleep_for(std::chrono::seconds(6));

	std::cout << "GTD orders should have expired\n";

	// Test cancel order functionality
	std::cout << "\nTesting cancel order functionality...\n";
	Order cancelTestOrder(11, GTC, 50, 100, 0, true);
	orderBook.placeOrder(cancelTestOrder);
	std::cout << "Order 11 placed successfully\n";
	orderBook.cancelOrder(11);
	std::cout << "Order 11 cancelled successfully\n";

	// Test modify quantity functionality
	std::cout << "\nTesting modify quantity functionality...\n";
	Order modifyQuantityOrder(12, GTC, 100, 102, 0, true);
	orderBook.placeOrder(modifyQuantityOrder);
	std::cout << "Order 12 placed with quantity 100\n";
	orderBook.modifyQuantity(12, 150);
	std::cout << "Order 12 quantity modified to 150\n";

	// Test modify price functionality
	std::cout << "\nTesting modify price functionality...\n";
	Order modifyPriceOrder(13, GTC, 75, 101, 0, false);
	orderBook.placeOrder(modifyPriceOrder);
	std::cout << "Order 13 placed with price 101\n";
	orderBook.modifyPrice(13, 99);
	std::cout << "Order 13 price modified to 99\n";

	// Test modify expiry time functionality
	std::cout << "\nTesting modify expiry time functionality...\n";
	uint32_t newCurrentTime = static_cast<uint32_t>(
			std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::system_clock::now().time_since_epoch()
				).count()
			);
	uint32_t newExpiryTime = newCurrentTime + 10;

	Order modifyExpiryOrder(15, GTD, 80, 106, newCurrentTime + 3, true);
	orderBook.placeOrder(modifyExpiryOrder);
	std::cout << "Order 15 placed as GTD with 3 second expiry\n";
	orderBook.modifyExpiryTime(15, newExpiryTime);
	std::cout << "Order 15 expiry time modified to 10 seconds from now\n";

	// Test displayStats functionality
	std::cout << "\nTesting displayStats functionality...\n";
	orderBook.displayStats();

	std::cout << "\nAll tests completed successfully!\n";

	return 0;
}
