#ifndef ARC_HPP
#define ARC_HPP

/**
 * @file arc.hpp
 * @brief Adaptive Replacement Cache (ARC) 算法实现
 * 
 * ARC算法是一种自适应的页面替换算法，它通过维护四个双向链表（T1、T2、B1、B2）
 * 来实现对不同访问模式的自适应。该算法能够在扫描和循环访问模式下都表现良好。
 */

#include <unordered_map>
#include <list>
#include <cassert>
#include <algorithm>

template<typename K, typename V>
class ARCache {
private:
    // ====== 核心参数定义 ======
    size_t capacity;  // 缓存总容量
    double p;         // 自适应参数，用于动态调整T1和T2的目标大小

    // ====== 数据结构定义 ======
    // 缓存空间（Cache）
    std::list<K> t1;  // T1: 存储最近只被访问一次的页面
    std::list<K> t2;  // T2: 存储最近被多次访问的页面（热点页面）
    
    // 历史信息空间（Ghost Cache）
    std::list<K> b1;  // B1: 存储从T1中淘汰的页面的历史信息
    std::list<K> b2;  // B2: 存储从T2中淘汰的页面的历史信息

    // 映射表
    // cache: 存储实际的键值对和对应的位置信息
    std::unordered_map<K, std::pair<V, typename std::list<K>::iterator>> cache;
    // ghost: 仅存储在B1或B2中的键的位置信息
    std::unordered_map<K, typename std::list<K>::iterator> ghost;

    /**
     * @brief 页面替换核心算法
     * @param key 要插入的新页面的键
     * 
     * 该函数实现了ARC的核心替换策略：
     * 1. 根据页面在B1或B2中的位置调整p值
     * 2. 根据当前各个列表的大小选择合适的替换策略
     * 3. 维护ghost cache的大小限制
     */
    void replace(const K& key) {
        // 检查页面是否在历史信息空间中
        bool in_b1 = (ghost.find(key) != ghost.end() && 
                     std::find(b1.begin(), b1.end(), key) != b1.end());
        bool in_b2 = (ghost.find(key) != ghost.end() && 
                     std::find(b2.begin(), b2.end(), key) != b2.end());

        // 自适应调整p值
        if (in_b1) {
            // 如果在B1中命中，增加p值，倾向于保留更多的一次访问页面
            p = std::min(p + 1.0, static_cast<double>(capacity));
        } else if (in_b2) {
            // 如果在B2中命中，减少p值，倾向于保留更多的多次访问页面
            p = std::max(p - 1.0, 0.0);
        }

        // 替换策略实现
        if (t1.size() + b1.size() >= capacity) {
            if (t1.size() < capacity) {
                // 如果T1未满但T1+B1已满，从B1中删除最后一个条目
                b1.pop_back();
                ghost.erase(b1.back());
                replace_lru();  // 执行LRU替换
            } else {
                // 如果T1已满，直接从T1中删除最后一个条目
                auto lru = t1.back();
                t1.pop_back();
                cache.erase(lru);
            }
        } else if (t1.size() + t2.size() + b1.size() + b2.size() >= 2 * capacity) {
            // 确保ghost cache (B1+B2)的总大小不超过缓存容量
            if (t1.size() + t2.size() + b1.size() + b2.size() >= 2 * capacity) {
                b2.pop_back();
                ghost.erase(b2.back());
            }
        }
    }

