#ifndef CACHE_HPP
#define CACHE_HPP

#include <unordered_map>
#include <list>
#include <cstddef>

template<typename K, typename V>
class Cache { //abstract class, it cannot be instantiated
public:
    virtual ~Cache() = default;
    virtual void put(const K& key, const V& value) = 0;
    virtual bool get(const K& key, V& value) = 0;
    virtual size_t size() const = 0;
    virtual void clear() = 0;
};

#endif // CACHE_HPP
