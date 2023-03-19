<div align="center">
<h1>Krede Template Library</h1>

![Windows supported](https://img.shields.io/badge/Windows-win--10-green?style=flat-square)
![Linux supported](https://img.shields.io/badge/Linux-Ubuntu-green?style=flat-square)
![MacOS untested](https://img.shields.io/badge/MacOS-Untested-red?style=flat-square)

A C++ library containing various composable memory allocators and containers.<br/>
Allocators are mostly based on a [Talk by Andrei Alexandrescu](https://www.youtube.com/watch?v=LIb3L4vKZ7U).

[![Release](https://img.shields.io/github/v/release/KredeGC/KTL?display_name=tag&style=flat-square)](https://github.com/KredeGC/KTL/releases/latest)
[![Size](https://img.shields.io/github/languages/code-size/KredeGC/KTL?style=flat-square)](https://github.com/KredeGC/KTL/releases/latest)
[![License](https://img.shields.io/github/license/KredeGC/KTL?style=flat-square)](https://github.com/KredeGC/KTL/blob/master/LICENSE)

[![Issues](https://img.shields.io/github/issues/KredeGC/KTL?style=flat-square)](https://github.com/KredeGC/KTL/issues)
[![Tests](https://img.shields.io/github/actions/workflow/status/KredeGC/KTL/main.yml?branch=master&style=flat-square)](https://github.com/KredeGC/KTL/actions/workflows/main.yml)

</div>

# Status
The interface is in a mostly stable state, so severe changes should rarely occur.
In addition, most allocators presented are not thread-safe.
For now thread safety is not on the roadmap as it would degrade performance on single-threaded applications.
The functionality is fairly stable but should not be expected to be used in a production environment for the time being.

# Table of Content
* [Compatibility](#compatibility)
* [Installation](#installation)
* [Usage](#usage)
* [Allocators](#allocators)
* [Allocator Interface](#allocator-interface)
* [Containers](#containers)
  * [binary_heap interface](#binary_heap-interface)
  * [trivial_array interface](#trivial_array-interface)
  * [trivial_vector interface](#trivial_vector-interface)
* [Allocator examples](#allocator-examples)
* [Building and running tests](#building-and-running-tests)

# Compatibility
This library was made with C++17 in mind and is not compatible with earlier versions.
Backwards compatability is currently not on the roadmap.

# Installation
As this is a header-only library, you can simply copy the header files directly into your project.
The header files can either be downloaded from the [releases page](https://github.com/KredeGC/KTL/releases) or from the [`include/`](https://github.com/KredeGC/KTL/tree/master/include/ktl) directory on the master branch.
The source and header files inside the `src/` directory are only tests and should not be included into your project unless you want to run them yourself.

# Usage
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

Allocators always use an alignment of at least 8 bytes.

Raw allocators do not work like the STL `std::allocator`, in that 2 instances do not share the same state.<br/>
If you make a copy of a raw allocator it will not be able to deallocate anything that was allocated by the original.<br/>
If you want the state to be shared you can wrap it in a `shared<Allocator>` type, which will use ref-counting when copying and moving the allocator.<br/>
You may also want the allocator to be thread-safe, in which case you can instead wrap it in a `threaded<Allocator>` type.

In addition to this all STL containers require a typed allocator.<br/>
To make an allocator typed you can wrap it in a `type_allocator<T, Allocator>` type.<br/>
This is a composite allocator that you can use to make an allocator typed, like so: `type_allocator<int, linear_allocator<1024>>`.<br/>
All allocators also have a typedeffed version with a `type_` prefix as a shorthand, such as: `type_linear_allocator<int, 1024>`.<br/>
If you want to use an allocator with any STL container you should always make sure that it has shared state.

Unlike STL containers, the containers in this library do not copy or move the allocators (once passed in), so you should be able to use them without needing shared state.<br/>
However, if the allocator is not default/copy constructible or you want to use the same allocator for multiple containers, then you must use shared state like above.

If you are unsure about type-safety, STL and shared state, you can always just wrap the entire allocator in both a `type_allocator` and a `shared` type.<br/>
Like so: `type_allocator<int, shared<linear_allocator<1024>>>` or using the typedeffed version: `type_shared_linear_allocator<int, 1024>`.<br/>
This will ensure that the allocator works with any type of container, STL or not.

| Signature | Type | Description |
| --- | --- |--- |
| `linear_allocator<Size>` | Raw | Allocates a block of `Size` which it then hands out in chunks, similar to `stack_allocator`.<br/>Simply increments a counter during allocation, making allocations very fast, but it also rarely deallocates.<br/>Has a max allocation size of the `Size` given, but unlike the `stack_allocator` keeps its memory internally. |
| `mallocator` | Raw | An allocator which tries to align memory when allocating.<br/>Almost like std::allocator, except it has no type. |
| `null_allocator` | Raw | An allocator which allocates and owns nothing.<br/>Useful for ensuring that a composite allocator doesn't use a specific path when allocating. |
| `stack_allocator<Size>` | Raw | Uses a preallocated `stack<Size>`, which has to be passed in during construction.<br/>Simply increments a counter during allocation, making allocations very fast, but it also rarely deallocates.<br/>Has a max allocation size of the `Size` given. |
| `cascading<Allocator>` | Composite | Attempts to allocate using the given allocator, but upon failure will create a new allocator and keep a reference to the old one.<br/>Deallocation can take O(n) time as it may have to traverse multiple allocator instances to find the right one.<br/>The allocator type must be default-constructible, which means the `stack_allocator` can't be used. |
| `fallback<Primary, Fallback>` | Composite | Delegates allocation between 2 allocators.<br/>It first attempts to allocate with the `Primary` allocator, but upon failure will use the `Fallback` allocator. |
| `freelist<Min, Max, Alloc>` | Composite | Allocates using the given allocator, if the size specified is within the range of `Min` and `Max`, otherwise returns `nullptr`.<br/>When deallocating, it keeps the free memory in a linked list which can be reused on later allocations. |
| `overflow<Allocator, std::ostream>` | Composite | Checks for memory corruption/leak when allocating/constructing via it's specified allocator. It streams the results to the std::ostream specified. |
| `segragator<Threshold, Primary, Fallback>` | Composite | Delegates allocation between 2 allocators based on a size threshold. |
| `shared<Allocator>` | Composite | Wraps around the specified allocator, making it ref-counted. This can be used to make an allocator STL compliant, so they can be used with STL containers. |
| `threaded<Allocator>` | Composite | Wraps around the specified allocator, making it ref-counted and thread-safe. This can be used to make an allocator STL compliant, so they can be used with STL containers. |
| `type_allocator<T, Allocator>` | Composite | Wraps around the specified allocator with a type. This can be used to make an allocator STL compliant, so they can be used with STL containers. |

NOTES:
Exceptions are not used with any of the allocators above. This means that upon failure, they will simply return a null pointer to indicate that they were unable to allocate anything. Some synthethic allocators may rely upon this nullptr feature, like fallback_allocator, which upon failure will attempt to use the given `Fallback` allocator instead.

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
| [trivial_array<br/>\<T, Alloc\>](#trivial_array-interface) | An array wrapper class, similar to `std::array`, but uses dynamic allocation and is optimized for trivial types. Takes a type `T` and an allocator `Alloc`. | The container uses a straight `memcpy` for most of its operations.<br/>It's not recommended to use this with non-trivial types, eg. types that have custom default, copy or move constructors or custom destructors. |
| [trivial_vector<br/>\<T, Alloc\>](#trivial_vector-interface) | A vector class, similar to `std::vector`, but optimized for trivial types. Takes a type `T` and an allocator `Alloc`. | The container uses a straight `memcpy` for most of its operations.<br/>It's not recommended to use this with non-trivial types, eg. types that have custom default, copy or move constructors or custom destructors. |

## binary_heap interface
| Method | Description |
| --- | --- |
| `size_t capacity() const` | Returns the current capacity of the heap. |
| `void clear()` | Clear all elements in the heap. |
| `T* data() const` | Returns a pointer to the start of the heap. |
| `bool empty() const` | Returns true if the heap has no elements. |
| `iterator find(const K& index) const` | Returns an iterator to the element `index`. Returns `end()` if not found. Takes O(n) time. |
| `void insert(const T& value)` | Pushes a new element into the heap by copying. |
| `void insert(T&& value)` | Pushes a new element into the heap by moving. |
| `T& peek()` | Peeks at the root element (lowest or highest, depending on min or max heap) and returns a reference to it. |
| `T pop()` | Removes the root element (lowest or highest, depending on min or max heap) and returns it. |
| `void reserve(size_t size)` | Reserves the capacity of the heap to `size`, without initializing any elements. |
| `size_t size() const` | Returns the current size of the heap. |

## trivial_array interface
| Method | Description |
| --- | --- |
| `T& operator[size_t index]` | Returns a reference to the element at `index`. |
| `void assign(const T* first, const T* last)` | Assigns the given values from `first` to `last`. It also resizes if the size doesn't match. |
| `T& at(size_t index) const` | Returns the element at the given index. |
| `T* data() const` | Returns a pointer to the start of the array. |
| `bool empty() const` | Returns true if the array has been initialized with no size. |
| `void resize(size_t size)` | Resizes the array to the given size. |
| `size_t size() const` | Returns the current size of the array. |

## trivial_vector interface
| Method | Description |
| --- | --- |
| `T& operator[size_t index]` | Returns a reference to the element at `index`. |
| `T& at(size_t index) const` | Returns the element at the given index. |
| `size_t capacity() const` | Returns the current capacity of the vector. |
| `void clear()` | Clear all elements in the vector. |
| `T* data() const` | Returns a pointer to the start of the array in the vector. |
| `void emplace(const_iterator iter, Args&& args)` | Creates a new element at the given location in the vector. |
| `void emplace_back(Args&& args)` | Creates a new element and pushes it to the vector. |
| `bool empty() const` | Returns whether or not the vector is empty. |
| `iterator erase(const_iterator iter)` | Erases the element pointed to by the iterator. |
| `iterator erase(const_iterator first, const_iterator last)` | Erases the elements within the range pointed to by `first` and `last`. |
| `void pop_back()` | Removes the last element from the vector. |
| `iterator push_back(const T& value)` | Pushes a new value by copying it. |
| `iterator push_back(T&& value)` | Pushes a new value by moving it. |
| `iterator push_back(const T* first, const T* last)` | Pushes a range of values from `first` to `last`. |
| `void reserve(size_t size)` | Reserves the size of the array to `size`, without initializing any elements. |
| `void resize(size_t size)` | Resizes the vector to the given size. |
| `size_t size() const` | Returns the current amount of elements in the vector. |

# Allocator Examples
The following examples all have `using namespace ktl` or equivalent at the top for brevity.
These examples can all be seen as unit tests in [`src/test/exotic_allocator_test.cpp`](https://github.com/KredeGC/KTL/tree/master/src/test/exotic_allocator_test.cpp).

Create an allocator which will attempt to use a linear allocator for allocation, but fall back on malloc when full.
```cpp
// Create the allocator from some 8kb buffer and straight malloc
type_fallback_allocator<double, linear_allocator<8192>, mallocator> alloc;
// Allocate and deallocate 3 doubles
double* p1 = alloc.allocate(3);
alloc.deallocate(p1, 3);
// Allocate and deallocate 2048 doubles, which should be handled by malloc
double* p2 = alloc.allocate(2048);
alloc.deallocate(p2, 2048);
```

Create an allocator that monitors when memory corruption has occurred around any allocations.
```cpp
#include <iostream>

// Create the allocator with std::cerr
type_overflow_allocator<double, mallocator> alloc(std::cerr);
// Allocate and deallocate 1 double
double* p = alloc.allocate(1);
// Write to the address before what was allocated, which is illegal
*(p - 1) = 32;
// When it deallocates it should give a message in the standard error stream
alloc.deallocate(p, 1);
```

Create an allocator which will use a cascading list allocator for anything less than 8kb and malloc for anything larger.
```cpp
// Create the allocator from a combination of a cascading 8kb linear allocator and malloc
// Anything smaller than 8kb should use the cascading linear allocator, while anything larger should use malloc
type_segragator_allocator<double, 8192, cascading<linear_allocator<8192>>, mallocator> alloc;
// Allocate 1024 doubles
double* p1 = alloc.allocate(1024);
// Allocate another 1024 doubles, which should force the allocator to create a new linear allocator
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
using Alloc = segragator_builder_max<
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
// Allocate and deallocate 2048 bytes, which should use the cascading linear allocator
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
```bash
premake5 build --config=(release | debug)
```

You can also run the tests using the command below, or simply run the binary located in `bin/{{config}}-{{platform}}-{{architecture}}`:
```bash
premake5 test --config=(release | debug)
```