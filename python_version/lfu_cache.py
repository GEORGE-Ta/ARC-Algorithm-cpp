from collections import OrderedDict, defaultdict
from typing import TypeVar, Generic, Dict
from cache import Cache

K = TypeVar('K')  # Key type
V = TypeVar('V')  # Value type

class LFUCache(Cache[K, V]):
    """Least Frequently Used (LFU) cache implementation."""
    
    def __init__(self, capacity: int):
        """Initialize LFU cache with given capacity."""
        self.capacity = capacity
        self.min_freq = 0
        self.key_freq: Dict[K, int] = {}  # key -> frequency
        self.freq_keys: Dict[int, OrderedDict[K, V]] = defaultdict(OrderedDict)  # frequency -> ordered dict of keys
    
    def get(self, key: K) -> V | None:
        """Get a value by key from the cache."""
        if key not in self.key_freq:
            return None
        
        # Get the value and update frequency
        freq = self.key_freq[key]
        value = self.freq_keys[freq][key]
        
        # Remove from current frequency bucket
        del self.freq_keys[freq][key]
        if not self.freq_keys[freq] and freq == self.min_freq:
            self.min_freq += 1
            
        # Add to next frequency bucket
        freq += 1
        self.key_freq[key] = freq
        self.freq_keys[freq][key] = value
        
        return value
    
    def put(self, key: K, value: V) -> None:
        """Put a key-value pair into the cache."""
        if self.capacity <= 0:
            return
            
        if key in self.key_freq:
            # Update existing key
            freq = self.key_freq[key]
            del self.freq_keys[freq][key]
            if not self.freq_keys[freq] and freq == self.min_freq:
                self.min_freq += 1
                
            freq += 1
            self.key_freq[key] = freq
            self.freq_keys[freq][key] = value
        else:
            # Add new key
            if len(self.key_freq) >= self.capacity:
                # Remove least frequent used item
                lfu_dict = self.freq_keys[self.min_freq]
                lfu_key, _ = lfu_dict.popitem(last=False)  # Remove the first item (least recently used among least frequent)
                del self.key_freq[lfu_key]
                if not self.freq_keys[self.min_freq]:
                    del self.freq_keys[self.min_freq]
            
            self.key_freq[key] = 1
            self.freq_keys[1][key] = value
            self.min_freq = 1
    
    def size(self) -> int:
        """Return the current size of the cache."""
        return len(self.key_freq)
    
    def clear(self) -> None:
        """Clear all items from the cache."""
        self.key_freq.clear()
        self.freq_keys.clear()
        self.min_freq = 0
