#ifndef LFU_CACHE_HPP
#define LFU_CACHE_HPP

#include "cache.hpp"
#include <unordered_map>
#include <map>

template<typename K, typename V>
class LFUCache : public Cache<K, V> {
private:
    size_t capacity; //the maximum elements in cache
    size_t minFreq; //element with the minimum frequency
    std::unordered_map<K, std::pair<V, size_t>> keyToVal;  // key -> {value, freq}
    std::unordered_map<K, typename std::list<K>::iterator> keyToIter;  // key -> iterator in freqToKeys
    std::map<size_t, std::list<K>> freqToKeys;  // freq -> list of keys with the same frequency

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
    explicit LFUCache(size_t size) : capacity(size), minFreq(0) {}

    void put(const K& key, const V& value) override {
        if (capacity == 0) return;

        if (keyToVal.count(key)) {
            keyToVal[key].first = value;
            increment(key);
            return;
        }

        if (keyToVal.size() >= capacity) {
            K evictKey = freqToKeys[minFreq].back();
            freqToKeys[minFreq].pop_back(); //evict the minimum frequency element
            if (freqToKeys[minFreq].empty()) {
                freqToKeys.erase(minFreq);
            } //if the list of minFreq is empty
            keyToVal.erase(evictKey);
            keyToIter.erase(evictKey);
        }

        keyToVal[key] = {value, 1};
        freqToKeys[1].push_front(key);
        keyToIter[key] = freqToKeys[1].begin();
        minFreq = 1;
    }

    bool get(const K& key, V& value) override {
        if (keyToVal.count(key) == 0) {
            return false;
        }
        value = keyToVal[key].first;
        increment(key);
        return true;
    }

    size_t size() const override {
        return keyToVal.size();
    }

    void clear() override {
        keyToVal.clear();
        keyToIter.clear();
        freqToKeys.clear();
        minFreq = 0;
    }
};

#endif // LFU_CACHE_HPP
