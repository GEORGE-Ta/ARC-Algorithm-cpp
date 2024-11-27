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

## Implementation

### C++ Version
The C++ implementation is header-only and can be easily integrated into your project. The main implementation is in `arc_cache.hpp`.

Key features:
- Template-based implementation for flexibility
- Header-only for easy integration
- Efficient memory management
- Thread-safe operations

### Python Version
A Python implementation is also available in the `python_version` directory, which includes a comparison script to evaluate ARC against other caching algorithms (LRU, LFU).

Key features:
- Comparative analysis with LRU and LFU
- Visual performance metrics
- Multiple access pattern tests
- Automated performance reporting

## Building and Running

To compile the test program:
```bash
g++ -std=c++17 test_cache.cpp -o test_cache
```

To run the tests:
```bash
./test_cache
```

## Usage

### C++ Version
```cpp
#include "arc_cache.hpp"

// Create an ARC cache with size 1000
ARCache<int, int> cache(1000);

// Add or update items
cache.put(key, value);

// Get items
auto value = cache.get(key);
```

### Python Version
```bash
# Navigate to python_version directory
cd python_version

# Create and activate virtual environment (Windows)
python -m venv venv
venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt

# Run the comparison script
python compare_caches.py
```

The Python script will:
1. Compare ARC with LRU and LFU caches
2. Test different access patterns:
   - Random access
   - Locality-based access
   - Periodic access
   - Zipfian distribution
3. Generate performance metrics showing:
   - Hit rates for each algorithm
   - Performance comparison with ARC
   - Summary statistics
4. Plot the results in `cache_comparison.png`

Example output:
```
=== Cache Performance Comparison ===
Pattern Type     Cache Type   Hit Rate   Time (s)     vs ARC    Performance
---------------------------------------------------------------------------
Random          LRU           9.89%      0.027       -0.08%    ↑ ARC Better
                LFU           9.92%      0.048       -0.05%    ↑ ARC Better
                ARC           9.97%      0.082                 
...

=== Summary of ARC Performance ===
Total scenarios where ARC performs better: 6/8 (75.0%)

Breakdown by pattern:
Random           : ARC better in 2/2 cases (100.0%)
Locality         : ARC better in 2/2 cases (100.0%)
Periodic         : Equal performance in all cases
Zipfian          : ARC better in 2/2 cases (100.0%)
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
- Python 3.7 or later (for Python version)

## References
- [Nimrod Megiddo and Dharmendra S. Modha, "ARC: A Self-Tuning, Low Overhead Replacement Cache"](https://www.usenix.org/conference/fast-03/arc-self-tuning-low-overhead-replacement-cache)
