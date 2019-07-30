<a href='http://www.recurse.com' title='Made with love at the Recurse Center'><img src='https://cloud.githubusercontent.com/assets/2883345/11325206/336ea5f4-9150-11e5-9e90-d86ad31993d8.png' height='20px'/></a>

# Table of Contents
&nbsp;[Introduction](https://github.com/mtrebi/memory-allocators#introduction)  <br/> 
&nbsp;[Build instructions](https://github.com/mtrebi/memory-allocators#build-instructions)  <br/> 
&nbsp;[What's wrong with Malloc?](https://github.com/mtrebi/memory-allocators#whats-wrong-with-malloc)  <br/> 
&nbsp;[Custom allocators](https://github.com/mtrebi/memory-allocators#custom-allocators)  <br/> 
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Linear Allocator](https://github.com/mtrebi/memory-allocators#linear-allocator)  <br/> 
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Stack Allocator](https://github.com/mtrebi/memory-allocators#stack-allocator)  <br/> 
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Pool Allocator](https://github.com/mtrebi/memory-allocators#pool-allocator)  <br/> 
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Free list Allocator](https://github.com/mtrebi/memory-allocators#free-list-allocator)  <br/> 
&nbsp;[Benchmarks](https://github.com/mtrebi/memory-allocators#benchmarks)  <br/> 
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Time complexity](https://github.com/mtrebi/memory-allocators#time-complexity)  <br/> 
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Space complexity](https://github.com/mtrebi/memory-allocators#space-complexity)  <br/> 
&nbsp;[Summary](https://github.com/mtrebi/memory-allocators#summary)  <br/> 
&nbsp;[Last thoughts](https://github.com/mtrebi/memory-allocators#last-thoughts)  <br/> 
&nbsp;[Acknowledgments](https://github.com/mtrebi/memory-allocators#acknowledgments)  <br/> 

# Introduction
When applications need more memory this can be allocated in the heap (rather than in the stack) in _runtime_. This memory is called 'dynamic memory' because it can't be known at compile time and its need changes during the execution. Our programs can ask for dynamic memory usin 'malloc'. Malloc returns an address to a position in memory where we can store our data. Once we're done with that data, we can call 'free' to free the memory and let others processes use it.

For this project I've implemented different ways to manage by ourselves dynamic memory in C++.This means that instead of using native calls like 'malloc' or 'free' we're going to use a custom memory allocator that will do this for us but in a more efficient way.
The goal, then, is to understand how the most common allocators work, what they offer and compare them to see which one performs better.

# Build instructions

```c
git clone https://github.com/mtrebi/memory-allocators.git
cmake -S memory-allocator -B build 
cmake --build build
```

# What's wrong with Malloc?
* **General purpose**: Being a general purpose operation means that it must work in all cases (from 1byte to 1GB or more...). For this reason the implementation is not as efficient as it could be if the needs were more specific.
* **Slow**: Sometimes, when allocating memory, malloc needs to change from user to kernel mode to get more memory from the system. When this happens, malloc turns out to be super slow!

# Custom allocators
Because every program has specific needs, it makes no sense to use a general purpose allocator. We can choose the right allocator that works best for us. This way we can increase our **performance**.

In general, custom allocators share some features:
* **Low number of mallocs**: Any custom allocator tries to keep the number of mallocs low. To do that, they malloc _big chunks of memory_ and then, they manage this chunk internally to provide smaller allocations.
* **Data structures**: Secondary data structures like _Linked Lists_, _Trees_, _Stacks_ to manage these big chunks of memory. Usually they are used to keep track of the allocated and/or free portions of memory to _speed up_ operations.
* **Constraints**: Some allocators are very specific and have constraints over the data or operations that can be performed. This allows them to achieve a high performance but can only be used in some applications. 

## Linear allocator
This is the simplest kind of allocator. The idea is to keep a pointer at the first memory address of your memory chunk and move it every time an allocation is done. In this allocator, the internal fragmentation is kept to a minimum because all elements are sequentially (spatial locality) inserted and the only fragmentation between them is the alignment.

### Data structure
This allocator only requires a pointer (or an offset) to tell us the position of the last allocation. It doesn't require any extra information or data structure.

![Data structure of a Linear Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/linear1.png)

_Complexity: **O(1)**_

### Allocate
Simply move the pointer (or offset) forward.

![Allocating memory in a Linear Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/linear2.png)

_Complexity: **O(1)**_

### Free
Due to its simplicity, this allocator doesn't allow specific positions of memory to be freed. Usually, all memory is freed together.

## Stack allocator
This is a smart evolution of the Linear Allocator. The idea is to manage the memory as a Stack. So, as before, we keep a pointer to the current memory address and we move it forward for every allocation. However, we also can move it backwards when a free operation is done. As before, we keep the spatial locality principle and the fragmentation is still very low.

### Data structure
As I said, we need the pointer (or offset) to keep track of the last allocation. In order to be able to free memory, we also need to store a _header_ for each allocation that tell us the size of the allocated block. Thus, when we free, we know how many positions we have to move back the pointer. 

![Data structure of a Stack Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/stack1.png)

_Complexity: **O(N*H) --> O(N)**_ where H is the Header size and N is the number of allocations

### Allocate
Simply move the pointer (or offset) forward and place a header right before the memory block indicating its size.

![Allocating memory in a Stack Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/stack2.png)

_Complexity: **O(1)**_

### Free
Simply read the block size from the header and move the pointer backwards given that size.

![Freeing memory in a Stack Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/stack3.png)

_Complexity: **O(1)**_

## Pool allocator
A Pool allocator is quite different from the previous ones. It splits the big memory chunk in smaller chunks of the same size and keeps track of which of them are free. When an allocation is requested it returns the free chunk size. When a freed is done, it just stores it to be used in the next allocation. This way, allocations work super fast and the fragmentation is still very low.

![Splitting scheme in a Pool Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/pool1.png)

### Data structure
To keep track of the free blocks of memory, the Pool allocator uses a Linked List that links the address of each free memory block. 

![Linked List used in a Pool Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/pool2.png)

To reduce the space needed, this **Linked List is stored in the same free blocks** (smart right?). However, this set the constraint that the data chunks must be at least as big as our nodes in the Linked List (so that, we can store the Linked List in the free memory blocks).

![In memory Linked List used in a Pool Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/pool3.png)

_Complexity: **O(1)**_ 

### Allocate
An allocation simply means to take (pop) the first free block of the Linked List.

![Allocation in a Pool Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/pool4.png)

The linked list doesn't have to be sorted. Its order its determined by the how the allocations and free are done.

![Random State of a Linked List in a Pool Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/pool5.png)

_Complexity: **O(1)**_

### Free
Free means to add (push) the freed element as the first element in the Linked List.

_Complexity: **O(1)**_

## Free list allocator

This is a general purpose allocator that, contrary to the others, doesn't impose any restriction. It allows allocations and deallocations to be done in any order. For this reason, its performance is not as good as its predecessors. Depending on the data structure used to speed up this allocator, there are two common implementations: one that uses a Linked List and one that uses a Red black tree.

### Linked list data structure
As the name says, this implementation uses a Linked List to store, in a sorted manner, the start address of each free contiguous block in memory and its size.
When an allocation is requested, it searches in the linked list for a block where the data can fit. Then it removes the element from the linked list and places an allocation header (used on deallocations) right before the data (as we did in the Stack allocator).
On deallocations, we get back the allocation header to know the size of the block that we are going to free. Once we free it we insert it into the sorted linked list and we try to merge contiguous memory blocks together creating bigger blocks.

*Notes: My implementation has some constraints on the size and alignment of the data that can be allocated using **this** allocator. For example, the minimum size that can be allocated should be equals or bigger than the size required of a Free Node. Otherwise, we would be wasting more space in meta-data than in real data (Something similar happens with the alignment) These constraints are related with my implementation. A better implementation would probably handle these cases. I decided not to do so because performance would be drastically affected. In these cases its probably better to use a different allocator.*

![Data structure in a Free list Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/freelist_seq1.png)

_Complexity: **O(N*HF + M*HA)--> O(M)**_ where N is the number of free blocks, HF is the size of the header of free blocks, M the number of allocated blocks and HA the size of the header of allocated blocks


### Linked list Allocate
 When an allocation is requested, we look for a block in memory where our data can fit. This means that we have to iterate our linked list until we find a block that has a size equal or bigger than the size requested (it can store this data plus the allocation header) and remove it from the linked list. This would be a **first-fit** allocation because it stops when it finds the first block where the memory fits. There is another type of search called **best-fit** that looks for the free memory block of smaller size that can handle our data. The latter operation may take more time because is always iterating through all elements but it can reduce fragmentation.

![Allocating in a Free list Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/freelist_seq2.png)

_Complexity: **O(N)**_ where N is the number of free blocks 

### Linked list Free
First of all we get back the information about the allocation from the header. Then, we iterate the Linked List to intert the free block in the right position (because is sorted by address). Once we've inserted it, we merge contiguous blocks. We can merge in _O(1) because our Linked List is sorted. We only need to look at the previous and next elements in the linked list to see if we can merge this contiguous blocks. This operation of merging contiguous memory blocks to create bigger blocks is called _Coalescence_
If we used instead a Sorted Linked List of free and allocated blocks, the complexity would be *O(1)* but the allocation compleixity would be *O(N) where N is the number of free and allocated blocks and space complexity would be much higher. When we free a memory block we also look at the previous and next blocks to see if we can merge them into one bigger block. 

![Freeing in a Free list Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/freelist_seq3.png)

_Complexity: **O(N)**_ where N is the number of free blocks

### Red black tree data structure
The purpose of using a Red black tree is to speed up allocations and deallocations. In the Linked List (or sequential) implementation every time an operation was made we needed to iterate the Linked List. This was O(N) in all cases.

Using Red Black trees we can reduce its complexity to O(log N) while keeping space complexity quite low because the tree data is stored inside the free memory blocks. In addition, this structure allows a **best-fit** algorithm to be used, reducing the fragmentation and keeping performance. However, an additional sorted Doubly Linked list is required to store allocated and free elements in order to be able to do coalescence operations in O(1).
This implementation is the most common and most used in real systems because it offers high flexibility while keeping performance very high.

# Benchmarks
Now its time to make sure that all the effort in designing and implementing custom memory allocators is worth. 
I've made several benchmarks with different block sizes, number of operations, random order, etc. The time benchmark measures the time execution that takes initializing the allocator 'Init()' (malloc big chunk, setup additional data structures...) and untill the last operation (allocation or free) is performed.

Here I'm only showing what I believe is relevant for the goal of this project.

## Time complexity
* **Malloc** is without doubt the **worst allocator**.Due to its general and flexible use. _**O(n)**_
* **Free list allocator** is **A much better choice than malloc** as a general purpose allocator.It uses Linked List to speed up allocations/free. It's about three times better than malloc _**O(n)**_

The next allocator are even better BUT they are no longer general purpose allocators. They **impose restrictions** in how we can use them:
* **Pool allocator** forces us to always allocate the same size but then we can allocate and deallocate in any order. The complexity of this one is slightly better than the free list allocator, wait what? The complexity of the pool allocator was supposed to be constant not linear! And that's true. What its happening here is that the initialization of the additional data structure (the linked list) is _**O(n)**_. It has to create all memory chunks in then linked them in the linked list. This operation is hiding the truly complexity of the allocation and free operations that is _**O(1)**_.  So, take into account to initialize the Pool allocator (and all the allocators in general) before to avoid this kind of behaviors.
* **Stack allocator** can allocate any size, but deallocations must be done in a LIFO fashion with a _**O(1)**_ complexity. In the chart the complexity is not completely constant due to init function that has to allocate the first big chunk of memory, similarly as before in the pool allocator.
* **Linear allocator** is the simplest and the best performant allocator with a _**O(1)**_ complexity but its also the most restrictive because single free operations are not allowed. As with the stack, the complexity doesn't look completely constant due to the init function.

![Time complexity of different allocators](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/operations_over_time.png)

In the next chart we can see that if we don't include the Init() function in the benchmark, the overall execution time is reduced and as a consequence we can effectively see that the Linear, Stack and Pool allocators are constant while malloc and free list are clearly linear. The free list implementation using black tree can reduce the complexity to _**O(log n)**_ and therefore its position in the chart would be between the pool allocator and the free list.

![Time complexity of different allocators](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/operations_over_time_no_init.png)

_Note: The time complexity (in general) scales following a linear fashion regarding the size of the allocation request.

## Space complexity
As we can see, even that the space complexity for each allocator is slightly different(due to constants), in the end, all of them have the same space complexity **O(N)**. It is very clear, then, why when denoting big O, constants can be ignored: because its weight in the overall equation is very low when N grows.

![Space complexity of different allocators](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/operations_over_space.png)

# Summary
This is a brief summary describing when you should use each allocator. From more restrictive and efficient allocators to less efficient and general.

* **Linear allocator**. If your data does not follows any specific structure. However, there's a common behavior in time: all data "expires" after a certain time and then is no longer useful and thus can be freed. Think about games for example, you can allocate data in one frame using a this allocator and free all data at the start of the next frame.  
* **Stack allocator**. The same as the Linear allocator but think if it useful to free elements in a LIFO fashion.
* **Pool allocator**. Your data has definitely a structure. All elements of your data have the same size. This is your choice, fast and no fragmentation.
* **Buddy allocator** (_Not implemented here_). Your data is organized in exponential sizes power-of-two (1,2,4,8,16,32...). This allocator performs extremely well when data is structure in that way, being fast and wasting little space.
* **Free list allocator**. No structure or common behavior. This allocator allows you to allocate and free memory as you wish. This is a general purpose allocator that works much better than malloc, but is not as good as the previous allocators, given its flexibility to work in all situations.

# Last thoughts
* Avoid dynamic memory as much as possible. Its behavior is unexpected and a source of problems
* If you are worried about performance and your application uses dynamic memory, think about using a custom allocator instead of malloc
* Try to understand your data and its behavior to choose the right allocator for you. Specific allocators (that impose restrictions on how we can structure/use our data) are far more better than the generic ones. We saw a huge gap between the specific purpose allocator: **Linear, Stack and Pool** and the general purpose: **Free list and Malloc**
* Always choose a less restrictive (or more general) allocator if unsure. If you later see that your data is structured you can always change to use a more restrictive one.

# Future work
* Implement every memory allocator assuming that the alignment is always 8 bytes and thus everything is always align (we no longer need headers).
* Implement a Free list allocator using Red Black Trees to improve performance from O(N) to O(log N)
* Implement a Buddy allocator
* Implement a Slab allocator
* Benchmark internal fragmentation
* Benchmark spatial location (cache misses)

# Acknowledgments

Thanks to [Vanessa](https://github.com/vipyne) and [Krish](https://github.com/sigmasleep) for helping me in the different stages of this project.
