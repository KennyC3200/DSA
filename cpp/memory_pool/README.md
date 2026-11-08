# Memory Pool
Each memory pool has blocks/chunks, and each block/chunk has slots of
fixed size. When requested, the pool hands out already allocated slots
to the user, contrasting `new` and `malloc` which both require a
kernel call which is very CPU intensive.

The memory pool keeps track of the "free" slots in a "free-list". The
free-list is an intrusive linked list, where the pointer to the next
node is stored in the Slot itself:

```cpp
template <typename T>
struct Slot {
    Slot* m_next;
    alignas(T) std::byte m_storage[slot_size];
};
```

If the slot is unused, it is treated as a `Slot*`, but if the slot is
used, it is treated as the address to storage.

The memory pool keeps track of the free list with `m_free_head`, which
as the name suggests is the head of the linked list. When the user
requests a free slot, if `m_free_head` is not `nullptr`, it will
return `m_free_head` as a `std::byte*` to the `m_storage`. If
`m_free_head` is `nullptr`, it will `Grow()` the memory pool,
allocating an entirely new block/chunk to use.

## Implementations
The [first implementation](memory_pool.hpp) is a very basic memory
pool implementation, while the 
[second implementation](memory_pool_STL.hpp) works with the standard
template library for containers that are node-based. So containers
like `std::list` and `std::set` would work, while contiguous
containers like `std::vector` would not work. See 
[the first example](example1.cpp) and 
[the second example](example2.cpp).

Also, when compiling `example2.cpp`, use the `-O2` flag. This will
demonstrate a much more measurable performance difference.

## Benchmark Results
Both examples tested 20,000,000 allocation operations across 100,000 
live targets, performed on an Intel Core i7 Ultra 165H.

`example1.cpp`: resulted in a ~3.8 times speedup.
| Allocator | Time (ms) |
| -------- | -------- |
| new/delete  | ~3800  |
| MemoryPool  | ~1000  |

`example2.cpp`: resulted in a ~7 times speedup.
| Allocator | Time (ms) |
| -------- | -------- |
| new/delete  | ~3500  |
| MemoryPool  | ~500  |