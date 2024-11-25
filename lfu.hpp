#ifndef LFU_HPP
#define LFU_HPP

/**
 * @file lfu.hpp
 * @brief Least Frequently Used (LFU) 缓存算法实现
 * 
 * LFU算法基于"最少使用频率"原则，当缓存满时淘汰访问次数最少的页面。
 * 如果有多个页面访问次数相同，则淘汰最早的那个。
 */

#include <unordered_map>
#include <list>

template<typename K, typename V>
class LFUCache {
private:
    size_t capacity;  // 缓存容量
    size_t minFreq;   // 当前最小频率

    // 存储键值对和频率信息的节点结构
    struct Node {
        V value;
        size_t freq;
        typename std::list<K>::iterator iter;
    };

    // 主缓存，存储键到节点的映射
    std::unordered_map<K, Node> cache;
    // 频率到键列表的映射
    std::unordered_map<size_t, std::list<K>> freq_lists;

    /**
     * @brief 更新节点频率
     * @param key 要更新的键
     * @param node 节点引用
     */
    void update_freq(const K& key, Node& node) {
        // 从原频率列表中删除
        freq_lists[node.freq].erase(node.iter);
        
        // 如果当前频率的列表为空，且是最小频率，更新最小频率
        if (freq_lists[node.freq].empty()) {
            freq_lists.erase(node.freq);
            if (minFreq == node.freq) {
                minFreq = node.freq + 1;
            }
        }

        // 增加频率并添加到新频率列表
        node.freq++;
        freq_lists[node.freq].push_front(key);
        node.iter = freq_lists[node.freq].begin();
    }

public:
    /**
     * @brief 构造函数
     * @param size 缓存容量
     */
    explicit LFUCache(size_t size) : capacity(size), minFreq(0) {}

    /**
     * @brief 获取缓存中的值
     * @param key 要查找的键
     * @param value 输出参数，存储找到的值
     * @return 如果找到则返回true，否则返回false
     */
    bool get(const K& key, V& value) {
        auto it = cache.find(key);
        if (it == cache.end()) {
            return false;  // 缓存未命中
        }

        // 更新访问频率并返回值
        value = it->second.value;
        update_freq(key, it->second);
        return true;  // 缓存命中
    }

    /**
     * @brief 将键值对放入缓存
     * @param key 键
     * @param value 值
     */
    void put(const K& key, const V& value) {
        // 如果容量为0，直接返回
        if (capacity == 0) return;

        auto it = cache.find(key);
        if (it != cache.end()) {
            // 更新现有条目
            it->second.value = value;
            update_freq(key, it->second);
            return;
        }

        // 如果缓存已满，删除最少使用的条目
        if (cache.size() >= capacity) {
            // 获取最小频率的最后一个键
            auto& min_freq_list = freq_lists[minFreq];
            K lfu_key = min_freq_list.back();
            min_freq_list.pop_back();
            
            if (min_freq_list.empty()) {
                freq_lists.erase(minFreq);
            }
            
            cache.erase(lfu_key);
        }

        // 插入新条目
        minFreq = 1;  // 新项的频率为1
        freq_lists[minFreq].push_front(key);
        cache[key] = {value, minFreq, freq_lists[minFreq].begin()};
    }

    /**
     * @brief 获取当前缓存大小
     */
    size_t size() const {
        return cache.size();
    }

    /**
     * @brief 检查缓存是否为空
     */
    bool empty() const {
        return cache.empty();
    }

    /**
     * @brief 清空缓存
     */
    void clear() {
        cache.clear();
        freq_lists.clear();
        minFreq = 0;
    }
};

#endif // LFU_HPP
