<div align="center">
<h1>Krede Template Library</h1>

![Windows supported](https://img.shields.io/badge/Windows-win--10-green?style=flat-square)
![Linux supported](https://img.shields.io/badge/Linux-Ubuntu-green?style=flat-square)
![MacOS untested](https://img.shields.io/badge/MacOS-Untested-red?style=flat-square)

<p>A library containing various composite allocators and containers.</p>

[![Release](https://img.shields.io/github/v/release/KredeGC/KTL?display_name=tag&style=flat-square)](https://github.com/KredeGC/KTL/releases)
[![License](https://img.shields.io/github/license/KredeGC/KTL?style=flat-square)](https://github.com/KredeGC/KTL/blob/master/LICENSE)
[![Issues](https://img.shields.io/github/issues/KredeGC/KTL?style=flat-square)](https://github.com/KredeGC/KTL/issues)
[![Tests](https://img.shields.io/github/workflow/status/KredeGC/KTL/Test?style=flat-square)](https://github.com/KredeGC/KTL/actions/workflows/main.yml)

</div>

# Status
The interface is currently not in a stable state, so expect changes between updates to cause some disruption, as functions or types may not exist between versions.
In addition, most allocators presented are not thread-safe.
For now thread safety is not on the roadmap as it would degrade performance on single-threaded applications.
The library itself is fairly stable and should be able to be used in a production environment.

# Installation
As this is a header-only library, you can simply copy the header files directly into your project.
The header files can either be downloaded from the [release page](https://github.com/KredeGC/KTL/releases) or from the `include/` directory on the master branch.
The source and header files inside the `src/` directory are only tests and should not be included into your project.

# Containers
This library also contains various containers that are STL compliant.

| Signature | Description | Notes |
| --- | --- | --- |
| binary_heap<br/>\<T, Comp, Alloc\> | A binary heap, sorted using the `Comp` and allocated using the given `Alloc` allocator. | `Comp` can be either `std::greater<T>` or `std::less<T>` or some other custom implementation.<br/>A shorthand version of both a min and a max heap can be used, via the `binary_min_heap<T, Alloc>` type. |
| trivial_vector<br/>\<T, Alloc\> | A vector class, similar to `std::vector`, but optimized for trivial types. Takes a type `T` and an allocator `Alloc`. | The container uses a straight `memcpy` for most of its operations.<br/>It's not recommended to use this with non-trivial types, eg. types that have custom default, copy or move constructors or destructors. |

# Allocators
This library contains 2 different types of allocators:
* Raw void* allocators - Do the actual allocation/deallocation and construction/destruction
* Composite/synthetic allocators - Attach to other allocators to provide extra features on top

Both of these allocator types are not stl standard-compliant, but can be made to be if used with the `type_allocator<T, Allocator>` type.<br/>
This is a composite allocator that you can use to make an allocator typed, like so: `type_allocator<int, mallocator>`.<br/>
All allocators also have a typedef version with a `type_` prefix as a shorthand, such as: `type_mallocator<int>`.

| Signature | Type | Description |
| --- | --- |--- |
| mallocator | Raw | An allocator which tries to align memory when allocating.<br/>Almost like std::allocator, except it has no type. |
| pre_allocator<br/>\<Size\> | Raw | Uses a linked list to determine whether a given chunk of memory is free or allocated, which takes O(n) time.<br/>Has a max allocation size of the `Size` given. |
| stack_allocator<br/>\<Size\> | Raw | Uses a preallocated `stack<Size>`, which has to be passed in during construction.<br/>Simply increments a counter during allocation, making it faster than pre_allcoator, but it also rarely deallocates.<br/>Has a max allocation size of the `Size` given. |
| composite_allocator<br/>\<Primary, Fallback\> | Composite | Delegates allocation between 2 allocators.<br/>It first attempts to allocate with the `Primary` allocator, but upon failure will use the `Fallback` allocator. |
| overflow_allocator<br/>\<Allocator, std::ostream\> | Composite | Checks for memory corruption/leak when allocating/constructing via it's specified allocator. It streams the results to the std::ostream specified. |
| segragator_allocator<br/>\<Threshold, Primary, Fallback\> | Composite | Delegates allocation between 2 allocators based on a size threshold. |
| type_allocator<br/>\<T, Allocator\> | Composite | Wraps around the specified allocator with a type. This can be used to make the other allocators stl compliant, so they can be used with stl containers. |

NOTES:
Exceptions are not used with any of the allocators above. This means that upon failure, they will simply return a null pointer to indicate that they were unable to allocate anything. Some synthethic allocators may rely upon this nullptr feature, like composite_allocator, which upon failure will attempt to use the fallback allocator instead.

# Usage

# Building and running tests