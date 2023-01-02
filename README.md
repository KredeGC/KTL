<div align="center">
<h1>Krede Template Library</h1>

![Windows supported](https://img.shields.io/badge/Windows-win--10-green?style=flat-square)
![Linux supported](https://img.shields.io/badge/Linux-Ubuntu-green?style=flat-square)
![MacOS untested](https://img.shields.io/badge/MacOS-Untested-red?style=flat-square)

A C++ library containing various composite memory allocators and containers.<br/>
Allocators are mostly based on a [Talk by Andrei Alexandrescu](https://www.youtube.com/watch?v=LIb3L4vKZ7U).

[![Release](https://img.shields.io/github/v/release/KredeGC/KTL?display_name=tag&style=flat-square)](https://github.com/KredeGC/KTL/releases)
[![License](https://img.shields.io/github/license/KredeGC/KTL?style=flat-square)](https://github.com/KredeGC/KTL/blob/master/LICENSE)
[![Issues](https://img.shields.io/github/issues/KredeGC/KTL?style=flat-square)](https://github.com/KredeGC/KTL/issues)
[![Tests](https://img.shields.io/github/actions/workflow/status/KredeGC/KTL/main.yml?branch=master&style=flat-square)](https://github.com/KredeGC/KTL/actions/workflows/main.yml)

</div>

# Status
The interface is currently not in a stable state, so expect changes between updates to cause some disruption, as functions or types may not exist between versions.
In addition, most allocators presented are not thread-safe.
For now thread safety is not on the roadmap as it would degrade performance on single-threaded applications.
The functionality itself is fairly stable but should not be expected to be used in a production environment for the time being.

# Table of Content
* [Installation](#installation)
* [Usage](#usage)
* [Allocators](#allocators)
* [Allocator Interface](#allocator-interface)
* [Containers](#containers)
  * [binary_heap interface](#binary_heap-interface)
  * [trivial_array interface](#trivial_array-interface)
  * [trivial_vector interface](#trivial_vector-interface)
  * [unordered_map interface](#unordered_map-interface)
  * [unordered_multimap interface](#unordered_multimap-interface)
* [Allocator examples](#allocator-examples)
* [Building and running tests](#building-and-running-tests)

# Installation
As this is a header-only library, you can simply copy the header files directly into your project.
The header files can either be downloaded from the [releases page](https://github.com/KredeGC/KTL/releases) or from the [`include/`](https://github.com/KredeGC/KTL/tree/master/include/ktl) directory on the master branch.
The source and header files inside the `src/` directory are only tests and should not be included into your project.

# Usage
This library was made with C++17 in mind and may or may not be compatible with earlier versions.
The library has 3 main files that you can include into your application.
* [`ktl/ktl.h`](https://github.com/KredeGC/KTL/tree/master/include/ktl/ktl.h) - Includes everything into this translation unit
* [`ktl/ktl_alloc_fwd.h`](https://github.com/KredeGC/KTL/tree/master/include/ktl/ktl_alloc_fwd.h) - Includes forward declarations for all the allocators
* [`ktl/ktl_container_fwd.h`](https://github.com/KredeGC/KTL/tree/master/include/ktl/ktl_container_fwd.h) - Includes forward declarations for all the containers

If you only need a specific allocator, container or forward declaration, you can simply include the ones you need. Forward declaration files are postfixed with `_fwd` in their name.
Allocators and containers are separated into the [`allocators/`](https://github.com/KredeGC/KTL/tree/master/include/ktl/allocators) and [`containers/`](https://github.com/KredeGC/KTL/tree/master/include/ktl/containers) directories.

For more information about a specific type, you can look in the interface descriptions below or look up the source code or tests. You can also look at the examples in [Allocator examples](#allocator-examples).

# Allocators
This library contains 2 different types of allocators:
* Raw void* allocators - Do the actual allocation/deallocation and construction/destruction
* Composite/synthetic allocators - Attach to other allocators to provide extra features on top

Both of these allocator types are not STL compliant, but can be made to be if used with the `type_allocator<T, Allocator>` type.<br/>
This is a composite allocator that you can use to make an allocator typed, like so: `type_allocator<int, linear_allocator>`.<br/>
All allocators also have a typedeffed version with a `type_` prefix as a shorthand, such as: `type_linear_allocator<int>`.

| Signature | Type | Description |
| --- | --- |--- |
| linear_allocator<br/>\<Size\> | Raw | Allocates a block of `Size` which it then hands out in chunks, similar to `stack_allocator`.<br/>Simply increments a counter during allocation, making it faster than linked_allcoator, but it also rarely deallocates.<br/>Has a max allocation size of the `Size` given, but unlike the `stack_allocator` constructs it's memory dynamically. |
| mallocator | Raw | An allocator which tries to align memory when allocating.<br/>Almost like std::allocator, except it has no type. |
| null_allocator | Raw | An allocator which allocates and owns nothing.<br/>Useful for ensuring that a composite allocator doesn't use a specific path when allocating. |
| stack_allocator<br/>\<Size\> | Raw | Uses a preallocated `stack<Size>`, which has to be passed in during construction.<br/>Simply increments a counter during allocation, making it faster than linked_allcoator, but it also rarely deallocates.<br/>Has a max allocation size of the `Size` given. |
| cascading<br/>\<Allocator\> | Composite | Attempts to allocate using the given allocator, but upon failure will create a new allocator and keep a reference to the old one.<br/>Deallocation can take O(n) time as it may have to traverse multiple allocator instances to find the right one. |
| fallback<br/>\<Primary, Fallback\> | Composite | Delegates allocation between 2 allocators.<br/>It first attempts to allocate with the `Primary` allocator, but upon failure will use the `Fallback` allocator. |
| freelist<br/>\<Min, Max, Alloc\> | Composite | Allocates using the given allocator, if the size specified is within the range of `Min` and `Max`, otherwise returns `nullptr`.<br/>When deallocating, it keeps the free memory in a linked list which can be reused on later allocations. |
| linked<br/>\<Size\> | Composite | Allocates one big chunk of memory when instantiated.<br/>Uses a linked list to determine whether a given sub-chunk of memory is free or allocated, which takes O(n) time.<br/>Has a max allocation size of the `Size` given.<br/>More versatile than `linear_allocator`, but also more slow on deallocation. |
| overflow<br/>\<Allocator, std::ostream\> | Composite | Checks for memory corruption/leak when allocating/constructing via it's specified allocator. It streams the results to the std::ostream specified. |
| segragator<br/>\<Threshold, Primary, Fallback\> | Composite | Delegates allocation between 2 allocators based on a size threshold. |
| type_allocator<br/>\<T, Allocator\> | Composite | Wraps around the specified allocator with a type. This can be used to make the other allocators STL compliant, so they can be used with STL containers. |

NOTES:
Exceptions are not used with any of the allocators above. This means that upon failure, they will simply return a null pointer to indicate that they were unable to allocate anything. Some synthethic allocators may rely upon this nullptr feature, like fallback_allocator, which upon failure will attempt to use the fallback allocator instead.

## Allocator interface
The allocators roughly follow the standard for STL allocators, except that they are not typed, so use void* instead.
`type_allocator` is the only allocator that uses T* for allocations.
Some additional methods have also been added.

| Method | Description |
| --- | --- |
| `void* allocate(size_type size)` | Attempts to allocate a chunk of memory defined by `size`. For non-typed allocators the size is in bytes, but for typed allocators it's the amount of objects of the given type. |
| `void deallocate(void* ptr, size_type size)` | Attempts to deallocate the memory at location `ptr` with the given size, `size`. |
| `void construct(T* ptr, Args&& args)` | Calls the constructor of a specific type at the location `ptr` with `args`.<br/>Most allocators do not define this method. |
| `void destroy(T* ptr)` | Calls the destructor of a specific type at the location `ptr`.<br/>Most allocators do not define this method. |
| `size_type max_size()` | Returns the maximum size this allocator could possibly allocate.<br/>Not all allocators define this method. |
| `bool owns(void* ptr) const` | Returns whether or not the given memory at location `ptr` is owned by this allocator.<br/>Not all allocators define this method, such as `mallocator`. |
| `Alloc& get_allocator() const` | Returns the allocator that this allocator wraps around.<br/>Only some composite allocators define this method. |

# Containers
This library also contains various containers that are STL compliant.

| Signature | Description | Notes |
| --- | --- | --- |
| [binary_heap<br/>\<T, Comp, Alloc\>](#binary_heap-interface) | A binary heap, sorted using the `Comp` and allocated using the given `Alloc` allocator. | `Comp` can be either `std::greater<T>` or `std::less<T>` or some other custom implementation.<br/>A shorthand version of both a min and a max heap can be used, via the `binary_min_heap<T, Alloc>` and `binary_max_heap<T, Alloc>` types. |
| [trivial_array<br/>\<T, Alloc\>](#trivial_array-interface) | An array wrapper class, similar to `std::array`, but uses dynamic allocation and is optimized for trivial types. Takes a type `T` and an allocator `Alloc`. | The container uses a straight `memcpy` for most of its operations.<br/>It's not recommended to use this with non-trivial types, eg. types that have custom default, copy or move constructors or custom destructors.<br/>Just like with the standard array, the container may invalidate iterators and references on insertion, but not on erasure. |
| [trivial_vector<br/>\<T, Alloc\>](#trivial_vector-interface) | A vector class, similar to `std::vector`, but optimized for trivial types. Takes a type `T` and an allocator `Alloc`. | The container uses a straight `memcpy` for most of its operations.<br/>It's not recommended to use this with non-trivial types, eg. types that have custom default, copy or move constructors or custom destructors.<br/>Just like with the standard vector, the container may invalidate iterators and references on insertion, but not on erasure. |
| [unordered_map<br/><K, V, Hash, EqualTo, Alloc>](#unordered_map-interface) | A hash map class similar to `std::unordered_map`, but optimized for cache locality using open addressing with linear probing. | The container uses a `Hash` struct, `EqualTo` struct and `Alloc` class passed in as template parameters, just like the standard unordered map.<br/>Unlike `std::unordered_map` iterator and reference invalidation may happen on insertion. However, only iterators and references to the erased object are invalidated on erasure. |
| [unordered_multi_map<br/><K, V, Hash, EqualTo, Alloc>](#unordered_multimap-interface) | A hash multimap class similar to `std::unordered_multimap`, but optimized for cache locality using open addressing with linear probing. | The container uses a `Hash` struct, `EqualTo` struct and `Alloc` class passed in as template parameters, just like the standard unordered map.<br/>Unlike `std::unordered_multimap` iterator and reference invalidation may happen on insertion. However, only iterators and references to the erased object are invalidated on erasure. |

## binary_heap interface
| Method | Description |
| --- | --- |
| `void clear()` | Clear all elements in the heap. |
| `T pop()` | Removes the root element (lowest or highest, depending on min or max heap) and returns it. |
| `void insert(const T& value)` | Pushes a new element into the heap by copying. |
| `void insert(T&& value)` | Pushes a new element into the heap by moving. |

## trivial_array interface
| Method | Description |
| --- | --- |
| `T& operator[size_t index]` | Returns a reference to the element at `index`. |
| `void assign(const T* first, const T* last)` | Assigns the given values from `first` to `last`. It also resizes if the size doesn't match. |
| `T* data()` | Returns a pointer to the start of the array. |
| `bool empty()` | Returns true if the array has been initialized with no size. |
| `void resize(size_t size)` | Resizes the array to the given size. |
| `size_t size()` | Returns the current size of the array. |

## trivial_vector interface
| Method | Description |
| --- | --- |
| `T& operator[size_t index]` | Returns a reference to the element at `index`. |
| `T& at(size_t index) const` | Returns the element at the given index. |
| `size_t capacity() const` | Returns the current capacity of the vector. |
| `void clear()` | Clear all elements in the vector. |
| `T* data()` | Returns a pointer to the start of the array in the vector. |
| `void emplace(const_iterator iter, Args&& args)` | Creates a new element at the given location in the vector. |
| `void emplace_back(Args&& args)` | Creates a new element and pushes it to the vector. |
| `bool empty() const` | Returns whether or not the vector is empty. |
| `iterator erase(const_iterator iter)` | Erases the element pointed to by the iterator. |
| `iterator erase(const_iterator first, const_iterator last)` | Erases the elements within the range pointed to by `first` and `last`. |
| `void pop_back()` | Removes the last element from the vector. |
| `void push_back(const T& value)` | Pushes a new value by copying it. |
| `void push_back(T&& value)` | Pushes a new value by moving it. |
| `void push_back(const T* first, const T* last)` | Pushes a range of values from `first` to `last`. |
| `void reserve(size_t size)` | Reserves the size of the array to `size`, without initializing any elements. |
| `void resize(size_t size)` | Resizes the vector to the given size. |
| `size_t size() const` | Returns the current amount of elements in the vector. |

## unordered_map interface
| Method | Description |
| --- | --- |
| `V& operator[const K& index]` | Returns a reference to the element with the given key `index` and inserts one if it doesn't exist. |
| `V& at(const K& index) const` | Returns a reference to the element with the given key and asserts if it doesn't exist. |
| `size_t capacity() const` | Returns the current capacity of the map. Always a power of two. |
| `void clear()` | Clear all elements in the map. |
| `bool empty() const` | Returns whether or not the map is empty. |
| `size_t erase(const K& index)` | Erase an element by its key. |
| `iterator erase(iterator iter)` | Erase an element by an iterator pointing to it. |
| `iterator insert(K&& index, V&& value)` | Inserts an element into the map with the key `index` and value `value`. Returns an iterator to the element. |
| `iterator find(const K& index) const` | Attempts to find the value with the given key `index`. Returns `end()` if not found. |
| `float load_factor() const` | Returns the load factor of the map, between 0 and 1. |
| `void reserve(size_t size)` | Reserves a buffer of `size` elements, if the map isn't already this big. |
| `size_t size() const` | Returns the current amount of elements in the map. |

## unordered_multimap interface
| Method | Description |
| --- | --- |
| `V& at(const K& index) const` | Returns a reference to the element with the given key and asserts if it doesn't exist. |
| `size_t capacity() const` | Returns the current capacity of the map. Always a power of two. |
| `void clear()` | Clear all elements in the map. |
| `bool empty() const` | Returns whether or not the map is empty. |
| `size_t erase(const K& index)` | Erase an element by its key. |
| `iterator erase(iterator iter)` | Erase an element by an iterator pointing to it. |
| `key_iterator erase(key_iterator iter)` | Erase an element by a key iterator pointing to it. |
| `iterator insert(K&& index, V&& value)` | Inserts an element into the map with the key `index` and value `value`. Returns an iterator to the element. |
| `key_iterator find(const K& index) const` | Attempts to find the value with the given key `index`. Returns `end()` if not found. |
| `float load_factor() const` | Returns the load factor of the map, between 0 and 1. |
| `void reserve(size_t size)` | Reserves a buffer of `size` elements, if the map isn't already this big. |
| `size_t size() const` | Returns the current amount of elements in the map. |

# Allocator Examples
Create an allocator which will attempt to use a linked list allocator for allocation, but fall back on malloc when full.
```cpp
// Create the allocator from some 16kb buffer and straight malloc
type_fallback_allocator<double, linked<16384, mallocator>, mallocator> alloc;
// Allocate and deallocate 3 doubles
double* p1 = alloc.allocate(3);
alloc.deallocate(p1, 3);
// Allocate and deallocate 4096 doubles, which should be handled by malloc
double* p2 = alloc.allocate(4096);
alloc.deallocate(p2, 4096);
```

Create an allocator that monitors when memory corruption has occurred around any allocations.
```cpp
#include <iostream>

// Create the allocator with std::cerr
type_overflow_allocator<double, mallocator, std::cerr> alloc;
// Allocate and deallocate 1 double
double* p = alloc.allocate(1);
// Write to the address before what was allocated, which is illegal
*(p - 1) = 32;
// When it deallocates it should give a message in the standard error stream
alloc.deallocate(p, 1);
```

Create an allocator which will use a cascading list allocator for anything less than 8kb and malloc for anything larger.
```cpp
// Create the allocator from a combination of a cascading 8kb list allocator and malloc
// Anything smaller than 8kb should use the cascading list allcoator, while anything larger should use malloc
type_segragator_allocator<double, 8192, cascading_allocator<linked<8192, mallocator>>, mallocator> alloc;
// Allocate 1024 doubles
double* p1 = alloc.allocate(1024);
// Allocate another 1024 doubles, which should force the allocator to create a new list allocator
double* p2 = alloc.allocate(1024);
// Allocate 2048 doubles, which should use malloc instead
double* p3 = alloc.allocate(2048);
// Deallocate all of it
alloc.deallocate(p1, 1024);
alloc.deallocate(p2, 1024);
alloc.deallocate(p3, 2048);
```

Create an allocator which will reuse earlier allocations in a freelist.
```cpp
// Create the allocator from a freelist, backed by malloc
type_segragator_allocator<double, 16, freelist<0, 16, mallocator>, mallocator> alloc;
// Allocate 1 double and deallocate, so that it ends up in the freelist
double* p1 = alloc.allocate(1);
alloc.deallocate(p1, 1);
// Allocate 2 doubles, which should reuse the previous allocation
double* p2 = alloc.allocate(2);
alloc.deallocate(p2, 2);
// Allocate 4 doubles, which should be handled by the backup malloc
double* p3 = alloc.allocate(4);
alloc.deallocate(p3, 4);
```

Create a non-typed allocator with many freelists of different sizes, backed by malloc for anything larger.
```cpp
// Just a shorthand for writing freelists
template<size_t Max>
using FList = freelist<0, Max, mallocator>;
// Create the allocator from various thresholded freelists, backed by a cascading linear allocator and malloc
using Alloc = segragator_builder_t<
    FList<8>,
    threshold<8>,
    FList<128>,
    threshold<128>,
    FList<512>,
    threshold<512>,
    FList<1024>,
    threshold<1024>,
    cascading<linear_allocator<4096>>,
    threshold<4096>,
    mallocator>;

Alloc alloc;
// Allocate and deallocate 256 bytes, which should use the third freelist
void* p1 = alloc.allocate(256);
alloc.deallocate(p1, 256);
// Allocate and deallocate 256 bytes, which should reuse the previous allocation
void* p2 = alloc.allocate(256);
alloc.deallocate(p2, 256);
// Allocate and deallocate 2048 bytes, which should use the cascading linear list
void* p3 = alloc.allocate(2048);
alloc.deallocate(p3, 2048);
```

# Building and running tests
The tests require premake5 as build system.
Generating project files can be done by running:
```bash
# Linux
premake5 gmake2 --toolset=gcc
# Windows
premake5 vs2019 --toolset=msc
```

Afterwards the tests can be built using the command below:
```
premake5 build --config=(release | debug)
```

The built binary can then be found in `bin/{{config}}-{{platform}}-{{architecture}}`.