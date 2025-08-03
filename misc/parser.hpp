#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <mutex>
#include <vector>
#include <deque>
#include <condition_variable>
#include <atomic>
#include "orderbook.hpp"



class Parser{
public:
	Parser(const std::string assetName);
	void startOrderBook();
private:
	OrderBook* book;
	std::deque<std::string> lineQueue;
	std::mutex queueMutex;
	std::condition_variable queueCV;
	std::atomic<bool> producerDone{false};
	std::vector<uint64_t> allLatencies;
	std::mutex latencyMutex;
	std::string filePath;
	void parseCSVThread();
	void consumeOrderQueue();
};



#endif
