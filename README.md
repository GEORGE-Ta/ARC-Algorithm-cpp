# Cache Replacement Algorithm Comparison

This project implements and compares three cache replacement algorithms:
- ARC (Adaptive Replacement Cache)
- LRU (Least Recently Used)
- LFU (Least Frequently Used)

## Implementation Details

The project contains the following key components:

- `cache.hpp`: Base interface for all cache implementations
- `arc_cache.hpp`: Implementation of the ARC algorithm
- `lru_cache.hpp`: Implementation of the LRU algorithm
- `lfu_cache.hpp`: Implementation of the LFU algorithm
- `test_cache.cpp`: Test program that compares the performance of all three algorithms

## Building and Running

To compile the test program:
```bash
g++ -std=c++17 test_cache.cpp -o test_cache
```

To run the tests:
```bash
./test_cache
```

## Test Results

The test program generates a mixed workload that includes:
1. Frequently accessed items
2. Recently accessed items
3. Random access patterns

This mixed workload demonstrates ARC's ability to adapt and perform better than both LRU and LFU in real-world scenarios.

Example output:
```
Testing cache performance with:
Cache size: 100
Data range: 1000
Access pattern length: 10000

Hit rates:
ARC: 13.26%
LRU: 5.77%
LFU: 12.90%
```

## Why ARC Performs Better

ARC (Adaptive Replacement Cache) outperforms traditional algorithms because:
1. It dynamically balances between recency and frequency
2. It maintains ghost lists to track recently evicted items
3. It automatically adapts its policy based on the workload pattern

## Requirements
- C++17 or later
- A C++ compiler (e.g., g++, clang++)
