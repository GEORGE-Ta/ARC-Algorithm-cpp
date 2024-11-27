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
// Random access pattern
std::vector<int> generate_random_access_pattern(int data_range, int pattern_length, int seed = 42) {
    std::vector<int> pattern;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dist(0, data_range - 1);
    for (int i = 0; i < pattern_length; i++) {
        pattern.push_back(dist(gen));
    }
    return pattern;
}

// Locality access pattern (sequential access with some randomness)
std::vector<int> generate_locality_access_pattern(int data_range, int pattern_length, int locality_size, int seed = 42) {
    std::vector<int> pattern;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dist(0, data_range - locality_size);
    std::uniform_int_distribution<> local_dist(0, locality_size - 1);
    for (int i = 0; i < pattern_length; i++) {
        int base = dist(gen);
        pattern.push_back(base + local_dist(gen));
    }
    return pattern;
}

// Periodic access pattern
std::vector<int> generate_periodic_access_pattern(int data_range, int pattern_length, int period, int seed = 42) {
    std::vector<int> pattern;
    for (int i = 0; i < pattern_length; i++) {
        pattern.push_back((i % period) % data_range);
    }
    return pattern;
}

// Zipfian access pattern
std::vector<int> generate_zipfian_access_pattern(int data_range, int pattern_length, double skew, int seed = 42) {
    std::vector<int> pattern;
    std::mt19937 gen(seed);
    double denom = 0.0;
    for (int i = 1; i <= data_range; i++) {
        denom += 1.0 / pow(i, skew);
    }
    std::vector<double> probabilities(data_range + 1, 0.0);
    probabilities[0] = 0.0;
    for (int i = 1; i <= data_range; i++) {
        probabilities[i] = probabilities[i - 1] + (1.0 / pow(i, skew)) / denom;
    }
    std::uniform_real_distribution<> dist(0.0, 1.0);
    for (int i = 0; i < pattern_length; i++) {
        double p = dist(gen);
        int low = 1, high = data_range;
        while (low < high) {
            int mid = (low + high) / 2;
            if (probabilities[mid] >= p)
                high = mid;
            else
                low = mid + 1;
        }
        pattern.push_back(low - 1); // 索引从 0 开始
    }
    return pattern;
}
int main() {
    const int DATA_RANGE = 1000;
    const int PATTERN_LENGTH = 10000;
    const std::vector<int> CACHE_SIZES = {50, 100, 200}; // 不同的缓存容量
    const std::vector<int> LOCALITY_SIZES = {10, 50, 100}; // 对于局部性访问模式
    const std::vector<int> PERIODS = {100, 200, 500}; // 对于周期性访问模式
    const std::vector<double> ZIPF_SKEWS = {0.5, 1.0, 1.5}; // 对于 Zipf 分布
    
    // 定义缓存策略名称和对应的构造函数
    std::map<std::string, std::function<Cache<int, int>*(int)>> cache_factories = {
        {"ARC", [](int size) { return new ARCache<int, int>(size); }},
        {"LRU", [](int size) { return new LRUCache<int, int>(size); }},
        {"LFU", [](int size) { return new LFUCache<int, int>(size); }}
    };
    
    // 存储实验结果
    struct Result {
        std::string pattern_type;
        int cache_size;
        std::string cache_type;
        double hit_rate;
    };
    std::vector<Result> results;
    
    // 运行实验
    for (const auto& cache_size : CACHE_SIZES) {
        // 测试随机访问模式
        {
            std::vector<int> access_pattern = generate_random_access_pattern(DATA_RANGE, PATTERN_LENGTH);
            for (const auto& cache_pair : cache_factories) {
                Cache<int, int>* cache = cache_pair.second(cache_size);
                double hit_rate = test_cache_scenario(*cache, access_pattern);
                results.push_back({"Random", cache_size, cache_pair.first, hit_rate});
                delete cache;
            }
        }
        // 测试局部性访问模式
        for (const auto& locality_size : LOCALITY_SIZES) {
            std::vector<int> access_pattern = generate_locality_access_pattern(DATA_RANGE, PATTERN_LENGTH, locality_size);
            for (const auto& cache_pair : cache_factories) {
                Cache<int, int>* cache = cache_pair.second(cache_size);
                double hit_rate = test_cache_scenario(*cache, access_pattern);
                results.push_back({"Locality(" + std::to_string(locality_size) + ")", cache_size, cache_pair.first, hit_rate});
                delete cache;
            }
        }
        // 测试周期性访问模式
        for (const auto& period : PERIODS) {
            std::vector<int> access_pattern = generate_periodic_access_pattern(DATA_RANGE, PATTERN_LENGTH, period);
            for (const auto& cache_pair : cache_factories) {
                Cache<int, int>* cache = cache_pair.second(cache_size);
                double hit_rate = test_cache_scenario(*cache, access_pattern);
                results.push_back({"Periodic(" + std::to_string(period) + ")", cache_size, cache_pair.first, hit_rate});
                delete cache;
            }
        }
        // 测试 Zipf 分布访问模式
        for (const auto& skew : ZIPF_SKEWS) {
            std::vector<int> access_pattern = generate_zipfian_access_pattern(DATA_RANGE, PATTERN_LENGTH, skew);
            for (const auto& cache_pair : cache_factories) {
                Cache<int, int>* cache = cache_pair.second(cache_size);
                double hit_rate = test_cache_scenario(*cache, access_pattern);
                results.push_back({"Zipf(" + std::to_string(skew) + ")", cache_size, cache_pair.first, hit_rate});
                delete cache;
            }
        }
    }
    
    // 输出结果
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Cache Performance Results:\n";
    std::cout << "Pattern\t\tCache Size\tCache Type\tHit Rate (%)\n";
    for (const auto& result : results) {
        std::cout << result.pattern_type << "\t"
                  << result.cache_size << "\t\t"
                  << result.cache_type << "\t\t"
                  << result.hit_rate * 100 << "%\n";
    }
    
    return 0;
}
