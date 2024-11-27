/**
 * @file arc_cache.hpp
 * @brief ARC（自适应替换缓存）的实现
 * 
 * ARC是一种自适应的缓存替换算法，它结合了LRU和LFU的优点。
 * 通过维护两个LRU列表（T1和T2）和它们对应的影子列表（B1和B2），
 * 动态调整策略以适应不同的访问模式。
 */

#ifndef ARC_CACHE_HPP
#define ARC_CACHE_HPP

#include "cache.hpp"
#include <unordered_map>
#include <list>

/**
 * @brief ARC缓存类
 * @tparam K 键的类型
 * @tparam V 值的类型
 */
template<typename K, typename V>
class ARCache : public Cache<K, V> {
private:
    size_t capacity;  // 缓存总容量
    size_t p;         // T1的目标大小，动态调整
    
    std::list<K> t1;  // 最近一次使用的页面列表
    std::unordered_map<K, std::pair<V, typename std::list<K>::iterator>> t1_map;
    
    std::list<K> t2;  // 至少使用两次的页面列表
    std::unordered_map<K, std::pair<V, typename std::list<K>::iterator>> t2_map;
    
    std::list<K> b1;  // T1的影子列表，存储从T1移出的页面的"ghost"条目
    std::unordered_map<K, typename std::list<K>::iterator> b1_map;
    
    std::list<K> b2;  // T2的影子列表，存储从T2移出的页面的"ghost"条目
    std::unordered_map<K, typename std::list<K>::iterator> b2_map;

    /**
     * @brief 替换策略的核心实现
     * @param in_b2 是否在B2中找到了页面
     * 
     * 根据当前状态和参数决定从T1还是T2中移除页面
     */
    void replace(bool in_b2) {
        if (!t1.empty() && ((t1.size() > p) || (in_b2 && t1.size() == p))) {
            // 从T1移除
            K lru_key = t1.back();
            t1.pop_back();
            V val = t1_map[lru_key].first;
            t1_map.erase(lru_key);
            
            // 添加到B1
            b1.push_front(lru_key);
            b1_map[lru_key] = b1.begin();
            
            // 如果B1太大，移除最旧的条目
            if (b1.size() > capacity) {
                K ghost_key = b1.back();
                b1.pop_back();
                b1_map.erase(ghost_key);
            }
        } else {
            // 从T2移除
            K lru_key = t2.back();
            t2.pop_back();
            V val = t2_map[lru_key].first;
            t2_map.erase(lru_key);
            
            // 添加到B2
            b2.push_front(lru_key);
            b2_map[lru_key] = b2.begin();
            
            // 如果B2太大，移除最旧的条目
            if (b2.size() > capacity) {
                K ghost_key = b2.back();
                b2.pop_back();
                b2_map.erase(ghost_key);
            }
        }
    }

public:
    /**
     * @brief 构造函数
     * @param size 缓存的最大容量
     */
    explicit ARCache(size_t size) : capacity(size), p(0) {}

    /**
     * @brief 插入或更新一个键值对
     * @param key 键
     * @param value 值
     * 
     * 根据页面的访问历史动态调整缓存策略
     */
    void put(const K& key, const V& value) override {
        // Case 1: 键在T1中
        if (t1_map.count(key)) {
            t1.erase(t1_map[key].second);
            t1_map.erase(key);
            t2.push_front(key);
            t2_map[key] = {value, t2.begin()};
            return;
        }
        
        // Case 2: 键在T2中
        if (t2_map.count(key)) {
            t2.erase(t2_map[key].second);
            t2.push_front(key);
            t2_map[key] = {value, t2.begin()};
            return;
        }
        
        // Case 3: 键在B1中（页面曾经在T1中）
        if (b1_map.count(key)) {
            // 增加p，因为在B1中命中说明最近使用的页面更重要
            size_t delta = 1;
            if (b1.size() >= b2.size()) {
                delta = b1.size() / b2.size();
            }
            p = std::min(capacity, p + delta);
            
            replace(false);
            b1.erase(b1_map[key]);
            b1_map.erase(key);
            t2.push_front(key);
            t2_map[key] = {value, t2.begin()};
            return;
        }
        
        // Case 4: 键在B2中（页面曾经在T2中）
        if (b2_map.count(key)) {
            // 减少p，因为在B2中命中说明频繁使用的页面更重要
            size_t delta = 1;
            if (b2.size() >= b1.size()) {
                delta = b2.size() / b1.size();
            }
            p = std::max(size_t(0), p - delta);
            
            replace(true);
            b2.erase(b2_map[key]);
            b2_map.erase(key);
            t2.push_front(key);
            t2_map[key] = {value, t2.begin()};
            return;
        }
        
        // Case 5: 键是新的
        size_t total = t1.size() + t2.size();
        if (total >= capacity) {
            if (total == capacity) {
                replace(false);
            }
            
            if (t1.size() < capacity) {
                t1.push_front(key);
                t1_map[key] = {value, t1.begin()};
            } else {
                t2.push_front(key);
                t2_map[key] = {value, t2.begin()};
            }
        } else {
            t1.push_front(key);
            t1_map[key] = {value, t1.begin()};
        }
    }

    /**
     * @brief 获取键对应的值
     * @param key 要查找的键
     * @param value 如果找到，值将被存储在这里
     * @return 如果找到键返回true，否则返回false
     */
    bool get(const K& key, V& value) override {
        // 检查T1
        if (t1_map.count(key)) {
            value = t1_map[key].first;
            t1.erase(t1_map[key].second);
            t1_map.erase(key);
            t2.push_front(key);
            t2_map[key] = {value, t2.begin()};
            return true;
        }
        
        // 检查T2
        if (t2_map.count(key)) {
            value = t2_map[key].first;
            t2.erase(t2_map[key].second);
            t2.push_front(key);
            t2_map[key] = {value, t2.begin()};
            return true;
        }
        
        return false;
    }

    /**
     * @brief 获取当前缓存中的项目数量
     */
    size_t size() const override {
        return t1.size() + t2.size();
    }

    /**
     * @brief 清空缓存
     */
    void clear() override {
        t1.clear();
        t2.clear();
        b1.clear();
        b2.clear();
        t1_map.clear();
        t2_map.clear();
        b1_map.clear();
        b2_map.clear();
        p = 0;
    }
};

#endif // ARC_CACHE_HPP
