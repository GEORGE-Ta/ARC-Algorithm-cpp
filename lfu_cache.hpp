/**
 * @file lfu_cache.hpp
 * @brief LFU（最不经常使用）缓存的实现
 * 
 * LFU缓存跟踪每个项目的访问频率，
 * 当缓存满时，删除访问频率最低的项目。
 * 如果有多个最低频率的项目，删除最早的那个。
 */

#ifndef LFU_CACHE_HPP
#define LFU_CACHE_HPP

#include "cache.hpp"
#include <unordered_map>
#include <map>

/**
 * @brief LFU缓存类
 * @tparam K 键的类型
 * @tparam V 值的类型
 */
template<typename K, typename V>
class LFUCache : public Cache<K, V> {
private:
    size_t capacity;  // 缓存容量
    size_t minFreq;   // 当前最小频率
    std::unordered_map<K, std::pair<V, size_t>> keyToVal;  // 键 -> {值, 频率}
    std::unordered_map<K, typename std::list<K>::iterator> keyToIter;  // 键 -> 在频率列表中的迭代器
    std::map<size_t, std::list<K>> freqToKeys;  // 频率 -> 具有该频率的键的列表

    /**
     * @brief 增加键的访问频率
     * @param key 要增加频率的键
     * 
     * 将键从当前频率列表移到下一个频率列表
     * 如果当前频率列表为空且是最小频率，更新最小频率
     */
    void increment(const K& key) {
        size_t freq = keyToVal[key].second;
        auto iter = keyToIter[key];
        freqToKeys[freq].erase(iter);
        
        if (freqToKeys[freq].empty()) {
            freqToKeys.erase(freq);
            if (minFreq == freq) minFreq++;
        }
        
        freq++;
        freqToKeys[freq].push_front(key);
        keyToIter[key] = freqToKeys[freq].begin();
        keyToVal[key].second = freq;
    }

public:
    /**
     * @brief 构造函数
     * @param size 缓存的最大容量
     */
    explicit LFUCache(size_t size) : capacity(size), minFreq(0) {}

    /**
     * @brief 插入或更新一个键值对
     * @param key 键
     * @param value 值
     * 
     * 如果键已存在，更新值并增加频率
     * 如果键不存在且缓存已满，删除最低频率的项目
     */
    void put(const K& key, const V& value) override {
        if (capacity == 0) return;

        if (keyToVal.count(key)) {
            keyToVal[key].first = value;
            increment(key);
            return;
        }

        if (keyToVal.size() >= capacity) {
            K evictKey = freqToKeys[minFreq].back();
            freqToKeys[minFreq].pop_back();
            if (freqToKeys[minFreq].empty()) {
                freqToKeys.erase(minFreq);
            }
            keyToVal.erase(evictKey);
            keyToIter.erase(evictKey);
        }

        keyToVal[key] = {value, 1};
        freqToKeys[1].push_front(key);
        keyToIter[key] = freqToKeys[1].begin();
        minFreq = 1;
    }

    /**
     * @brief 获取键对应的值
     * @param key 要查找的键
     * @param value 如果找到，值将被存储在这里
     * @return 如果找到键返回true，否则返回false
     * 
     * 如果找到键，会增加其访问频率
     */
    bool get(const K& key, V& value) override {
        if (keyToVal.count(key) == 0) {
            return false;
        }
        value = keyToVal[key].first;
        increment(key);
        return true;
    }

    /**
     * @brief 获取当前缓存中的项目数量
     */
    size_t size() const override {
        return keyToVal.size();
    }

    /**
     * @brief 清空缓存
     */
    void clear() override {
        keyToVal.clear();
        keyToIter.clear();
        freqToKeys.clear();
        minFreq = 0;
    }
};

#endif // LFU_CACHE_HPP