    /**
     * @brief LRU替换策略实现
     * 
     * 该函数根据p值和当前状态决定从T1还是T2中淘汰页面：
     * 1. 如果T1大小超过p或者在B2中发现T1的LRU页面，从T1中淘汰
     * 2. 否则从T2中淘汰
     */
    void replace_lru() {
        if (!t1.empty() && ((ghost.find(t1.back()) != ghost.end() && 
            std::find(b2.begin(), b2.end(), t1.back()) != b2.end()) || 
            t1.size() > p)) {
            // 从T1移动到B1
            K lru = t1.back();
            t1.pop_back();
            b1.push_front(lru);  // 保持历史信息
            ghost[lru] = b1.begin();
            cache.erase(lru);
        } else {
            // 从T2移动到B2
            K lru = t2.back();
            t2.pop_back();
            b2.push_front(lru);  // 保持历史信息
            ghost[lru] = b2.begin();
            cache.erase(lru);
        }
    }

public:
    /**
     * @brief 构造函数
     * @param size 缓存容量
     */
    explicit ARCache(size_t size) : capacity(size), p(0.0) {}

    /**
     * @brief 获取缓存中的值
     * @param key 要查找的键
     * @param value 输出参数，存储找到的值
     * @return 如果找到则返回true，否则返回false
     * 
     * 实现了ARC的页面访问策略：
     * 1. 如果页面在T1中，移动到T2（变为热点页面）
     * 2. 如果页面在T2中，移动到T2的头部（保持热点状态）
     */
    bool get(const K& key, V& value) {
        auto it = cache.find(key);
        if (it == cache.end()) {
            return false;  // 缓存未命中
        }

        // 页面访问处理
        if (std::find(t1.begin(), t1.end(), key) != t1.end()) {
            // 从T1升级到T2
            t1.erase(it->second.second);
            t2.push_front(key);
            it->second.second = t2.begin();
        } else {
            // 在T2中前移
            t2.erase(it->second.second);
            t2.push_front(key);
            it->second.second = t2.begin();
        }

        value = it->second.first;
        return true;  // 缓存命中
    }

    /**
     * @brief 将键值对放入缓存
     * @param key 键
     * @param value 值
     * 
     * 实现了ARC的页面放入策略：
     * 1. 如果是更新现有条目，按get操作处理
     * 2. 如果缓存满，执行替换策略
     * 3. 根据页面在ghost cache中的位置决定放入T1还是T2
     */
    void put(const K& key, const V& value) {
        auto cache_it = cache.find(key);
        if (cache_it != cache.end()) {
            // 更新现有条目
            cache_it->second.first = value;
            if (std::find(t1.begin(), t1.end(), key) != t1.end()) {
                // 从T1升级到T2
                t1.erase(cache_it->second.second);
                t2.push_front(key);
                cache_it->second.second = t2.begin();
            } else {
                // 在T2中前移
                t2.erase(cache_it->second.second);
                t2.push_front(key);
                cache_it->second.second = t2.begin();
            }
            return;
        }

        // 处理缓存满的情况
        if (cache.size() >= capacity) {
            replace(key);
        }

        // 根据ghost cache状态决定放入策略
        auto ghost_it = ghost.find(key);
        if (ghost_it != ghost.end()) {
            if (std::find(b1.begin(), b1.end(), key) != b1.end()) {
                // 从B1升级到T2
                b1.erase(ghost_it->second);
                ghost.erase(ghost_it);
                t2.push_front(key);
                cache[key] = {value, t2.begin()};
            } else {
                // 从B2重新加入T2
                b2.erase(ghost_it->second);
                ghost.erase(ghost_it);
                t2.push_front(key);
                cache[key] = {value, t2.begin()};
            }
        } else {
            // 新页面放入T1
            t1.push_front(key);
            cache[key] = {value, t1.begin()};
        }
    }

    /**
     * @brief 获取当前缓存大小
     * @return 缓存中的条目数
     */
    size_t size() const {
        return cache.size();
    }

    /**
     * @brief 检查缓存是否为空
     * @return 如果缓存为空返回true，否则返回false
     */
    bool empty() const {
        return cache.empty();
    }

    /**
     * @brief 清空缓存
     * 重置所有数据结构和参数到初始状态
     */
    void clear() {
        t1.clear();
        t2.clear();
        b1.clear();
        b2.clear();
        cache.clear();
        ghost.clear();
        p = 0.0;
    }
};

#endif // ARC_HPP
