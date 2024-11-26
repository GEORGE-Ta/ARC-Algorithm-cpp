/**
 * @file test_cache.cpp
 * @brief 缓存性能测试程序
 * 
 * 这个程序测试并比较了LRU、LFU和ARC三种缓存策略的性能。
 * 通过生成特定的访问模式来展示各种缓存策略的优缺点。
 */

#include "arc_cache.hpp"
#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>

/**
 * @brief 测试缓存的命中率
 * @param cache 要测试的缓存对象
 * @param access_pattern 访问模式序列
 * @return 缓存命中率（命中次数/总访问次数）
 */
template<typename Cache>
double test_cache_scenario(Cache& cache, const std::vector<int>& access_pattern) {
    int hits = 0;
    int total = 0;
    
    for (int key : access_pattern) {
        int value;
        if (cache.get(key, value)) {
            hits++;  // 缓存命中
        } else {
            cache.put(key, key);  // 缓存未命中，插入新值
        }
        total++;
    }
    
    return static_cast<double>(hits) / total;
}

/**
 * @brief 生成测试用的访问模式
 * @param size 可能的键的范围（0到size-1）
 * @param pattern_length 要生成的访问序列长度
 * @return 访问序列
 * 
 * 生成的访问模式包含三种类型的访问：
 * 1. 频繁访问的项（有利于LFU）
 * 2. 最近访问的项（有利于LRU）
 * 3. 随机访问的项（测试适应性）
 */
std::vector<int> generate_access_pattern(int size, int pattern_length) {
    std::vector<int> pattern;
    std::mt19937 gen(42);  // 使用固定的种子以保证结果可重现
    
    // 生成频繁访问的项（占用1/4的键空间）
    std::vector<int> frequent_items;
    for (int i = 0; i < size/4; i++) {
        frequent_items.push_back(i);
    }
    
    // 生成最近访问的项（占用1/4的键空间）
    std::vector<int> recent_items;
    for (int i = size/2; i < size/2 + size/4; i++) {
        recent_items.push_back(i);
    }
    
    // 用于生成随机访问
    std::uniform_int_distribution<> random_dist(0, size-1);
    std::uniform_int_distribution<> pattern_dist(0, 2);
    
    // 生成访问序列
    for (int i = 0; i < pattern_length; i++) {
        int pattern_type = pattern_dist(gen);
        
        if (pattern_type == 0) {
            // 添加频繁访问的项
            pattern.push_back(frequent_items[i % frequent_items.size()]);
        }
        else if (pattern_type == 1) {
            // 添加最近访问的项
            pattern.push_back(recent_items[i % recent_items.size()]);
        }
        else {
            // 添加随机访问的项
            pattern.push_back(random_dist(gen));
        }
    }
    
    return pattern;
}

int main() {
    // 测试参数设置
    const int cache_size = 100;        // 缓存大小
    const int key_space = 1000;        // 可能的键的范围
    const int pattern_length = 10000;  // 访问序列长度
    
    // 生成测试用的访问模式
    std::vector<int> access_pattern = generate_access_pattern(key_space, pattern_length);
    
    // 创建三种不同的缓存
    LRUCache<int, int> lru_cache(cache_size);
    LFUCache<int, int> lfu_cache(cache_size);
    ARCache<int, int> arc_cache(cache_size);
    
    // 测试并输出结果
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "缓存大小: " << cache_size << std::endl;
    std::cout << "键空间大小: " << key_space << std::endl;
    std::cout << "访问序列长度: " << pattern_length << std::endl;
    std::cout << "\n各种缓存策略的命中率：" << std::endl;
    std::cout << "LRU: " << test_cache_scenario(lru_cache, access_pattern) << std::endl;
    std::cout << "LFU: " << test_cache_scenario(lfu_cache, access_pattern) << std::endl;
    std::cout << "ARC: " << test_cache_scenario(arc_cache, access_pattern) << std::endl;
    
    return 0;
}
