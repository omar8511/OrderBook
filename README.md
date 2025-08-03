# C++ Order Book

A multi-threaded order book implementation in C++20 with support for various order types and automatic expiry handling.

## Features

### Order Types
- **GTC** (Good Till Cancelled) - Standard limit orders
- **IOC** (Immediate or Cancel) - Execute immediately, cancel remainder  
- **FOK** (Fill or Kill) - Execute completely or not at all
- **MKT** (Market) - Execute at best available price
- **GTD** (Good Till Date) - Orders with automatic expiry

### Functionality
- Order placement, cancellation, and modification
- Price/quantity/expiry time modifications  
- Automatic order expiry with dedicated background thread
- Level 1 & Level 2 market data display
- Thread-safe operations with shared mutexes
- Order tracking and statistics

## Architecture

### Data Structures
- **OrderedPriceLevel**: Maintains sorted price levels using `std::vector` + `std::unordered_map` for O(1) lookups
- **PriceLevel**: Uses `std::list` for FIFO order queue with `std::unordered_map` for O(1) order ID lookups
- **ExpiryEntry**: Priority queue for time-based order expiry

### Threading
- Separate read/write locks for buy and sell sides
- Shared mutexes allow concurrent reads
- Dedicated expiry handler thread with condition variables
- Thread-safe metadata tracking

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Testing

### Latency Tests
```bash
./latencyTests <numThreads> <ordersPerThread>
```

### Throughput Tests  
```bash
./throughputTests
```

### Order Type Tests
```bash
./orderTypeTests
```
