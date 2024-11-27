import random
import time
from typing import List, TypeVar
import numpy as np
from arc_cache import ARCache

def test_cache_scenario(cache: ARCache, access_pattern: List[int]) -> float:
    """Test cache performance with given access pattern."""
    hits = 0
    total = 0
    
    for key in access_pattern:
        if cache.get(key) is not None:
            hits += 1
        else:
            cache.put(key, key)
        total += 1
    
    return hits / total if total > 0 else 0.0

def generate_random_access_pattern(data_range: int, pattern_length: int, seed: int = 42) -> List[int]:
    """Generate random access pattern."""
    random.seed(seed)
    return [random.randint(0, data_range - 1) for _ in range(pattern_length)]

def generate_locality_access_pattern(data_range: int, pattern_length: int, locality_size: int, seed: int = 42) -> List[int]:
    """Generate access pattern with locality."""
    random.seed(seed)
    pattern = []
    for _ in range(pattern_length):
        base = random.randint(0, data_range - locality_size)
        pattern.append(base + random.randint(0, locality_size - 1))
    return pattern

def generate_periodic_access_pattern(data_range: int, pattern_length: int, period: int, seed: int = 42) -> List[int]:
    """Generate periodic access pattern."""
    random.seed(seed)
    base_pattern = [random.randint(0, data_range - 1) for _ in range(period)]
    pattern = []
    for _ in range(pattern_length):
        pattern.extend(base_pattern)
    return pattern[:pattern_length]

def generate_zipfian_access_pattern(data_range: int, pattern_length: int, skew: float, seed: int = 42) -> List[int]:
    """Generate Zipfian access pattern."""
    random.seed(seed)
    np.random.seed(seed)
    
    # Generate Zipfian probabilities
    x = np.arange(1, data_range + 1)
    weights = 1 / (x ** skew)
    weights /= weights.sum()
    
    # Generate pattern
    return list(np.random.choice(range(data_range), size=pattern_length, p=weights))

def main():
    """Main function to test cache implementations."""
    # Test parameters
    cache_size = 100
    data_range = 1000
    pattern_length = 10000
    
    # Initialize cache
    arc_cache = ARCache(cache_size)
    
    # Test different access patterns
    patterns = {
        "Random": generate_random_access_pattern(data_range, pattern_length),
        "Locality": generate_locality_access_pattern(data_range, pattern_length, 20),
        "Periodic": generate_periodic_access_pattern(data_range, pattern_length, 50),
        "Zipfian": generate_zipfian_access_pattern(data_range, pattern_length, 1.0)
    }
    
    print(f"{'Pattern Type':<15} {'Hit Rate':>10}")
    print("-" * 30)
    
    for pattern_name, pattern in patterns.items():
        arc_cache.clear()
        hit_rate = test_cache_scenario(arc_cache, pattern)
        print(f"{pattern_name:<15} {hit_rate:>10.2%}")

if __name__ == "__main__":
    main()
