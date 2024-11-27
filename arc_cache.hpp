#ifndef ARC_CACHE_HPP
#define ARC_CACHE_HPP

#include "cache.hpp"
#include <unordered_map>
#include <list>

template <typename K, typename V>
class ARCache : public Cache<K, V>
{
private:
    size_t capacity; // Maximum number of items in cache
    size_t p;        // Target size for T1

    // T1: Recent items
    std::list<K> t1;                                                             // Most recently used items
    std::unordered_map<K, std::pair<V, typename std::list<K>::iterator>> t1_map; // t1_map is a Hashing table. Key is K type, value is a std::pair type including a value V and a container pointing to the position in std::list<K>. Key to value and postion in t1

    // T2: Frequent items
    std::list<K> t2; // Most frequently used items
    std::unordered_map<K, std::pair<V, typename std::list<K>::iterator>> t2_map;

    // B1: Ghost entries for recently evicted from T1
    std::list<K> b1;
    std::unordered_map<K, typename std::list<K>::iterator> b1_map;

    // B2: Ghost entries for recently evicted from T2
    std::list<K> b2;
    std::unordered_map<K, typename std::list<K>::iterator> b2_map;

    void replace(bool in_b2)
    { // whether replacement is in b2 ；in_b2 表示导致缓存未命中的页面是否存在于 B2 中
        if (!t1.empty() && ((t1.size() > p) || (in_b2 && t1.size() == p)))
        {                          // Delete the LRU in T1 and move it to MRU in B1
            K lru_key = t1.back(); // delete the last element in t1(LRU)
            t1.pop_back();         // O(1),delete the element at the end;  erase is O(n)
            V val = t1_map[lru_key].first;
            t1_map.erase(lru_key);

            b1.push_front(lru_key); // insert in the beginning of b1
            b1_map[lru_key] = b1.begin();

            /*   if (b1.size() > capacity) { //if the size of b1 is larger than the size of cache
                   K ghost_key = b1.back();
                   b1.pop_back();
                   b1_map.erase(ghost_key);
               }*/
        }
        else
        { // Delete the last element in t2(LRU in t2),and move it to MRU in B2
            K lru_key = t2.back();
            // LRU key in t2
            t2.pop_back();
            V val = t2_map[lru_key].first;
            t2_map.erase(lru_key);

            b2.push_front(lru_key);
            b2_map[lru_key] = b2.begin();

            if (b2.size() > capacity)
            {
                K ghost_key = b2.back();
                b2.pop_back();
                b2_map.erase(ghost_key);
            }
        }
    }

public:
    explicit ARCache(size_t size) : capacity(size), p(0) {} // Constructor

    void put(const K &key, const V &value) override
    { // Put key-value pair in cache
        // Case 1: Key exists in T1, Recent used items to be moved to front of T2
        if (t1_map.count(key))
        {                                      // check whether key is in t1_map
            t1.erase(t1_map[key].second);      // delete element
            t1_map.erase(key);                 // delete it in t1_map
            t2.push_front(key);                // insert an item in the list t2
            t2_map[key] = {value, t2.begin()}; // set the value of key and beginning postion in t2
            return;
        }

        // Case 2: Key exists in T2
        if (t2_map.count(key))
        {
            t2.erase(t2_map[key].second);
            t2.push_front(key);
            t2_map[key] = {value, t2.begin()}; // put them to the beginning of the T2
            return;
        }

        // Case 3: Key in B1(cache miss)
        if (b1_map.count(key))
        {
            double delta = std::max(1.0, static_cast<double>(b2.size()) / static_cast<double>(std::max(size_t(1), b1.size()))); // the ratio of b2 and b1, make sure that it is larger than 1
            p = std::min(capacity, static_cast<size_t>(p + delta));                                                             // make sure p is smaller than capacity
            replace(false);                                                                                                     //?
            b1.erase(b1_map[key]);
            b1_map.erase(key);
            t2.push_front(key);
            t2_map[key] = {value, t2.begin()};
            return;
        }

        // Case 4: Key in B2 (cache miss)
        if (b2_map.count(key))
        {
            double delta = std::max(1.0, static_cast<double>(b1.size()) / static_cast<double>(std::max(size_t(1), b2.size()))); // the ratio of b1 and b2
            p = std::max(size_t(0), static_cast<size_t>(p >= delta ? p - delta : 0));                                           // if p>=delta, p=p-delta,otherwise p=0 ; change the size of p according to the b1 and b2
            replace(true);
            b2.erase(b2_map[key]);
            b2_map.erase(key);
            t2.push_front(key);
            t2_map[key] = {value, t2.begin()};
            return;
        }

        // Case 5: Cache miss 调整！！
        /*size_t total = t1.size() + t2.size();
        if (total >= capacity) {
            if (total == capacity) {
                replace(false);
            }
            else {
                if (t1.size() < capacity) {
                    K lru_key = t2.back();
                    t2.pop_back();
                    t2_map.erase(lru_key);
                }
            }
        }

        t1.push_front(key);
        t1_map[key] = {value, t1.begin()};*/

        // Case 5: Super Cache miss
        if (t1.size() + b1.size() >= capacity)
        {
            if (t1.size() < capacity)
            {
                if (!b1.empty())
                {
                    K lru_key = b1.back();
                    b1.pop_back();
                    b1_map.erase(lru_key);
                }
                replace(false);
            }
            else
            {
                K lru_key = t1.back();
                t1.pop_back();
                t1_map.erase(lru_key);
            }
        }
        else if (t1.size() + t2.size() + b1.size() + b2.size() >= capacity)
        {
            if (t1.size() + t2.size() + b1.size() + b2.size() == 2 * capacity)
            {
                if (!b2.empty())
                {
                    K lru_key = b2.back();
                    b2.pop_back();
                    b2_map.erase(lru_key);
                }
            }
            replace(false);
        }
        // 默认情况下，将新键添加到 T1
        t1.push_front(key);
        t1_map[key] = {value, t1.begin()};
    }

    bool get(const K &key, V &value) override
    {
        // Case 1: Key in T1
        if (t1_map.count(key))
        {
            value = t1_map[key].first;
            K k = key;
            V v = value;
            put(k, v); // This will move it to T2
            return true;
        }

        // Case 2: Key in T2
        if (t2_map.count(key))
        {
            value = t2_map[key].first;
            K k = key;
            V v = value;
            put(k, v); // This will move it to front of T2
            return true;
        } // if we find key, return false

        return false; // if we don't find key, return false
    }

    size_t size() const override
    {
        return t1.size() + t2.size();
    }

    void clear() override
    {
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
