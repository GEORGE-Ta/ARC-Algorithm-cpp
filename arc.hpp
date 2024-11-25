#ifndef ARC_HPP
#define ARC_HPP

#include <unordered_map>
#include <list>
#include <cassert>
#include <algorithm>

template<typename K, typename V>
class ARCache {
private:
    size_t capacity;  // 缓存总容量
    double p;         // 自适应参数

    // T1: 最近使用过一次的页面
    std::list<K> t1;
    // T2: 最近使用过至少两次的页面
    std::list<K> t2;
    // B1: 最近从T1驱逐出的页面的ghost list
    std::list<K> b1;
    // B2: 最近从T2驱逐出的页面的ghost list
    std::list<K> b2;

    // 存储实际的键值对
    std::unordered_map<K, std::pair<V, typename std::list<K>::iterator>> cache;
    // ghost cache只存储key
    std::unordered_map<K, typename std::list<K>::iterator> ghost;

    void replace(const K& key) {
        bool in_b1 = (ghost.find(key) != ghost.end() && 
                     std::find(b1.begin(), b1.end(), key) != b1.end());
        bool in_b2 = (ghost.find(key) != ghost.end() && 
                     std::find(b2.begin(), b2.end(), key) != b2.end());

        if (in_b1) {
            p = std::min(p + 1.0, static_cast<double>(capacity));
        } else if (in_b2) {
            p = std::max(p - 1.0, 0.0);
        }

        if (t1.size() + b1.size() >= capacity) {
            if (t1.size() < capacity) {
                b1.pop_back();
                ghost.erase(b1.back());
                replace_lru();
            } else {
                auto lru = t1.back();
                t1.pop_back();
                cache.erase(lru);
            }
        } else if (t1.size() + t2.size() + b1.size() + b2.size() >= 2 * capacity) {
            if (t1.size() + t2.size() + b1.size() + b2.size() >= 2 * capacity) {
                b2.pop_back();
                ghost.erase(b2.back());
            }
        }
    }

    void replace_lru() {
        if (!t1.empty() && ((ghost.find(t1.back()) != ghost.end() && 
            std::find(b2.begin(), b2.end(), t1.back()) != b2.end()) || 
            t1.size() > p)) {
            // 从T1移动到B1
            K lru = t1.back();
            t1.pop_back();
            b1.push_front(lru);
            ghost[lru] = b1.begin();
            cache.erase(lru);
        } else {
            // 从T2移动到B2
            K lru = t2.back();
            t2.pop_back();
            b2.push_front(lru);
            ghost[lru] = b2.begin();
            cache.erase(lru);
        }
    }

public:
    explicit ARCache(size_t size) : capacity(size), p(0.0) {}

    bool get(const K& key, V& value) {
        auto it = cache.find(key);
        if (it == cache.end()) {
            return false;
        }

        // 如果在T1中找到
        if (std::find(t1.begin(), t1.end(), key) != t1.end()) {
            t1.erase(it->second.second);
            t2.push_front(key);
            it->second.second = t2.begin();
        }
        // 如果在T2中找到，移到T2的前端
        else {
            t2.erase(it->second.second);
            t2.push_front(key);
            it->second.second = t2.begin();
        }

        value = it->second.first;
        return true;
    }

    void put(const K& key, const V& value) {
        auto cache_it = cache.find(key);
        if (cache_it != cache.end()) {
            // 更新现有条目
            cache_it->second.first = value;
            if (std::find(t1.begin(), t1.end(), key) != t1.end()) {
                t1.erase(cache_it->second.second);
                t2.push_front(key);
                cache_it->second.second = t2.begin();
            } else {
                t2.erase(cache_it->second.second);
                t2.push_front(key);
                cache_it->second.second = t2.begin();
            }
            return;
        }

        // 处理缓存未命中的情况
        if (cache.size() >= capacity) {
            replace(key);
        }

        // 检查是否在ghost cache中
        auto ghost_it = ghost.find(key);
        if (ghost_it != ghost.end()) {
            if (std::find(b1.begin(), b1.end(), key) != b1.end()) {
                // 从B1移动到T2
                b1.erase(ghost_it->second);
                ghost.erase(ghost_it);
                t2.push_front(key);
                cache[key] = {value, t2.begin()};
            } else {
                // 从B2移动到T2
                b2.erase(ghost_it->second);
                ghost.erase(ghost_it);
                t2.push_front(key);
                cache[key] = {value, t2.begin()};
            }
        } else {
            // 新项添加到T1
            t1.push_front(key);
            cache[key] = {value, t1.begin()};
        }
    }

    size_t size() const {
        return cache.size();
    }

    bool empty() const {
        return cache.empty();
    }

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
