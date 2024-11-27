from typing import TypeVar, Generic, Dict, List
from collections import OrderedDict
from cache import Cache

K = TypeVar('K')  # Key type
V = TypeVar('V')  # Value type

class ARCache(Cache[K, V]):
    """Adaptive Replacement Cache (ARC) implementation."""
    
    def __init__(self, capacity: int):
        """Initialize ARC with given capacity."""
        self.capacity = capacity
        self.p = 0  # Target size for T1
        
        # T1: Recent items
        self.t1: OrderedDict[K, V] = OrderedDict()
        
        # T2: Frequent items
        self.t2: OrderedDict[K, V] = OrderedDict()
        
        # B1: Ghost entries for recently evicted from T1
        self.b1: OrderedDict[K, None] = OrderedDict()
        
        # B2: Ghost entries for recently evicted from T2
        self.b2: OrderedDict[K, None] = OrderedDict()

    def _replace(self, in_b2: bool) -> None:
        """Helper method for replacement policy."""
        if (len(self.t1) >= max(1, self.p) or (in_b2 and len(self.t1) == self.p)) and len(self.t1) > 0:
            # Delete LRU from T1 and move it to MRU of B1
            key, value = self.t1.popitem(last=True)
            self.b1[key] = None
            if len(self.b1) > self.capacity:
                self.b1.popitem(last=True)
        elif len(self.t2) > 0:
            # Delete LRU from T2 and move it to MRU of B2
            key, value = self.t2.popitem(last=True)
            self.b2[key] = None
            if len(self.b2) > self.capacity:
                self.b2.popitem(last=True)

    def put(self, key: K, value: V) -> None:
        """Put a key-value pair into the cache."""
        # Case 1: Key exists in T1
        if key in self.t1:
            del self.t1[key]
            self.t2[key] = value
            return
            
        # Case 2: Key exists in T2
        if key in self.t2:
            del self.t2[key]
            self.t2[key] = value
            return

        # Case 3: Key exists in B1
        if key in self.b1:
            # Increase the target size for the T1
            delta = 1
            if len(self.b1) > 0:
                delta = max(1, len(self.b2) // len(self.b1))
            self.p = min(self.capacity, self.p + delta)
            self._replace(False)
            # Remove from B1 and add to T2
            del self.b1[key]
            self.t2[key] = value
            return

        # Case 4: Key exists in B2
        if key in self.b2:
            # Decrease the target size for the T1
            delta = 1
            if len(self.b2) > 0:
                delta = max(1, len(self.b1) // len(self.b2))
            self.p = max(0, self.p - delta)
            self._replace(True)
            # Remove from B2 and add to T2
            del self.b2[key]
            self.t2[key] = value
            return

        # Case 5: Key doesn't exist in cache
        total = len(self.t1) + len(self.t2)
        if total >= self.capacity:
            if total == self.capacity:
                if len(self.t1) < self.capacity:
                    if len(self.b2) > 0:
                        self.b2.popitem(last=True)
                    self._replace(False)
                else:
                    key_to_remove, _ = self.t1.popitem(last=True)
                    self.b1[key_to_remove] = None
            else:
                self._replace(False)
        
        # Add to T1
        self.t1[key] = value

    def get(self, key: K) -> V | None:
        """Get a value by key from the cache."""
        if key in self.t1:
            value = self.t1[key]
            del self.t1[key]
            self.t2[key] = value
            return value
        if key in self.t2:
            value = self.t2[key]
            del self.t2[key]
            self.t2[key] = value
            return value
        return None

    def size(self) -> int:
        """Return the current size of the cache."""
        return len(self.t1) + len(self.t2)

    def clear(self) -> None:
        """Clear all items from the cache."""
        self.t1.clear()
        self.t2.clear()
        self.b1.clear()
        self.b2.clear()
        self.p = 0
