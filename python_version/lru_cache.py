from collections import OrderedDict
from typing import TypeVar, Generic
from cache import Cache

K = TypeVar('K')  # Key type
V = TypeVar('V')  # Value type

class LRUCache(Cache[K, V]):
    """Least Recently Used (LRU) cache implementation."""
    
    def __init__(self, capacity: int):
        """Initialize LRU cache with given capacity."""
        self.capacity = capacity
        self.cache: OrderedDict[K, V] = OrderedDict()
    
    def get(self, key: K) -> V | None:
        """Get a value by key from the cache."""
        if key not in self.cache:
            return None
        
        # Move to end (most recently used)
        value = self.cache.pop(key)
        self.cache[key] = value
        return value
    
    def put(self, key: K, value: V) -> None:
        """Put a key-value pair into the cache."""
        if key in self.cache:
            self.cache.pop(key)
        elif len(self.cache) >= self.capacity:
            # Remove least recently used item (first item)
            self.cache.popitem(last=False)
        
        self.cache[key] = value
    
    def size(self) -> int:
        """Return the current size of the cache."""
        return len(self.cache)
    
    def clear(self) -> None:
        """Clear all items from the cache."""
        self.cache.clear()
