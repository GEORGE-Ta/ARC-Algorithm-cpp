#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include "arc.hpp"
#include "lru.hpp"
#include "lfu.hpp"

// 测试场景类型
enum class TestPattern {
    SEQUENTIAL,      // 顺序访问
    RANDOM,         // 随机访问
    LOOP,           // 循环访问
    MIXED           // 混合访问模式
};

// 生成测试数据
std::vector<int> generate_test_data(TestPattern pattern, size_t size, size_t range) {
    std::vector<int> data;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, range - 1);

    switch (pattern) {
        case TestPattern::SEQUENTIAL:
            // 顺序访问
            for (size_t i = 0; i < size; ++i) {
                data.push_back(i % range);
            }
            break;

        case TestPattern::RANDOM:
            // 随机访问
            for (size_t i = 0; i < size; ++i) {
                data.push_back(dis(gen));
            }
            break;

        case TestPattern::LOOP:
            // 循环访问（重复一个小序列）
            {
                size_t loop_size = range / 4;
                for (size_t i = 0; i < size; ++i) {
                    data.push_back(i % loop_size);
                }
            }
            break;

        case TestPattern::MIXED:
            // 混合访问模式
            {
                std::vector<int> mixed_data;
                // 20%顺序访问
                for (size_t i = 0; i < size / 5; ++i) {
                    mixed_data.push_back(i % range);
                }
                // 40%随机访问
                for (size_t i = 0; i < 2 * size / 5; ++i) {
                    mixed_data.push_back(dis(gen));
                }
                // 40%循环访问
                size_t loop_size = range / 4;
                for (size_t i = 0; i < 2 * size / 5; ++i) {
                    mixed_data.push_back(i % loop_size);
                }
                // 随机打乱混合数据
                std::shuffle(mixed_data.begin(), mixed_data.end(), gen);
                data = mixed_data;
            }
            break;
    }
    return data;
}

// 测试函数模板
template<typename Cache>
double test_cache(Cache& cache, const std::vector<int>& data) {
    size_t hits = 0;
    int dummy = 0;

    for (int key : data) {
        if (cache.get(key, dummy)) {
            hits++;
        } else {
            cache.put(key, key);
        }
    }

    return static_cast<double>(hits) / data.size();
}

// 运行所有测试场景
void run_tests(size_t cache_size, size_t data_size, size_t key_range) {
    std::vector<std::pair<TestPattern, std::string>> patterns = {
        {TestPattern::SEQUENTIAL, "Sequential"},
        {TestPattern::RANDOM, "Random"},
        {TestPattern::LOOP, "Loop"},
        {TestPattern::MIXED, "Mixed"}
    };

    std::cout << std::setw(15) << "Pattern" 
              << std::setw(15) << "ARC" 
              << std::setw(15) << "LRU"
              << std::setw(15) << "LFU" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    for (const auto& [pattern, name] : patterns) {
        try {
            // 生成测试数据
            auto data = generate_test_data(pattern, data_size, key_range);

            // 测试每种算法
            ARCache<int, int> arc(cache_size);
            LRUCache<int, int> lru(cache_size);
            LFUCache<int, int> lfu(cache_size);

            double arc_hit_rate = test_cache(arc, data);
            double lru_hit_rate = test_cache(lru, data);
            double lfu_hit_rate = test_cache(lfu, data);

            // 打印结果
            std::cout << std::fixed << std::setprecision(4)
                      << std::setw(15) << name
                      << std::setw(15) << arc_hit_rate
                      << std::setw(15) << lru_hit_rate
                      << std::setw(15) << lfu_hit_rate << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error testing " << name << ": " << e.what() << std::endl;
        }
    }
}

int main() {
    // 测试参数
    const size_t CACHE_SIZE = 20;     // 缓存大小（等于键的范围）
    const size_t DATA_SIZE = 100;    // 测试数据量
    const size_t KEY_RANGE = 20;     // 键的范围

    std::cout << "Cache Size: " << CACHE_SIZE << std::endl;
    std::cout << "Data Size: " << DATA_SIZE << std::endl;
    std::cout << "Key Range: " << KEY_RANGE << std::endl;
    std::cout << std::endl;

    try {
        run_tests(CACHE_SIZE, DATA_SIZE, KEY_RANGE);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
