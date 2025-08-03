#include "orderbook.hpp"
#include "order.hpp"

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib> // for std::atoi

void worker(OrderBook &orderBook, uint64_t numOrders){
	for(uint64_t i = 0; i < numOrders; i++){
		Order toPlace(i, GTC, 1, 100, 1000, false);
		orderBook.placeOrder(toPlace);
	}
}


int main(int argc, char* argv[]){
	if (argc < 3) {
        	std::cerr << "Usage: " << argv[0] << " <numThreads> <ordersPerThread>\n";
        	return 1;
    	}
	OrderBook ob("AAPL");
	std::vector<std::thread> threads;
	uint64_t numOrdersPerThread = std::stoull(argv[2]);
	int numThreads = std::atoi(argv[1]);


    	auto start = std::chrono::steady_clock::now();	
	for(int i = 0; i < numThreads; i++){
        	threads.emplace_back(worker, std::ref(ob), numOrdersPerThread);	
	}
	
	for (auto& t : threads) {
        	t.join();
   	}

 	auto end = std::chrono::steady_clock::now();
    	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	
	uint64_t totalOrders = (uint64_t)numThreads * numOrdersPerThread;
    	double seconds = duration_ms / 1000.0;
    	double throughput = totalOrders / seconds;

	std::cout << "Processed " << totalOrders << " orders in " << seconds << " seconds.\n";
    	std::cout << "Throughput: " << throughput << " orders/second.\n";


    	return 0;
}
