import random
import time
from typing import List, Dict, Type
import numpy as np
import matplotlib.pyplot as plt

from cache import Cache
from lru_cache import LRUCache
from lfu_cache import LFUCache
from arc_cache import ARCache

def test_cache_scenario(cache: Cache, access_pattern: List[int]) -> tuple[float, float]:
    """Test cache performance with given access pattern."""
    hits = 0
    total = 0
    start_time = time.time()
    
    for key in access_pattern:
        if cache.get(key) is not None:
            hits += 1
        else:
            cache.put(key, key)
        total += 1
    
    end_time = time.time()
    hit_rate = hits / total if total > 0 else 0.0
    return hit_rate, end_time - start_time

def generate_random_pattern(data_range: int, pattern_length: int, seed: int = 42) -> List[int]:
    """Generate random access pattern."""
    random.seed(seed)
    return [random.randint(0, data_range - 1) for _ in range(pattern_length)]

def generate_locality_pattern(data_range: int, pattern_length: int, locality_size: int, seed: int = 42) -> List[int]:
    """Generate access pattern with locality."""
    random.seed(seed)
    pattern = []
    for _ in range(pattern_length):
        if random.random() < 0.8:  # 80% chance to access local region
            base = random.randint(0, data_range - locality_size)
            pattern.append(random.randint(base, base + locality_size - 1))
        else:
            pattern.append(random.randint(0, data_range - 1))
    return pattern

def generate_periodic_pattern(data_range: int, pattern_length: int, period: int, seed: int = 42) -> List[int]:
    """Generate periodic access pattern."""
    random.seed(seed)
    base_pattern = [random.randint(0, data_range - 1) for _ in range(period)]
    pattern = []
    while len(pattern) < pattern_length:
        pattern.extend(base_pattern)
    return pattern[:pattern_length]

def generate_zipfian_pattern(data_range: int, pattern_length: int, skew: float, seed: int = 42) -> List[int]:
    """Generate Zipfian access pattern."""
    np.random.seed(seed)
    # Calculate Zipfian probabilities
    x = np.arange(1, data_range + 1)
    probabilities = 1 / (x ** skew)
    probabilities /= probabilities.sum()
    
    # Generate pattern
    return np.random.choice(data_range, size=pattern_length, p=probabilities).tolist()

def compare_caches(cache_types: Dict[str, Type[Cache]], cache_size: int, 
                  data_range: int, pattern_length: int) -> None:
    """Compare different cache implementations."""
    # Test parameters
    patterns = {
        "Random": generate_random_pattern(data_range, pattern_length),
        "Locality": generate_locality_pattern(data_range, pattern_length, locality_size=100),
        "Periodic": generate_periodic_pattern(data_range, pattern_length, period=1000),
        "Zipfian": generate_zipfian_pattern(data_range, pattern_length, skew=1.5)
    }
    
    # Store results
    results = {name: {"hit_rates": [], "times": []} for name in cache_types.keys()}
    pattern_summaries = []
    
    # Test each cache type with each pattern
    print("\n=== Cache Performance Comparison ===")
    print(f"{'Pattern Type':<15} {'Cache Type':<8} {'Hit Rate':>10} {'Time (s)':>10} {'vs ARC':>12} {'Performance':>15}")
    print("-" * 75)
    
    for pattern_name, pattern in patterns.items():
        arc_results = None
        pattern_results = {}
        pattern_summary = {"pattern": pattern_name, "wins": 0, "total": 0}
        
        # First get ARC results for comparison
        for cache_name, cache_type in cache_types.items():
            cache = cache_type(cache_size)
            hit_rate, exec_time = test_cache_scenario(cache, pattern)
            pattern_results[cache_name] = (hit_rate, exec_time)
            if cache_name == "ARC":
                arc_results = (hit_rate, exec_time)
        
        # Print results with comparison
        first_in_pattern = True
        for cache_name, (hit_rate, exec_time) in pattern_results.items():
            pattern_display = pattern_name if first_in_pattern else ""
            diff = ""
            performance = ""
            if cache_name != "ARC" and arc_results is not None:
                diff_val = (hit_rate - arc_results[0])*100
                diff = f"{diff_val:+.2f}%"
                if diff_val < -0.01:
                    performance = "↑ ARC Better"
                    pattern_summary["wins"] += 1
                elif diff_val > 0.01:
                    performance = "↓ ARC Worse"
                elif abs(diff_val) <= 0.01:
                    performance = "= Equal"
                pattern_summary["total"] += 1
            
            print(f"{pattern_display:<15} {cache_name:<8} {hit_rate*100:>10.2f}% {exec_time:>10.3f} {diff:>12} {performance:>15}")
            first_in_pattern = False
            
            results[cache_name]["hit_rates"].append(hit_rate * 100)
            results[cache_name]["times"].append(exec_time)
        print("-" * 75)
        pattern_summaries.append(pattern_summary)
    
    # Print summary statistics
    print("\n=== Summary of ARC Performance ===")
    total_wins = sum(summary["wins"] for summary in pattern_summaries)
    total_comparisons = sum(summary["total"] for summary in pattern_summaries)
    print(f"Total scenarios where ARC performs better: {total_wins}/{total_comparisons} ({total_wins/total_comparisons*100:.1f}%)")
    print("\nBreakdown by pattern:")
    for summary in pattern_summaries:
        if summary["total"] > 0:
            win_rate = summary["wins"]/summary["total"]*100
            print(f"{summary['pattern']:<15}: ARC better in {summary['wins']}/{summary['total']} cases ({win_rate:.1f}%)")
    
    # Plot results
    plot_results(results, patterns.keys())

def plot_results(results: Dict[str, Dict[str, List[float]]], pattern_names: List[str]) -> None:
    """Plot comparison results."""
    plt.figure(figsize=(15, 5))
    
    # Hit rates plot
    plt.subplot(1, 2, 1)
    x = np.arange(len(pattern_names))
    width = 0.25
    offsets = np.linspace(-(len(results)-1)*width/2, (len(results)-1)*width/2, len(results))
    
    for (cache_name, cache_results), offset in zip(results.items(), offsets):
        plt.bar(x + offset, cache_results["hit_rates"], width, label=cache_name)
    
    plt.xlabel("Access Pattern")
    plt.ylabel("Hit Rate (%)")
    plt.title("Cache Hit Rates by Access Pattern")
    plt.xticks(x, pattern_names, rotation=45)
    plt.legend()
    plt.grid(True, alpha=0.3)
    
    # Execution time plot
    plt.subplot(1, 2, 2)
    for (cache_name, cache_results), offset in zip(results.items(), offsets):
        plt.bar(x + offset, cache_results["times"], width, label=cache_name)
    
    plt.xlabel("Access Pattern")
    plt.ylabel("Execution Time (s)")
    plt.title("Cache Execution Times by Access Pattern")
    plt.xticks(x, pattern_names, rotation=45)
    plt.legend()
    plt.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig("cache_comparison.png")
    plt.close()

def main():
    # Cache parameters
    CACHE_SIZE = 1000
    DATA_RANGE = 10000
    PATTERN_LENGTH = 100000
    
    # Define cache types to test
    cache_types = {
        "LRU": LRUCache,
        "LFU": LFUCache,
        "ARC": ARCache
    }
    
    # Run comparison
    compare_caches(cache_types, CACHE_SIZE, DATA_RANGE, PATTERN_LENGTH)
    print("\nResults have been plotted to 'cache_comparison.png'")

if __name__ == "__main__":
    main()
