/**
 * @file cache.cpp
 * @brief 缓存基类的实现文件
 * 
 * 这个文件提供了一些通用的缓存测试函数和工具函数。
 * 注意：主要的缓存实现在各个模板类的头文件中。
 */

#include "cache.hpp"
#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include "arc_cache.hpp"
#include <iostream>
#include <cassert>

/**
 * @brief 运行基本的缓存测试
 * @tparam Cache 缓存类型
 * @param cache 缓存对象
 * 
 * 这个函数运行一些基本的测试来验证缓存的基本功能是否正常
 */
template<typename Cache>
void run_basic_tests(Cache& cache) {
    // 测试1：插入和获取
    std::cout << "测试1：基本的插入和获取操作" << std::endl;
    cache.put(1, 100);
    cache.put(2, 200);
    cache.put(3, 300);
    
    int value;
    assert(cache.get(1, value) && value == 100);
    assert(cache.get(2, value) && value == 200);
    assert(cache.get(3, value) && value == 300);
    assert(!cache.get(4, value));
    
    // 测试2：更新现有值
    std::cout << "测试2：更新现有值" << std::endl;
    cache.put(1, 150);
    assert(cache.get(1, value) && value == 150);
    
    // 测试3：清空缓存
    std::cout << "测试3：清空缓存" << std::endl;
    cache.clear();
    assert(cache.size() == 0);
    assert(!cache.get(1, value));
    assert(!cache.get(2, value));
    assert(!cache.get(3, value));
    
    std::cout << "所有基本测试通过！" << std::endl;
}

/**
 * @brief 测试缓存替换策略
 * @tparam Cache 缓存类型
 * @param cache 缓存对象
 * @param cache_name 缓存名称（用于输出）
 */
template<typename Cache>
void test_replacement_policy(Cache& cache, const std::string& cache_name) {
    std::cout << "\n测试" << cache_name << "的替换策略：" << std::endl;
    
    // 填满缓存
    for (int i = 0; i < 5; ++i) {
        cache.put(i, i * 100);
    }
    
    // 访问一些项以影响替换策略
    int value;
    cache.get(0, value);  // 访问最早的项
    cache.get(2, value);  // 多次访问中间的项
    cache.get(2, value);
    cache.get(4, value);  // 访问最新的项
    
    // 插入新项，触发替换
    cache.put(5, 500);
    
    // 检查哪些项还在缓存中
    std::cout << "缓存中的项：";
    for (int i = 0; i <= 5; ++i) {
        if (cache.get(i, value)) {
            std::cout << i << "(" << value << ") ";
        }
    }
    std::cout << std::endl;
}

/**
 * @brief 主函数，运行所有缓存测试
 */
int main() {
    // 创建不同类型的缓存，容量为5
    LRUCache<int, int> lru_cache(5);
    LFUCache<int, int> lfu_cache(5);
    ARCache<int, int> arc_cache(5);
    
    // 运行基本测试
    std::cout << "\n=== 测试 LRU 缓存 ===" << std::endl;
    run_basic_tests(lru_cache);
    
    std::cout << "\n=== 测试 LFU 缓存 ===" << std::endl;
    run_basic_tests(lfu_cache);
    
    std::cout << "\n=== 测试 ARC 缓存 ===" << std::endl;
    run_basic_tests(arc_cache);
    
    // 测试替换策略
    test_replacement_policy(lru_cache, "LRU");
    test_replacement_policy(lfu_cache, "LFU");
    test_replacement_policy(arc_cache, "ARC");
    
    return 0;
}
