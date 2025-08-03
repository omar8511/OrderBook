#include "orderbook.hpp"
#include "order.hpp"

#include <chrono>
#include <random>
#include <vector>
#include <thread>
#include <iostream>
#include <mutex>

std::mutex latencyMutex;
std::vector<uint64_t> latencies_ns;

Order generateRandomOrderWithId(uint64_t orderId) {
	thread_local std::mt19937 rng(std::random_device{}());
	thread_local std::uniform_int_distribution<int> typeDist(0, 4); // Include GTD orders
	thread_local std::bernoulli_distribution buySellDist(0.5);
	// Realistic market parameters: tighter spread, tick-based pricing
	thread_local std::uniform_int_distribution<int> priceOffsetCents(-50, 50); // +/- $0.50 from mid
	thread_local std::exponential_distribution<double> quantityDist(0.01); // Exponential for realistic volume distribution

	OrderType type = static_cast<OrderType>(typeDist(rng));
	bool isBuy = buySellDist(rng);

	// Use cents for realistic tick sizes (e.g., $100.00 = 10000 cents)
	uint32_t basePriceCents = 10000; // $100.00
	uint32_t price = basePriceCents + priceOffsetCents(rng);
	// Cap quantity at reasonable levels, bias toward smaller orders
	uint32_t quantity = std::min(static_cast<uint32_t>(quantityDist(rng) * 100) + 1, 5000u);
	uint32_t expiry = (type == GTD) ? static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()) + 10 : 0;

	return Order(orderId, type, quantity, price, expiry, isBuy);
}

void worker(OrderBook &orderBook, uint64_t startId, uint64_t count, int threadId,
		std::mutex &latencyMutex, std::vector<uint64_t> &latencies_ns) {
	std::vector<uint64_t> localLatencies;
	localLatencies.reserve(count);

	for (uint64_t i = 0; i < count; i++) {
		uint64_t orderId = startId + i;
		Order order = generateRandomOrderWithId(orderId);

		auto t1 = std::chrono::high_resolution_clock::now();
		orderBook.placeOrder(order);
		auto t2 = std::chrono::high_resolution_clock::now();

		uint64_t latency = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
		localLatencies.push_back(latency);
	}

	{
		std::lock_guard<std::mutex> lock(latencyMutex);
		latencies_ns.insert(latencies_ns.end(), localLatencies.begin(), localLatencies.end());
	}
}

int main(int argc, char* argv[]) {

	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <numThreads> <ordersPerThread>\n";
		return 1;
	}


	OrderBook orderBook("AAPL");
	std::vector<std::thread> threads;

	uint64_t totalOrders = 100000;
	int numThreads = 4;
	uint64_t ordersPerThread = totalOrders / numThreads;

	latencies_ns.reserve(totalOrders);


	for (int t = 0; t < numThreads; t++) {
		uint64_t startId = t * ordersPerThread;
		threads.emplace_back(worker, std::ref(orderBook), startId, ordersPerThread, t,
				std::ref(latencyMutex), std::ref(latencies_ns));
	}

	for (auto &t : threads) {
		t.join();
	}

	// Display order book statistics after all orders are placed
	std::cout << "\n=== Order Book State After Latency Test ===\n";
	orderBook.displayStats();

	if (latencies_ns.empty()) {
		std::cout << "No latency data collected!\n";
		return 1;
	}

	uint64_t totalLatency = 0;
	uint64_t minLatency = UINT64_MAX;
	uint64_t maxLatency = 0;

	for (auto lat : latencies_ns) {
		totalLatency += lat;
		if (lat < minLatency) minLatency = lat;
		if (lat > maxLatency) maxLatency = lat;
	}

	double avgLatency = static_cast<double>(totalLatency) / latencies_ns.size();

	std::cout << "Latency stats (nanoseconds):\n";
	std::cout << "Min: " << minLatency << " ns\n";
	std::cout << "Max: " << maxLatency << " ns\n";
	std::cout << "Avg: " << avgLatency << " ns\n";

	return 0;
}
