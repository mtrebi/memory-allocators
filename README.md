# Memory allocators in C++
## Introduction
When applications need more memory this can be allocated in the heap (rather than in the stack) in _runtime_. This memory is called 'dynamic memory' because it can't be known at compile time and its need changes during the execution. Our programs can ask for dynamic memory usin 'malloc'. Malloc returns an address to a position in memory where we can store our data. Once we're done with that data, we can call 'free' to free the memory and let others processes use it.

For this project I've implemented different ways to manage by ourselves dynamic memory in C++.This means that instead of using native calls like 'malloc' or 'free' we're going to use a custom memory allocator that will do this for us but in a more efficient way.
The goal, then, is to understand how the most common allocators work, what they offer and compare them to see which one performs better.

## What's wrong with Malloc?
* _General purpose_: Being a general purpose operation means that it must work in all cases (from 1byte to 1GB). To achieve that, most of the times, the implementation is not as efficient as it could be if the needs were more specific.
* _Slow_: Sometimes, when allocating memory, malloc needs to change from user to kernel mode to get more memory from the OS. When this happens, malloc turns out to be super slow!

## Custom allocators
Because every program has specific needs, it makes no sense to use a general purpose allocator. We can choose the right allocator that works best for us. This way we can have a huge gain in _performance_.

In general, custom allocators share some features:
* _Low number of mallocs_: Any custom allocator tries to keep the number of mallocs low. To do that, they malloc _big chunks of memory_ and then, they manage this chunk internally to provide smaller allocations.
* _Data structures_: Secondary data structures like _Linked Lists_, _Trees_, _Stacks_ to manage these big chunks of memory. Usually they are used to keep track of the allocated and/or free portions of memory to _speed up_ operations.
* _Constraints_: Some allocators are very specific and have constraints over the data or operations that can be performed. This allows them to achieve a high performance but can only be used in some applications. 

### Linear allocator
This is the simplest kind of allocator. The idea is to keep a pointer at the first memory address of your memory chunk and move it every time an allocation is done. In this allocator, the internal fragmentation is kept to a minimum because all elements are sequentially (spatial locality) inserted and the only fragmentation between them is the alignment.

#### Data structure
This allocator only requires a pointer (or an offset) to tell us the position of the last allocation. It doesn't require any extra information or data structure.

_Complexity: *O(1)*_
#### Allocate
Simply move the pointer (or offset) forward.

_Complexity: *O(1)*_
#### Free
Due to its simplicity, this allocator doesn't allow specific positions of memory to be freed. Usually, all memory is freed together.

### Stack allocator
This is a smart evolution of the Linear Allocator. The idea is to manage the memory as a Stack. So, as before, we keep a pointer to the current memory address and we move it forward for every allocation. However, we also can move it backwards when a free operation is done. As before, we keep the spatial locality principle and the fragmentation is still very low.

#### Data structure
As I said, we need the pointer (or offset) to keep track of the last allocation. In order to be able to free memory, we also need to store a _header_ for each allocation that tell us the size of the allocated block. Thus, when we free, we know how many positions we have to move back the pointer. 

_Complexity: *O(N\*H)*_ where H is the Header size and N is the number of allocations
#### Allocate
Simply move the pointer (or offset) forward and place a header right before the memory block indicating its size.

_Complexity: *O(1)*_
#### Free
Simply read the block size from the header and move the pointer backwards.

_Complexity: *O(1)*_
### Pool allocator
A Pool allocator is quite different from the previous ones. It splits the big memory chunk in smaller chunks of the same size and keeps track of which of them are free. When an allocation is requested it returns the free chunk size. When a freed is done, it just stores it to be used in the next allocation. This way, allocations work super fast and the fragmentation is still very low.

#### Data structure
To keep track of the free blocks of memory, the Pool allocator uses a Linked List that links the address of each free memory block. To reduce the space needed, this Linked List is stored in the same free blocks (smart right?). However, this set the constraint that the data chunks must be at least as big as our nodes in the Linked List (so that, we can store the Linked List in the free memory blocks).

_Complexity: *O(1)*_ 
#### Allocate
An allocation simply means to take (pop) the first free block of the Linked List.

_Complexity: *O(1)*_
#### Free
Free means to add (push) the freed element as the first element in the Linked List.

_Complexity: *O(1)*_
### Free list allocator

#### Sequential implementation
##### Data structure


_Complexity: *O(N\*HF + M\*HA)*_ where N is the number of free blocks, HF is the size of the header of free blocks, M the number of allocator blocks and HA the size of the header of allocated blocks
##### Allocate
Simply look for the first free block that has a size equal or bigger than the size requested (it can store this data) and remove it from the Linked List.

_Complexity: *O(N)*_ where N is the number of free blocks
##### Free
Simply look the right position where the free block should be put in the Sorted Linked List of free blocks. If we used instead a Sorted Linked List of free and allocated blocks, the complexity would be *O(1)* but the allocation compleixity would be *O(N) where N is the number of free and allocated blocks and space complexity would be much higher. When we free a memory block we also look at the previous and next blocks to see if we can merge them into one bigger block. This operations is called coalescence and is O(1) because the Linked List is sorted.

_Complexity: *O(N)*_ where N is the number of free blocks
#### Red black tree implementation
##### Data structure

_Complexity: *O(N\*HF + M\*HA)*_ 
##### Allocate
Now, instead of iterating the Linked List (as before) to find the a free memory block where our data can fit, we find in the Red Black Tree. This operation is not only faster, but also allows us to pick the best free memory block. This means that the space wasted is minimized. Once we have the node, we remove it from the tree.

_Complexity: *O(log N)*_ where N is the number of free blocks
##### Free

_Complexity: *O(log N)*_ where N is the number of free blocks

### Buddy allocator
 *TODO*
### Slab allocator
 *TODO*
## Benchmarks
    *TODO*:
        What is measured, how
        Individual results
        Comparisions
## Which allocator should I use?
    *TODO*:
        Avoid dynamic memory as much as possible (unexpected behavior)
        Draw what to choose (know your data)
            Anything better than malloc
            Know your program, know your data
            A freelist allocator is a "general purpose" custom allocator that works better than malloc
## Future work
* Benchmark internal fragmentation
* Benchmark spatial location (cache misses)

## Acknowledgments

Thanks to [Vanessa](https://github.com/vipyne) and [Krish](https://github.com/sigmasleep) for helping me in the various stages of this project.

