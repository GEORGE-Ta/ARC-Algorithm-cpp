# ARC (Adaptive Replacement Cache) Python Implementation

This is a Python implementation of the ARC (Adaptive Replacement Cache) algorithm. ARC combines the benefits of LRU (Least Recently Used) and LFU (Least Frequently Used) caching strategies to provide better performance across different workload patterns.

## Features

- Generic implementation supporting any hashable key type and any value type
- Efficient implementation using OrderedDict for O(1) operations
- Comprehensive test suite with different access patterns:
  - Random access
  - Locality-based access
  - Periodic access
  - Zipfian distribution

## Installation

1. Create a virtual environment (recommended):
```bash
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
```

2. Install dependencies:
```bash
pip install -r requirements.txt
```

## Usage

```python
from arc_cache import ARCache

# Create a cache with capacity of 100
cache = ARCache(100)

# Add items
cache.put("key1", "value1")
cache.put("key2", "value2")

# Get items
value = cache.get("key1")  # Returns "value1"
missing = cache.get("key3")  # Returns None

# Check size
size = cache.size()  # Returns current number of items in cache

# Clear cache
cache.clear()
```

## Running Tests

To run the test suite:

```bash
python test_cache.py
```

This will run tests with different access patterns and display hit rates for each pattern.

## Implementation Details

The implementation uses four internal data structures:
- T1: Recently used items
- T2: Frequently used items
- B1: Ghost entries for items recently evicted from T1
- B2: Ghost entries for items recently evicted from T2

The algorithm automatically adapts the target size of T1 (p) based on the workload pattern to optimize hit rates.
