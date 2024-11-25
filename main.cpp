#include <iostream>
#include <string>
#include "arc.hpp"

int main() {
    // 创建一个容量为3的ARC缓存
    ARCache<int, std::string> cache(3);

    // 测试基本的插入操作
    std::cout << "Testing basic insertion:" << std::endl;
    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");

    std::string value;
    std::cout << "Getting key 1: " << (cache.get(1, value) ? value : "not found") << std::endl;
    std::cout << "Getting key 2: " << (cache.get(2, value) ? value : "not found") << std::endl;
    std::cout << "Getting key 3: " << (cache.get(3, value) ? value : "not found") << std::endl;

    // 测试缓存替换
    std::cout << "\nTesting cache replacement:" << std::endl;
    cache.put(4, "four");  // 这应该会触发替换
    std::cout << "Getting key 1: " << (cache.get(1, value) ? value : "not found") << std::endl;
    std::cout << "Getting key 4: " << (cache.get(4, value) ? value : "not found") << std::endl;

    // 测试频繁访问的情况
    std::cout << "\nTesting frequent access:" << std::endl;
    cache.get(2, value);
    cache.get(2, value);
    cache.get(3, value);
    cache.put(5, "five");  // 这应该会替换掉较少使用的项

    std::cout << "Getting key 2: " << (cache.get(2, value) ? value : "not found") << std::endl;
    std::cout << "Getting key 3: " << (cache.get(3, value) ? value : "not found") << std::endl;
    std::cout << "Getting key 4: " << (cache.get(4, value) ? value : "not found") << std::endl;
    std::cout << "Getting key 5: " << (cache.get(5, value) ? value : "not found") << std::endl;

    return 0;
}
