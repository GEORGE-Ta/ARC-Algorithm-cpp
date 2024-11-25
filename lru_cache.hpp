#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include "cache.hpp"
#include <unordered_map>
#include <list>

template<typename K, typename V>
class LRUCache : public Cache<K, V> {
private:
    size_t capacity;
    std::list<std::pair<K, V>> cache_list;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> cache_map;

public:
    explicit LRUCache(size_t size) : capacity(size) {}

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

    bool get(const K& key, V& value) override {
        auto it = cache_map.find(key);
        if (it == cache_map.end()) {
            return false;
        }
        value = it->second->second;
        cache_list.splice(cache_list.begin(), cache_list, it->second);
        return true;
    }

    size_t size() const override {
        return cache_list.size();
    }

    void clear() override {
        cache_list.clear();
        cache_map.clear();
    }
};

#endif // LRU_CACHE_HPP
