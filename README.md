<div align="center">
<h1>Krede Template Library</h1>

![Windows supported](https://img.shields.io/badge/Windows-win--10-green?style=flat-square)
![Linux supported](https://img.shields.io/badge/Linux-Ubuntu-green?style=flat-square)
![MacOS untested](https://img.shields.io/badge/MacOS-Untested-red?style=flat-square)

<p>A library containing various composite memory allocators and containers.</p>

[![Release](https://img.shields.io/github/v/release/KredeGC/KTL?display_name=tag&style=flat-square)](https://github.com/KredeGC/KTL/releases)
[![License](https://img.shields.io/github/license/KredeGC/KTL?style=flat-square)](https://github.com/KredeGC/KTL/blob/master/LICENSE)
[![Issues](https://img.shields.io/github/issues/KredeGC/KTL?style=flat-square)](https://github.com/KredeGC/KTL/issues)
[![Tests](https://img.shields.io/github/workflow/status/KredeGC/KTL/Test?style=flat-square)](https://github.com/KredeGC/KTL/actions/workflows/main.yml)

</div>

# Status
The interface is currently not in a stable state, so expect changes between updates to cause some disruption, as functions or types may not exist between versions.
In addition, most allocators presented are not thread-safe.
For now thread safety is not on the roadmap as it would degrade performance on single-threaded applications.
The library itself is fairly stable but should not be expected to be used in a production environment for the time being.

# Installation
As this is a header-only library, you can simply copy the header files directly into your project.
The header files can either be downloaded from the [release page](https://github.com/KredeGC/KTL/releases) or from the `include/` directory on the master branch.
The source and header files inside the `src/` directory are only tests and should not be included into your project.

# Containers
This library contains various containers that are STL compliant.

| Signature | Description | Notes |
| --- | --- | --- |
| binary_heap<br/>\<T, Comp, Alloc\> | A binary heap, sorted using the `Comp` and allocated using the given `Alloc` allocator. | `Comp` can be either `std::greater<T>` or `std::less<T>` or some other custom implementation.<br/>A shorthand version of both a min and a max heap can be used, via the `binary_min_heap<T, Alloc>` and `binary_max_heap<T, Alloc>` types. |
| trivial_vector<br/>\<T, Alloc\> | A vector class, similar to `std::vector`, but optimized for trivial types. Takes a type `T` and an allocator `Alloc`. | The container uses a straight `memcpy` for most of its operations.<br/>It's not recommended to use this with non-trivial types, eg. types that have custom default, copy or move constructors or custom destructors. |

## binary_heap interface
The methods of the trivial_vector roughly follows the STL vector.
There are some additional methods added, like a range-based `push_back`.

| Method | Description |
| --- | --- |
| `void clear()` | Clear all elements in the heap. |
| `T pop()` | Removes the root element (lowest or highest, depending on min or max heap) and returns it. |
| `void insert(const T& value)` | Pushes a new element into the heap by copying. |
| `void insert(T&& value)` | Pushes a new element into the heap by moving. |

## trivial_vector interface
The methods of the trivial_vector roughly follows the STL vector.
There are some additional methods added, like a range-based `push_back`.

| Method | Description |
| --- | --- |
| `void clear()` | Clear all elements in the vector. |
| `void emplace_back(Args&& args)` | Creates a new element and pushes it to the vector. |
| `void pop_back()` | Removes the last element from the vector. |
| `void push_back(const T& value)` | Pushes a new value by copying it. |
| `void push_back(T&& value)` | Pushes a new value by moving it. |
| `void push_back(const T* begin, const T* end)` | Pushes a range of values from `begin` to `end`. |
| `void reserve(size_t size)` | Reserves the size of the array to `size`, without initializing any elements. |
| `void resize(size_t size)` | Resizes the vector to the given size. |

# Allocators
This library also contains 2 different types of allocators:
* Raw void* allocators - Do the actual allocation/deallocation and construction/destruction
* Composite/synthetic allocators - Attach to other allocators to provide extra features on top

Both of these allocator types are not STL compliant, but can be made to be if used with the `type_allocator<T, Allocator>` type.<br/>
This is a composite allocator that you can use to make an allocator typed, like so: `type_allocator<int, mallocator>`.<br/>
All allocators also have a typedeffed version with a `type_` prefix as a shorthand, such as: `type_mallocator<int>`.

| Signature | Type | Description |
| --- | --- |--- |
| mallocator | Raw | An allocator which tries to align memory when allocating.<br/>Almost like std::allocator, except it has no type. |
| pre_allocator<br/>\<Size\> | Raw | Uses a linked list to determine whether a given chunk of memory is free or allocated, which takes O(n) time.<br/>Has a max allocation size of the `Size` given. |
| stack_allocator<br/>\<Size\> | Raw | Uses a preallocated `stack<Size>`, which has to be passed in during construction.<br/>Simply increments a counter during allocation, making it faster than pre_allcoator, but it also rarely deallocates.<br/>Has a max allocation size of the `Size` given. |
| cascading_allocator<br/>\<Allocator\> | Composite | Attempts to allocate using the given allocator, but upon failure will create a new allocator and keep a reference to the old one.<br/>Deallocation can take O(n) time as it may have to traverse multiple allocator instances to find the right one. |
| composite_allocator<br/>\<Primary, Fallback\> | Composite | Delegates allocation between 2 allocators.<br/>It first attempts to allocate with the `Primary` allocator, but upon failure will use the `Fallback` allocator. |
| overflow_allocator<br/>\<Allocator, std::ostream\> | Composite | Checks for memory corruption/leak when allocating/constructing via it's specified allocator. It streams the results to the std::ostream specified. |
| segragator_allocator<br/>\<Threshold, Primary, Fallback\> | Composite | Delegates allocation between 2 allocators based on a size threshold. |
| type_allocator<br/>\<T, Allocator\> | Composite | Wraps around the specified allocator with a type. This can be used to make the other allocators STL compliant, so they can be used with STL containers. |

NOTES:
Exceptions are not used with any of the allocators above. This means that upon failure, they will simply return a null pointer to indicate that they were unable to allocate anything. Some synthethic allocators may rely upon this nullptr feature, like composite_allocator, which upon failure will attempt to use the fallback allocator instead.

## Allocator interface
The allocators roughly follow the standard for STL allocators, except that they are not typed, so use void* instead.
Some additional methods have also been added.

| Method | Description |
| --- | --- |
| void* allocate(size_type size) | Attempts to allocate a chunk of memory defined by `size`. |
| void deallocate(void* ptr, size_type size) | Attempts to deallocate the memory at location `ptr` with the given size, `size`. |
| void construct(T* ptr, Args&& args) | Calls the constructor of a specific type at the location `ptr` with `args`.<br/>Not all allocators define this method. |
| void destroy(T* ptr) | Calls the destructor of a specific type at the location `ptr`.<br/>Not all allocators define this method. |
| size_type max_size() | Returns the maximum size this allocator could possibly allocate.<br/>Not all allocators define this method. |
| bool owns(void* ptr) | Returns whether or not the given memory at location `ptr` is owned by this allocator.<br/>Not all allocators define this method, such as `mallocator`. |
| Alloc get_allocator() | Returns the allocator that this allocator wraps around.<br/>Most composite allocators define this method. |

# Usage

# Building and running tests
The tests require premake5 as build system.
Generating project files can be done by running:
```bash
// Linux
premake5 gmake2 --toolset=gcc
// Windows
premake5 vs2019 --toolset=msc
```

Afterwards the tests can be built using the command below:
```
premake5 build --config=(release | debug)
```

The built binary can then be found in `bin/{{config}}-{{platform}}-{{architecture}}`.