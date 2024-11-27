from abc import ABC, abstractmethod
from typing import TypeVar, Generic

K = TypeVar('K')  # Key type
V = TypeVar('V')  # Value type

class Cache(ABC, Generic[K, V]):
    """Abstract base class for cache implementations."""
    
    @abstractmethod
    def put(self, key: K, value: V) -> None:
        """Put a key-value pair into the cache."""
        pass
    
    @abstractmethod
    def get(self, key: K) -> V | None:
        """Get a value by key from the cache. Returns None if key not found."""
        pass
    
    @abstractmethod
    def size(self) -> int:
        """Return the current size of the cache."""
        pass
    
    @abstractmethod
    def clear(self) -> None:
        """Clear all items from the cache."""
        pass
