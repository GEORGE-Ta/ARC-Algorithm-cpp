#ifndef LRU_HPP
#define LRU_HPP

/**
 * @file lru.hpp
 * @brief Least Recently Used (LRU) 缓存算法实现
 * 
 * LRU算法基于"最近最少使用"原则，当缓存满时淘汰最长时间未被访问的页面。
 * 使用双向链表和哈希表实现O(1)的访问和更新操作。
 */

#include <unordered_map>
#include <list>

template<typename K, typename V>
class LRUCache {
private:
    size_t capacity;  // 缓存容量
    std::list<K> cache_list;  // 双向链表存储键，最近使用的在前面
    std::unordered_map<K, std::pair<V, typename std::list<K>::iterator>> cache_map;  // 哈希表存储键到值和链表节点的映射

public:
    /**
     * @brief 构造函数
     * @param size 缓存容量
     */
    explicit LRUCache(size_t size) : capacity(size) {}

    /**
     * @brief 获取缓存中的值
     * @param key 要查找的键
     * @param value 输出参数，存储找到的值
     * @return 如果找到则返回true，否则返回false
     */
    bool get(const K& key, V& value) {
        auto it = cache_map.find(key);
        if (it == cache_map.end()) {
            return false;  // 缓存未命中
        }

        // 将访问的节点移到链表头部
        cache_list.erase(it->second.second);
        cache_list.push_front(key);
        it->second.second = cache_list.begin();
        value = it->second.first;
        return true;  // 缓存命中
    }

    /**
     * @brief 将键值对放入缓存
     * @param key 键
     * @param value 值
     */
    void put(const K& key, const V& value) {
        auto it = cache_map.find(key);
        if (it != cache_map.end()) {
            // 更新现有条目
            it->second.first = value;
            cache_list.erase(it->second.second);
            cache_list.push_front(key);
            it->second.second = cache_list.begin();
            return;
        }

        // 如果缓存已满，删除最久未使用的条目
        if (cache_map.size() >= capacity) {
            cache_map.erase(cache_list.back());
            cache_list.pop_back();
        }

        // 在链表头部插入新条目
        cache_list.push_front(key);
        cache_map[key] = {value, cache_list.begin()};
    }

    /**
     * @brief 获取当前缓存大小
     */
    size_t size() const {
        return cache_map.size();
    }

    /**
     * @brief 检查缓存是否为空
     */
    bool empty() const {
        return cache_map.empty();
    }

    /**
     * @brief 清空缓存
     */
    void clear() {
        cache_list.clear();
        cache_map.clear();
    }
};

#endif // LRU_HPP
