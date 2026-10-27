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
&nbsp;[Fixes in this revision](https://github.com/mtrebi/memory-allocators#fixes-in-this-revision)  <br/> 
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Verification](https://github.com/mtrebi/memory-allocators#verification)  <br/> 
&nbsp;[Future work](https://github.com/mtrebi/memory-allocators#future-work)  <br/> 
&nbsp;[Acknowledgments](https://github.com/mtrebi/memory-allocators#acknowledgments)  <br/> 

# Introduction
When applications need more memory this can be allocated in the heap (rather than in the stack) in _runtime_. This memory is called 'dynamic memory' because it can't be known at compile time and its need changes during the execution. Our programs can ask for dynamic memory usin 'malloc'. Malloc returns an address to a position in memory where we can store our data. Once we're done with that data, we can call 'free' to free the memory and let others processes use it.

For this project I've implemented different ways to manage by ourselves dynamic memory in C++.This means that instead of using native calls like 'malloc' or 'free' we're going to use a custom memory allocator that will do this for us but in a more efficient way.
The goal, then, is to understand how the most common allocators work, what they offer and compare them to see which one performs better.

# Build instructions

```bash
git clone https://github.com/mtrebi/memory-allocators.git
cmake -S memory-allocators -B build
cmake --build build
```

Run the benchmark:

```bash
./build/main
```

Run the regression suite (43 cases, ~20 s):

```bash
ctest --test-dir build --output-on-failure
```

Without CMake, the same suite builds with plain `g++`:

```bash
bash tests/run_all.sh quick   # 定向探针 + 小规模对拍
bash tests/run_all.sh full    # 追加百万级压力（约 1~2 分钟）
```

Two build switches control diagnostics:

| Macro | Effect |
|---|---|
| `ALLOCATOR_TRACE` | Prints every `Allocate` / `Free` / merge to `stdout`. Off by default; independent of `NDEBUG` so that enabling assertions does not flood the log. |
| `ALLOCATOR_VALIDATE` | Adds `FreeListAllocator::Validate()`, `FreeBlockCount()`, `LargestFreeBlock()`. Used by `tests/stress_validate.cpp`, `tests/probe_extra.cpp` and `tests/probe_residual.cpp` to check list ordering, interval overlap, coalescence completeness, header `magic` liveness and `m_used` consistency after every operation. |

CI (`.github/workflows/ci.yml`) builds and tests on **gcc, clang, Apple clang and MSVC**, plus a dedicated **ASAN + UBSAN** job. The local development toolchain (MinGW g++ 8.1) has no `libasan`, so those two rows are the only way the "unverified toolchain" and "no automatic bounds checking" gaps get closed.

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
The allocator only needs a pointer (or offset) to keep track of the current top of the stack. Rollback points are stored outside the managed block, so allocations do not need a per-allocation header in the returned memory. This avoids wasting padding space and keeps the user-facing memory contiguous.

![Data structure of a Stack Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/stack1.png)

_Complexity: **O(N*H) --> O(N)**_ where H is the Header size and N is the number of allocations

### Allocate
Simply move the pointer (or offset) forward and place a header right before the memory block indicating its size.

![Allocating memory in a Stack Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/stack2.png)

_Complexity: **O(1)**_

### Free
Free is a LIFO rollback. The allocator restores the last saved offset and does not need to inspect any header stored in the allocation itself.

![Freeing memory in a Stack Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/stack3.png)

_Complexity: **O(1)**_

## Pool allocator
A Pool allocator is quite different from the previous ones. It splits the big memory chunk in smaller chunks of the same size and keeps track of which of them are free. When an allocation is requested it returns the free chunk size. When a freed is done, it just stores it to be used in the next allocation. This way, allocations work super fast and the fragmentation is still very low.

![Splitting scheme in a Pool Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/pool1.png)

### Data structure
To keep track of the free blocks of memory, the Pool allocator uses a Linked List that links the address of each free memory block. 

![Linked List used in a Pool Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/pool2.png)

To reduce the space needed, this **Linked List is stored in the same free blocks** (smart right?). However, this set the constraint that the data chunks must be at least as big as our nodes in the Linked List (so that, we can store the Linked List in the free memory blocks).

> **Correction (see `docs/修复计划书.md`, D-30).** An earlier revision of this document claimed that `Init()` *"has to create all memory chunks and then link them in the linked list"*. That is not what the code does. `Init()` only performs one `malloc` and resets a cursor; the free list is **built lazily** — a chunk enters the list only when it is `Free`d. Fresh chunks are handed out by advancing an offset (`m_start_ptr + m_offset * m_chunkSize`). The consequence matters for benchmarks: `Init()` for the pool is *O(1)*, not *O(n)*, so the pool's start-up cost is one large `malloc`, nothing more.

![In memory Linked List used in a Pool Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/pool3.png)

_Complexity: **O(1)**_ 

### Allocate
An allocation takes (pops) the first free block of the Linked List. If the list is empty, the next untouched chunk is taken by advancing `m_offset`; when the pool is exhausted the call returns `nullptr`.

![Allocation in a Pool Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/pool4.png)

The linked list doesn't have to be sorted. Its order its determined by the how the allocations and free are done.

![Random State of a Linked List in a Pool Allocator](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/pool5.png)

_Complexity: **O(1)**_

### Chunk size and alignment
`chunkSize` is rounded **up** to a multiple of `alignof(std::max_align_t)` (16 on x86-64). Without that rounding, chunk *k* sits at `start + k * chunkSize`; any `chunkSize` that is not a multiple of 16 would leave every other chunk on an 8-byte boundary, breaking the alignment guarantee for `double` / `long double`. Use `PoolAllocator::ChunkSize()` to read the effective value and `ChunkCount()` for the number of chunks actually available (the tail shorter than one chunk is not used).

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

> **Not implemented in this repository.** `DoublyLinkedList` exists in `includes/` but no translation unit references it; it is a leftover of this design sketch, now covered by a regression case (`tests/probe_round6.cpp`, `DoublyLinkedList`) that pins its insert/remove semantics. The only free-list implementation here is the sorted singly linked list described above, with both `FIND_FIRST` and `FIND_BEST` placement policies. A red-black tree free list remains in *Future work*.

# Benchmarks
Now its time to make sure that all the effort in designing and implementing custom memory allocators is worth. 
I've made several benchmarks with different block sizes, number of operations, random order, etc. The time benchmark measures the time execution that takes initializing the allocator 'Init()' (malloc big chunk, setup additional data structures...) and until the last operation (allocation or free) is performed.

> **Methodology note (see `docs/修复计划书.md`, D-12 / D-13 / D-27).** The original benchmark used `std::chrono::milliseconds` together with a hard-coded `OPERATIONS == 10`. Ten allocations take a few microseconds, so `duration_cast<milliseconds>` rounded every measurement down to `0 ms` and the reported throughput was `inf`. **Every number previously published in this section was therefore not a measurement.** The harness now times in **nanoseconds**, sizes its pointer array from the constructor argument, guards the division, and reports `ns/op` and `ops/s`. All figures below were produced by the fixed harness.
>
> A second documentation claim was also unsupported: the text promised a chart with `Init()` excluded (`operations_over_time_no_init.png`), but the code had no switch to exclude it. **`Init()` is always included in the timings below.** What `Init()` costs differs per allocator and is stated explicitly for each.

## Measurement conditions

| Item | Value |
|---|---|
| Compiler | g++ 8.1.0 (MinGW-w64), `-std=c++11 -O2 -DNDEBUG` |
| CPU | x86-64, single thread, no pinning |
| Operations per data point | 1000 |
| Allocation sizes | 32, 64, 256, 512, 1024, 2048, 4096 bytes |
| Alignment | 8 bytes |
| Reported figure | median of 3 runs |

## Measured throughput (ops/s)

`alloc` = `MultipleAllocation` at the given size; `free` = `MultipleFree` at the same size; `rand` = random size from the list above.

| Allocator | alloc 32 B | alloc 4096 B | free 32 B | free 4096 B | rand alloc | rand free |
|---|---:|---:|---:|---:|---:|---:|
| malloc (`CAllocator`) | 1.3e7 | 7.8e5 | 1.5e7 | 6.5e5 | 2.1e6 | 3.2e6 |
| Linear | 5.9e7 | 5.4e7 | n/a | n/a | 3.9e7 | n/a |
| Stack | 4.8e7 | 5.6e7 | 5.1e7 | 5.0e7 | 3.7e7 | 3.8e7 |
| Pool (4096 B chunks) | 1.2e8 | 1.2e8 | 9.3e5\* | 9.3e5\* | n/a | n/a |
| Free list (`FIND_FIRST`) | 3.9e7 | 1.9e6 | 4.9e6 | 2.0e6 | 3.7e6 | 4.8e6 |

\* The pool's `free` row looks slow only because `SingleFree` re-runs `Init()` (a fresh 16 MB `malloc`) inside the timed region — the 1000 frees themselves are *O(1)*. This is exactly the artefact the note above warns about.

**On comparing `Used()` across rows.** All allocators now report the same thing: bytes actually taken from the system or from the pool, including every allocator's own metadata. `CAllocator` therefore pays for a 16-byte alignment header of its own — that is why its 32 B throughput is slightly below a bare `malloc`. Reporting user bytes instead would make the malloc baseline look better than it is.

## Time complexity
* **Malloc** is the slowest general-purpose option here. Its per-size cost grows with the request: 1.3e7 ops/s at 32 B versus 7.8e5 ops/s at 4096 B, a 17x spread caused by the allocator walking its bins.
* **Free list allocator** is **faster than malloc across the whole size range** — about 3.0x at 32 B (3.9e7 vs 1.3e7) and about 2.4x at 4096 B (1.9e6 vs 7.8e5). The original claim of *"about three times better"* was never backed by a working measurement, but for small blocks it turns out to be roughly right.

**A caveat on `FIND_BEST`.** `FIND_FIRST` stops at the first block that fits; `FIND_BEST` must scan the entire free list on every allocation, and coalescing can leave that list long. Measured on a 1 MB pool fragmented into ~1000 *genuinely separate* free nodes (see `tests/measure_findbest.cpp`), an `Allocate`+`Free` pair of 100 B costs **26 ns under `FIND_FIRST` and 4605 ns under `FIND_BEST` — about 180x**. An earlier version of this document claimed 1290x; that number was wrong. The "fragmentation" step back then allocated a thousand blocks and then freed them *in order*, which lets adjacent free blocks merge immediately — the list ended up holding one large block, so the two policies looked almost identical in cost until a broken measurement exposed it. The lesson generalises: a workload that looks fragmented may not be, and every number in this document is now reproducible by a checked-in tool.

Two cheap optimisations now sit in front of the scan (`D-51`): an O(1) fast reject when even the largest free block cannot hold the request (previously every doomed allocation scanned the whole list), and an early exit on an exact size match. Neither changes the worst case — exact best-fit without an index is O(free nodes) by construction, which is why the red-black tree below is still future work.

The next allocator are even better BUT they are no longer general purpose allocators. They **impose restrictions** in how we can use them:
* **Pool allocator** forces us to always allocate the same size but then we can allocate and deallocate in any order. Both `Allocate` and `Free` are *O(1)*, and at 1.2e8 ops/s it is the fastest allocator in this set. The apparent linearity in the chart is **not** in the allocation path: it comes from `Init()` doing one large `malloc` and from the fixed 4096-byte chunk dominating large requests.
* **Stack allocator** can allocate any size, but deallocations must be done in a LIFO fashion with a _**O(1)**_ complexity. In the chart the complexity is not completely constant due to init function that has to allocate the first big chunk of memory, similarly as before in the pool allocator.
* **Linear allocator** is the simplest and the best performant allocator with a _**O(1)**_ complexity but its also the most restrictive because single free operations are not allowed. As with the stack, the complexity doesn't look completely constant due to the init function.

![Time complexity of different allocators](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/operations_over_time.png)

In the next chart we can see that if we don't include the Init() function in the benchmark, the overall execution time is reduced and as a consequence we can effectively see that the Linear, Stack and Pool allocators are constant while malloc and free list are clearly linear. The free list implementation using black tree can reduce the complexity to _**O(log n)**_ and therefore its position in the chart would be between the pool allocator and the free list.

![Time complexity of different allocators](https://github.com/mtrebi/memory-allocators/blob/master/docs/images/operations_over_time_no_init.png)

> **This chart cannot be reproduced with the code in this repository.** As noted above, the harness has no switch to exclude `Init()`; the image was produced with a modified local build that was never committed. It is kept here because the *shape* it illustrates (allocation-only cost) is still a useful qualitative statement, but treat the image as illustrative rather than as a measurement of this code. If you need the same comparison, subtract the per-allocator `Init()` cost measured on your machine, or time `Init()` separately.

_Note: The time complexity (in general) scales following a linear fashion regarding the size of the allocation request.

## Thread safety
**None of these allocators are thread-safe.** They keep no locks and mutate shared internal state (`m_offset`, `m_used`, the free lists) without synchronisation. Concurrent `Allocate` / `Free` calls on the same instance are a data race, even for the `malloc`-backed `CAllocator` wrapper, whose `m_used` / `m_peak` counters are plain `std::size_t`. Give each thread its own allocator instance, or add external synchronisation.

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

# Fixes in this revision

This tree is the upstream project **plus a correctness overhaul**. The full analysis — defect inventory, evidence, patch rationale, acceptance criteria — is in [`docs/修复计划书.md`](docs/修复计划书.md) (Chinese). Highlights:

**P0 — the allocators did not work**

| ID | Problem | Consequence |
|---|---|---|
| D-01 | `FindBest` never updated `smallestDiff`, and recorded the wrong predecessor | `FIND_BEST` looped forever on the second allocation, then returned overlapping addresses |
| D-02 | `PoolAllocator::Reset()` did not clear the free list | the same chunk was handed out twice |
| D-03 | `SinglyLinkedList` left `head` uninitialised | first `insert` branched on a garbage pointer |
| D-04 | `Free` only inserted when it found a larger address; OOM relied on `assert` | freed blocks were silently lost; `NDEBUG` builds crashed on OOM |
| D-24 | `Free` placed the node at the *header* address and added `padding` into `blockSize` a second time | every free overran its block by `2 × alignmentPadding` and corrupted the neighbouring block header |

**P1 — silently wrong results**

Alignment decided from a relative offset instead of the absolute address (D-05); a full `alignment` of padding inserted even when already aligned (D-06); `padding` stored in a `char` and truncated above 255 (D-07); the `padding` out-parameter of `Find` overwritten by the last visited node (D-08); `CAllocator` ignoring `alignment` (D-09); the `assert("msg" && cond)` idiom, which is always true (D-10); `StackAllocator::Free` ignoring its argument (D-11); `alignment = 0` dividing by zero (D-25); leftover space in `(0, sizeof(Node))` written past the pool end (D-26); capacity checks that wrap around on `size + padding` (D-32); pool chunks violating `max_align_t` (D-33).

**P2 — the published numbers were meaningless**

Millisecond timing with 10 operations produced `0 ms` and `inf ops/s` everywhere (D-12, D-13); `void* addresses[10]` overflowed for any larger operation count (D-14); `Init()` did not reset `m_used` / `m_peak` so peaks accumulated across rounds (D-16); repeated `Init()` leaked the previous block (D-17); `main` never deleted the free-list allocator (D-18); `Free` accepted foreign and duplicate pointers, underflowing `m_used` (D-37); copy construction was declared private but never `= delete`, so `a = b` shared one buffer and double-freed (D-34).

**P3 — engineering hygiene**

Missing include guards (D-20), non-self-contained headers (D-38), `_DEBUG` — which only MSVC defines, so all trace code was dead on GCC/Clang (D-35), no tests and no warnings in the build (D-23), no way to ask an allocator whether it supports `Free` (D-31), and a README that described an implementation it did not have (D-30).

Two defects were found **while implementing the fixes**, not in the original code, and are recorded as D-40 (`LinearAllocator::m_offset` never initialised) and D-41 (the data-block header was written before the node was unlinked, overwriting the node's own `next` pointer). A third, D-39, was a flaw in the *first version of the patch*: discarding leftover space smaller than a node created permanent holes that made coalescence impossible — a 4 MB pool ended up with 16618 free blocks instead of 1. The current code folds the remainder into the block size instead.

**A fourth round** re-checked the *fixed* code specifically under `NDEBUG`, on the theory that anything guarded only by `assert` had been fixed for debug builds and left broken for release builds. That theory was correct:

| ID | Problem | Evidence |
|---|---|---|
| **D-42** | `StackAllocator::Free` validated the pointer only inside `assert`. With `NDEBUG` the check vanished, so freeing a non-top block rewound the stack over a still-live allocation and the next `Allocate` handed out its address again. | `probe_residual stack_order`: `Allocate(64)` returned the address of a block that was still live, and its contents were then fair game. Now `FreeChecked()` validates unconditionally and returns whether it succeeded; `Free()` calls it and leaves state untouched on rejection; `RejectedFrees()` counts misuse. `Pop(marker)` also shrinks the marker array, which it previously did not — stale markers made later `Free` calls compare against an out-of-date bound. |
| **D-43** | `PoolAllocator::Allocate` checked `allocationSize == m_chunkSize` only in `assert`. Asking for 16 bytes from a 64-byte-chunk pool silently returned a 64-byte block. | `probe_residual pool_strict`. Now returns `nullptr`: the pool's contract is "exactly one chunk per call", and quietly handing out four times the request is worse than failing. |
| **D-44** | The `size + padding` overflow fixed in `Allocate` (D-32) was still present inside `FindFirst` / `FindBest`, where the same addition decided whether a block was big enough. | `probe_residual fl_overflow`. Both now compare with subtraction (`p <= blockSize && size <= blockSize - p`). |

Two things that looked like defects but are not, and were therefore **relaxed** rather than "fixed": `FreeListAllocator` no longer requires `alignment` to be a power of two (it computes padding with modulo and satisfies 24, 48, 96 correctly — only `_aligned_malloc` / `posix_memalign` need powers of two, which is `CAllocator`'s constraint), and it no longer rejects `size < sizeof(Node)` (the 16-byte header already guarantees the block can hold a free-list node).

`Used()` / `Peak()` were also made to mean one thing everywhere: **bytes actually taken from the system or from the pool, metadata included**. `CAllocator` had been counting only the user's bytes, which made the malloc baseline look cheaper than it is. It now reports the size it really asked `malloc` for.

## Verification

| Layer | What it covers | Result |
|---|---|---|
| 30 targeted probes (`tests/probe_*.cpp`) | one minimal case per defect, separate process each | all pass |
| Randomised differential fuzzing (`tests/fuzz_*.cpp`) | overlap, alignment, data overwrite against a reference model | 0 violations over 400k ops × 6 runs |
| Invariant checking (`tests/stress_validate.cpp`, `ALLOCATOR_VALIDATE`) | list address order, interval disjointness, adjacent free blocks merged, header `magic` still valid on every live pointer, `m_used` identity — after **every** operation | 0 violations over 1M ops × 2 policies |
| Coalescence completeness | after releasing everything, the free list must be exactly one node of pool size | holds (1 block / 4096 KB) |
| Release-mode contract checks (`tests/probe_residual.cpp`) | out-of-order / duplicate `Free`, size mismatch, non-power-of-two alignment, capacity overflow — all with `NDEBUG` set | all pass |
| Misuse rejection (`tests/probe_residual.cpp`) | double `Free` on the pool and the free list, foreign pointers, zero-byte requests — must be **refused and counted**, never silently accepted | all pass; 0 crashes |
| Fast-reject soundness (`tests/measure_findbest.cpp`) | the same 4000-op request sequence replayed on one allocator with `D-51` fast-reject on and off — outcomes must match request for request | 2667 allocations, 0 mismatches |
| Capacity conservation (`tests/probe_round6.cpp`) | 200 rounds of fill-the-pool-then-drain on the pool allocator; every round must hand out all 32 chunks again | holds |
| Build parity (`tests/check_build_parity.py`) | the case list in `CMakeLists.txt` and the one `run_all.sh` actually executes must be identical, plus a structural check of the CMake file | 39 = 39 cases |

The last row is the one that took five rounds to get right. Duplicate `Free` used to be inferred rather than detected: the pool relied on `m_used < m_chunkSize` (only true when a single chunk is live), and the free list had no check at all — its second `Free` read the free node's data as a padding value, computed an address outside the pool and wrote there, which is an immediate `SIGSEGV`. Both now use real per-allocation state: a `magic` word in the free list's 16-byte header (layout unchanged) and a one-bit-per-chunk bitmap in the pool. The magic must be cleared explicitly on release — the free node does not always overwrite it.

`CAllocator::Free` keeps **no** validation in the benchmark build — it is the raw-`malloc` control group, and instrumenting it would defeat the comparison. Under `-DALLOCATOR_VALIDATE` (which is what the regression suite and CI use) it does check: every allocation is registered, and a `Free` of an unknown or already-freed pointer is refused and counted via `RejectedFrees()`. The registry lives outside the pool, so the header layout — and therefore `Used()` — is byte-for-byte identical in both builds. An earlier attempt put a magic word in the header instead; that grew it from 16 to 24 bytes and made `Used()` depend on a debug macro, which broke the "all allocators measure the same thing" property. The registry is the version that kept both goals.

Still open, with the reason rather than a promise:

| Gap | Why it is still open |
|---|---|
| ASAN / UBSAN / Valgrind locally | MinGW g++ 8.1 ships no `libasan`, and there is no Valgrind on this platform. Covered in CI instead. |
| MSVC / Clang / macOS | Same — CI matrix only; never built locally. |
| Concurrency | All allocators are lock-free by design and documented as non-thread-safe. Making them safe is a different project. |
| `FIND_BEST` on long free lists | ~180x slower than `FIND_FIRST` at ~1000 genuinely separate free nodes (`tests/measure_findbest.cpp`; the earlier 1290x claim came from a fragmentation step that immediately merged itself away). Fixing it properly means the red-black tree, which is still *Future work*. |
| Small-block overhead in the free list | A 16-byte allocation occupies 32 bytes (16-byte header + padding). Structural, not a bug; a buddy or slab allocator is the answer. |
| `DoublyLinkedList` | Dead code kept as the counterpart to the red-black tree design in this document. It used to be completely untested — which is exactly where two real bugs hid (`head` never initialised, tail-insert never set `previous`, so a later `remove` walked off into garbage). Both are fixed and pinned by `tests/probe_round6.cpp`. |
| CMake / CTest path | `cmake` cannot be installed on the development machine (package mirrors unreachable), so `CMakeLists.txt` and `ctest` have **never been run locally**. Two compensating checks now exist: `tests/check_build_parity.py` proves the case list in CMakeLists matches what `run_all.sh` executes (39 = 39) and validates the file's block structure, and CI runs the real CMake build. |

# Future work
* Implement every memory allocator assuming that the alignment is always 8 bytes and thus everything is always align (we no longer need headers).
* Implement a Free list allocator using Red Black Trees to improve performance from O(N) to O(log N)
* Implement a Buddy allocator
* Implement a Slab allocator
* Benchmark internal fragmentation
* Benchmark spatial location (cache misses)

# Acknowledgments

Thanks to [Vanessa](https://github.com/vipyne) and [Krish](https://github.com/sigmasleep) for helping me in the different stages of this project.
