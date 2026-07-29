# Arena Allocator
This version of arena allocator is based off of Howard Hinnant's [stack_alloc](https://howardhinnant.github.io/stack_alloc.html), which is an implementation of an arena allocator that utilizes the stack and fallback heap.

This implementation, contrary to most implementations, is a custom allocator that can be used with the standard library. It involves two classes: `arena` and `short_alloc`. The `arena` class is the main object keeping track of the data, while `short_alloc` is a container for `arena` which provides countless conveniences.

## Usage
You have to first instantiate the underlying `arena` to initialize the buffer before implicitly casting it to be of type `short_alloc` for use. The templated type of the buffer of `arena` does not matter since it can be rebinded to a different templated type which depends on the templated type of the `short_alloc`:
```cpp
template <typename T, std::size_t BufferSize = 200>
using SmallVector = std::vector<T, short_alloc<T, Buffersize, alignof(T)>>;

int main(void) {
    // A std::vector with type int and 200 bytes allocated for the arena
    SmallVector<int>::allocator_type::arena_type arena;

    // NOTE: type arena_type is implicitly casted to be type short_alloc from constructor for
    //       short_alloc. This design choice is intentional
    SmallVector<int> vec(arena);

    return 0;
}
```
You also have to ensure that the `buffer` size is a multiple of the `Alignment`, or else the program will not compile. In this case, `alignof(int)` is 4 bytes, and `200 % 4 == 0` so we're good.

## Implementation of `arena`
The `arena` class uses a finite stack with max alignment of `alignment` for initial allocations and uses a heap for fallback allocations. 

The first element in `m_buffer` in `arena`, `&m_buffer[0]` is aligned to `alignment` which means all subsequent elements will also be aligned to `alignment`. Furthermore, if `alignment` is a power of 2, any future allocations that require alignment less than `alignment` and is a power of 2 will automatically be aligned.

**Note:** For the fallback heap, if it is compiled C++17 onwards, the allocations on the heap will be aligned properly. If compiled with say C++11, the allocations made on the heap will not be aligned due to the limitations of `::operator new` in C++11.

## Benefits of `short_alloc`
The first problem it solves is the most obvious: because there is only **one** `arena` object which is 99% of the time instantiated on the stack, imagine an `arena` object having to be an `int`, `float`, `double`, `node`, etc. all at the same time! Therefore, we use `short_alloc<_Ty>` to deal with allocating specific types `_Ty`. 

Containers frequently [rebind](#rebinding-allocators) to different types, which would be extremely inconvenient if it were a single `arena` object being passed around. 

Custom allocators must also satisfy the [Allocator concept](#allocator-concept) for a specific type `_Ty`.

### Rebinding Allocators
E.g. if you pass `short_alloc<int>` to `std::list<int>`, the list (which is a doubly linked list under the hood)  first needs to allocate internal nodes `std::__list_node<int>`, not raw `int`'s. Hence, the container uses `allocator::rebind` to create a `short_alloc<Node>` to make the necessary memory allocations for the `Node` objects.

For more details, in the `vector` standard header file, you will come across these two lines in the constructor of `vector`:
```cpp
using _Alty        = _Rebind_alloc_t<_Alloc, _Ty>;
using _Alty_traits = allocator_traits<_Alty>;
```
- `_Alty` is short for "Allocator type", which equals the allocator `_Alloc` being rebinded to type `_Ty`. `_Rebind_alloc_t` is shorthand for `typename std::allocator_traits<Alloc>::template rebind_alloc<_Ty>`. E.g. it takes an existing allocator type `Alloc<U>` and transforms it into `Alloc<_Ty>`
    ```
    Alloc<int> -> _Rebind_alloc_t<Alloc<int>, double> -> Alloc<double>
     ```
     Under the hood, `allocator_traits` does this by using two fallback rules:
     1. If the allocator explicity provides a nested `rebind` template (e.g. `Alloc::rebind<U>::other`, it uses that)
     2. If the allocator is a class template, like `Alloc<T, Args ...>`, `allocator_traits` automatically replaces `T` with `_Ty` to yield `Alloc<_Ty, Args ...>`. **Important:** this fallback only works if **ALL** template parameters are types. So in the `short_stack<class T, std::size_t N, std::size_t Align>` case, this fallback would not work, since `N` and `Align` are values and not types.
- `_Alty_traits` is short for "Allocator traits", which wraps `_Alty` in `std::allocator_traits` to access memory operations (e.g. `allocate`, `deallocate`, `construct`, and `destroy`). Prior to C++11, every custom allocator required a dozen mandatory typedefs (`pointer`, `const_pointer`, `reference`, etc.) and member functions (`construct`, `destroy`, etc.) which made custom allocators verbose and tedious to write. `allocator_traits` provides **defualts** for almost everything. Rather than call allocator methods directly:
    ```cpp
    alloc.construct(ptr, val);
    ```
    The Standard Library calls all allocator functions via `allocator_traits` static methods:
    ```cpp
    std::allocator_traits<Alloc>::construct(alloc, ptr, val);
    ```

### Allocator Concept
An allocator is simply a tool that hands out pieces of computer memory when a program requests it and takes it back when they are done. Standard library containers, like `std::vector`, don't call `malloc` or `::operator new` directly--instead, they delegate memory requests to an Allocator object, like `std::allocator<T>`. This design gives containers flexibility:
- You can allocate memory from a fixed arena, shared memory, a memory-mapped file, or a custom pool
- High-frequency allocations (like in linked lists or trees) are very slow with default heap allocators. A custom allocator will use stack buffers or region-based pools to achieve near-zero overhead
- Separation of concern: `std::vector` manages the dynamic array logic (resizing, indexing, etc.) while the allocator manages the raw-bytes

For a class to satisfy the Allocator concept, it must provide a specific set of types and methods so that container templates can interact with it. These are some basic requirements (C++11 onwards):

```cpp
template <typename T>
struct CustomAllocator {
    using value_type = T;

    CustomAllocator() noexcept;

    template <typename U>
    CustomAllocator(const CustomAllocator<U>&) noexcept;

    // Allocate memory for n objects of type T without constructing them
    [[nodiscard]] T* allocate(std::size_t n);

    // Deallocate memory previously allocated with allocate
    void deallocate(T* p, std::size_t n) noexcept;
};

// Two instances must be comparable for equality
template <typename T, typename U>
bool operator==(const CustomAllocator<T>& a, const CustomAllocator<U>& b) noexcept;

template <typename T, typename U>
bool operator!=(const CustomAllocator<T>& a, const CustomAllocator<U>& b) noexcept;
```

### Things to Note
In the comparison operator for `short_alloc`:
```cpp
template <class T1, std::size_t N1, std::size_t A1, class U, std::size_t M, std::size_t A2>
friend bool operator==(
    const short_alloc<T1, N1, A1>& x, 
    const short_alloc<U, M, A2>& y) noexcept
{
    return N1 == M && A1 == A2 && &x.m_arena == &y.m_arena;
}
```
If you ever try to compare two `short_alloc`s with different `N/Align`, that specific call won't compile and the compiler will give you a type error instead of `false`. This is because when the compiler builds this function for a specific pair of types, it has to generate machine code for every part of the expression, including the part after `&&`, even though that part might never execute. So it might be comparing say `short_alloc<int, 1024, 8>` to `short_alloc<int, 512, 4>`, which are two completely different types, causing it to generate a type error! Hence, as long as the `operator==` is not called with two `short_alloc`s of mismatched `N/Align`, there will be no compile error.