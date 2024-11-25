#include "arc_cache.hpp"
#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>

// Helper function to measure cache hit rate
template<typename Cache>
double test_cache_scenario(Cache& cache, const std::vector<int>& access_pattern) {
    int hits = 0;
    int total = 0;
    
    for (int key : access_pattern) {
        int value;
        if (cache.get(key, value)) {
            hits++;
        } else {
            cache.put(key, key);
        }
        total++;
    }
    
    return static_cast<double>(hits) / total;
}

// Helper function to generate access pattern that shows ARC's advantages
std::vector<int> generate_access_pattern(int size, int pattern_length) {
    std::vector<int> pattern;
    // Use fixed seed for reproducible results
    std::mt19937 gen(42);  // Fixed seed value
    
    // Generate a sequence that alternates between:
    // 1. Recently used items (favors LRU)
    // 2. Frequently used items (favors LFU)
    // 3. Some random items (tests adaptability)
    
    std::vector<int> frequent_items;
    for (int i = 0; i < size/4; i++) {
        frequent_items.push_back(i);
    }
    
    std::vector<int> recent_items;
    for (int i = size/2; i < size/2 + size/4; i++) {
        recent_items.push_back(i);
    }
    
    std::uniform_int_distribution<> random_dist(0, size-1);
    std::uniform_int_distribution<> pattern_dist(0, 2);
    
    for (int i = 0; i < pattern_length; i++) {
        int pattern_type = pattern_dist(gen);
        
        if (pattern_type == 0) {
            // Add frequent items
            pattern.push_back(frequent_items[i % frequent_items.size()]);
        }
        else if (pattern_type == 1) {
            // Add recent items
            pattern.push_back(recent_items[i % recent_items.size()]);
        }
        else {
            // Add random items
            pattern.push_back(random_dist(gen));
        }
    }
    
    return pattern;
}

int main() {
    const int CACHE_SIZE = 100;
    const int DATA_RANGE = 1000;
    const int PATTERN_LENGTH = 10000;
    
    // Create caches
    ARCache<int, int> arc_cache(CACHE_SIZE);
    LRUCache<int, int> lru_cache(CACHE_SIZE);
    LFUCache<int, int> lfu_cache(CACHE_SIZE);
    
    // Generate access pattern
    std::vector<int> access_pattern = generate_access_pattern(DATA_RANGE, PATTERN_LENGTH);
    
    // Test each cache
    std::cout << "Testing cache performance with:" << std::endl;
    std::cout << "Cache size: " << CACHE_SIZE << std::endl;
    std::cout << "Data range: " << DATA_RANGE << std::endl;
    std::cout << "Access pattern length: " << PATTERN_LENGTH << std::endl << std::endl;
    
    double arc_hit_rate = test_cache_scenario(arc_cache, access_pattern);
    double lru_hit_rate = test_cache_scenario(lru_cache, access_pattern);
    double lfu_hit_rate = test_cache_scenario(lfu_cache, access_pattern);
    
    // Print results
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Hit rates:" << std::endl;
    std::cout << "ARC: " << arc_hit_rate * 100 << "%" << std::endl;
    std::cout << "LRU: " << lru_hit_rate * 100 << "%" << std::endl;
    std::cout << "LFU: " << lfu_hit_rate * 100 << "%" << std::endl;
    
    return 0;
}
