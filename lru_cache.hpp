/**
 * @file lru_cache.hpp
 * @brief LRU（最近最少使用）缓存的实现
 * 
 * LRU缓存保留最近使用过的数据，当缓存满时，
 * 会删除最长时间没有被访问的数据。
 */

#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include "cache.hpp"
#include <unordered_map>
#include <list>

/**
 * @brief LRU缓存类
 * @tparam K 键的类型
 * @tparam V 值的类型
 */
template<typename K, typename V>
class LRUCache : public Cache<K, V> {
private:
    size_t capacity;  // 缓存容量
    std::list<std::pair<K, V>> cache_list;  // 双向链表，存储键值对，最近使用的在前面
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> cache_map;  // 哈希表，存储键到链表节点的映射

public:
    /**
     * @brief 构造函数
     * @param size 缓存的最大容量
     */
    explicit LRUCache(size_t size) : capacity(size) {}

    /**
     * @brief 插入或更新一个键值对
     * @param key 键
     * @param value 值
     * 
     * 如果键已存在，更新值并将其移到最前面
     * 如果键不存在且缓存已满，删除最后一个元素
     */
    void put(const K& key, const V& value) override {
        auto it = cache_map.find(key);
        if (it != cache_map.end()) {
            cache_list.erase(it->second);
            cache_map.erase(it);
        }
        else if (cache_list.size() >= capacity) {
            cache_map.erase(cache_list.back().first);
            cache_list.pop_back();
        }
        cache_list.push_front({key, value});
        cache_map[key] = cache_list.begin();
    }

    /**
     * @brief 获取键对应的值
     * @param key 要查找的键
     * @param value 如果找到，值将被存储在这里
     * @return 如果找到键返回true，否则返回false
     * 
     * 如果找到键，会将对应的项移到最前面
     */
    bool get(const K& key, V& value) override {
        auto it = cache_map.find(key);
        if (it == cache_map.end()) {
            return false;
        }
        value = it->second->second;
        cache_list.splice(cache_list.begin(), cache_list, it->second);
        return true;
    }

    /**
     * @brief 获取当前缓存中的项目数量
     */
    size_t size() const override {
        return cache_list.size();
    }

    /**
     * @brief 清空缓存
     */
    void clear() override {
        cache_list.clear();
        cache_map.clear();
    }
};

#endif // LRU_CACHE_HPP
