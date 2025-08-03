cmake_minimum_required(VERSION 3.16)
project(OrderBook)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_BUILD_TYPE Release)

# Find required packages
find_package(Threads REQUIRED)

# Common source files
set(COMMON_SOURCES
    orderbook.cpp
    pricelevel.cpp
    orderedpricelevel.cpp
)

# Latency Tests executable
add_executable(latencyTests 
    ${COMMON_SOURCES}
    latencyTests.cpp
)
target_link_libraries(latencyTests Threads::Threads)

# Order Type Tests executable
add_executable(orderTypeTests 
    ${COMMON_SOURCES}
    orderTypeTests.cpp
)
target_link_libraries(orderTypeTests Threads::Threads)

# Throughput Tests executable
add_executable(throughputTests 
    ${COMMON_SOURCES}
    throughputTests.cpp
)
target_link_libraries(throughputTests Threads::Threads)