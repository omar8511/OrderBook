#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

#include "parser.hpp"
#include "orderbook.hpp"
#include "order.hpp"


Parser::Parser(const std::string assetName){ // can quikc init do a heap class
	this->book = new OrderBook(assetName);
}

void Parser::parseCSVThread(){
	std::ifstream file(filePath);
	if(!file.is_open()){
		std::cerr << "Invalid filePath provided" << std::endl;
		return;
	}
	std::string line;
	getline(file, line); // Skip header

	while(getline(file, line)) {
         	{
              		std::lock_guard<std::mutex> lock(queueMutex);
              		lineQueue.push_back(line);
          	}
          	queueCV.notify_one();
      }
      
      // Signal that producer is done
      producerDone = true;
      queueCV.notify_all();
}


void Parser::consumeOrderQueue(){
	std::vector<uint64_t> localLatencies;

	while(true) {
		std::string line;
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			queueCV.wait(lock, [this] { return !lineQueue.empty() || producerDone; });
			
			if(lineQueue.empty() && producerDone) break;
			
			line = lineQueue.front();
			lineQueue.pop_front();
		}

		// Parse and time (no lock needed here)
		std::stringstream ss(line);
		std::vector<std::string> fields;
		std::string field;
		while (getline(ss, field, ',')) {
			fields.push_back(field);
		}

		if (fields.size() < 6) {
			std::cerr << "Malformed line: " << line << std::endl;
			continue;
		}

		uint64_t orderId = std::stoull(fields[0]);

		OrderType orderType;
		const std::string& type = fields[1];
		if (type == "GTC") orderType = GTC;
		else if (type == "IOC") orderType = IOC;
		else if (type == "FOK") orderType = FOK;
		else if (type == "MKT") orderType = MKT;
		else if (type == "GTD") orderType = GTD;
		else orderType = GTC; // default fallback

		uint32_t quantity = static_cast<uint32_t>(std::stoul(fields[2]));
		uint32_t price = static_cast<uint32_t>(std::stoul(fields[3]));
		uint32_t expiry = static_cast<uint32_t>(std::stoul(fields[4]));
		std::string buyField = fields[5];

 		buyField.erase(buyField.find_last_not_of(" \n\r\t") + 1);
  		bool isBuy = (buyField == "BUY");
		Order order(orderId, orderType, quantity, price, expiry, isBuy);

		auto t1 = std::chrono::high_resolution_clock::now();
		book->placeOrder(order);
		auto t2 = std::chrono::high_resolution_clock::now();
		
		uint64_t latency = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
		localLatencies.push_back(latency);
	}

	// Merge local latencies at end
	{
		std::lock_guard<std::mutex> lock(latencyMutex);
		allLatencies.insert(allLatencies.end(), localLatencies.begin(), localLatencies.end());
	}
}

void Parser::startOrderBook(){
	std::cout << "Please enter the file path: ";
	std::cin >> filePath;

	// Reset state for new run
	producerDone = false;
	allLatencies.clear();

	int numThreads = 4;
	allLatencies.reserve(100000); // Reserve space for latencies

	auto startTime = std::chrono::high_resolution_clock::now();

	// Start producer thread
	std::thread producerThread(&Parser::parseCSVThread, this);

	// Start consumer threads
	std::vector<std::thread> consumerThreads;
	for(int i = 0; i < numThreads; i++) {
		consumerThreads.emplace_back(&Parser::consumeOrderQueue, this);
	}

	// Wait for producer to finish
	producerThread.join();

	// Wait for all consumers to finish
	for(auto& t : consumerThreads) {
		t.join();
	}

	auto endTime = std::chrono::high_resolution_clock::now();

	// Calculate and display metrics
	if (allLatencies.empty()) {
		std::cout << "No latency data collected!\n";
		return;
	}

	uint64_t totalLatency = 0;
	uint64_t minLatency = UINT64_MAX;
	uint64_t maxLatency = 0;

	for (auto lat : allLatencies) {
		totalLatency += lat;
		if (lat < minLatency) minLatency = lat;
		if (lat > maxLatency) maxLatency = lat;
	}

	double avgLatency = static_cast<double>(totalLatency) / allLatencies.size();

	// Calculate throughput
	uint64_t totalTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
	double throughputOrdersPerSec = (allLatencies.size() * 1000.0) / totalTimeMs;

	std::cout << "Total orders processed: " << allLatencies.size() << "\n";
	std::cout << "Total time: " << totalTimeMs << " ms\n";
	std::cout << "Throughput: " << throughputOrdersPerSec << " orders/sec\n";
	std::cout << "\nLatency stats (nanoseconds):\n";
	std::cout << "Min: " << minLatency << " ns\n";
	std::cout << "Max: " << maxLatency << " ns\n";
	std::cout << "Avg: " << avgLatency << " ns\n";
}
