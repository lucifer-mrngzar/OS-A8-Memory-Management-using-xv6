# Operating Systems Assignment 8 — Memory Management in xv6

**Author:** Ranveer Gupta  
**Roll Number:** 2401MC20  
**Project:** xv6 Memory Management Extensions  
**Architecture:** x86 (32-bit)  
**Page Size:** 4 KB

---

## Overview

This repository contains the implementation and analysis of memory management extensions for the xv6 educational operating system.

The assignment explores how operating systems translate virtual addresses into physical addresses, allocate and reclaim physical memory, organize process page tables, and handle memory accesses through page faults. It extends the existing xv6 kernel with additional system calls, memory statistics, page table inspection, and lazy allocation.

The project combines kernel-level modifications with user-space test programs to examine memory management behavior experimentally.

## Objectives

The primary objectives of this assignment are to:

- Understand two-level paging and virtual-to-physical address translation.
- Expose page mapping information to user programs through system calls.
- Track free physical pages and per-process memory usage.
- Inspect page directories and page tables.
- Implement demand paging through lazy heap allocation.
- Handle page faults and invalid memory accesses safely.
- Evaluate memory allocation, process creation, and memory reclamation.
- Investigate additional memory management mechanisms and page replacement algorithms as required by the assignment.

## Table of Contents

1. [System Architecture](#system-architecture)
2. [Question 1: Virtual-to-Physical Address Translation](#question-1-virtual-to-physical-address-translation)
3. [Question 2: Physical Memory Statistics](#question-2-physical-memory-statistics)
4. [Question 3: Page Table Inspection](#question-3-page-table-inspection)
5. [Question 4: Lazy Memory Allocation](#question-4-lazy-memory-allocation)
6. [Testing and Validation](#testing-and-validation)
7. [Build and Execution](#build-and-execution)
8. [Implementation Challenges](#implementation-challenges)
9. [Repository Contents](#repository-contents)
10. [Author](#author)

---

## System Architecture

The project uses the 32-bit x86 version of xv6, which implements two-level paging with 4 KB pages.

A virtual address is divided into three fields:

| Field | Size | Purpose |
|---|---:|---|
| Page Directory Index (PDX) | 10 bits | Selects a page directory entry |
| Page Table Index (PTX) | 10 bits | Selects a page table entry |
| Page Offset | 12 bits | Identifies the byte within a page |

The translation process uses the page directory and page table to locate the physical frame corresponding to a virtual address.

The kernel also uses page-table flags to control access permissions:

- `PTE_P` — indicates that a mapping is present.
- `PTE_W` — permits writing to the mapped page.
- `PTE_U` — permits access from user mode.

These fields are central to the address translation, page inspection, and lazy allocation implementations.

---

## Question 1: Virtual-to-Physical Address Translation

### Objective

Implement a `va2pa()` system call that translates a virtual address belonging to the calling process into its corresponding physical address.

### Implementation

The system call uses xv6's existing `walkpgdir()` helper to locate the page table entry associated with the supplied virtual address.

The implementation checks whether the page table entry exists, whether the page is present, and whether user-mode access is permitted. Invalid or inaccessible addresses return `-1`.

The kernel additionally reports:

- Page directory index.
- Page table index.
- Page offset.
- Present, writable, and user-accessible flags.

### Test Program: `vatest.c`

The test program examines the mappings of:

- A global variable.
- A local variable.
- Dynamically allocated memory obtained through `malloc()`.
- An invalid or unmapped address, such as `0x7FFFFFFF`.

It also creates a child process using `fork()` and compares the virtual and physical addresses of a global variable.

In standard xv6, `fork()` duplicates the parent's user memory into the child's address space. Consequently, the same virtual address can map to different physical frames in the two processes.

### Expected Observation

Valid user-accessible mappings should produce physical addresses, while unmapped or inaccessible addresses should return `-1`. Parent and child mappings should be examined to verify the expected behavior of the implementation.

---

## Question 2: Physical Memory Statistics

### Objective

Extend xv6's physical memory allocator to expose memory usage statistics to user programs.

### Implementation

The physical page allocator in `kalloc.c` manages memory in 4 KB frames using a free list.

A free-page counter is maintained during allocation and deallocation. Updates are synchronized using `kmem.lock` to prevent inconsistent counts when multiple processes or CPUs access the allocator.

The `getmeminfo()` system call provides the following information:

| Field | Description |
|---|---|
| `total_pages` | Total physical pages represented by the implementation's memory model |
| `free_pages` | Number of currently available physical pages |
| Process pages | Process address-space size expressed in pages |
| Page-table pages | Physical pages occupied by the process's page directory and page tables |

The system call copies the statistics into a user-provided `struct meminfo`.

### Test Program: `memtest.c`

The test program collects statistics at different stages:

1. Before additional memory allocation.
2. After increasing the process's memory using `sbrk()` or `malloc()`.
3. After creating a child process.
4. After the child exits and is reaped using `wait()`.

### Expected Observation

Physical page availability should generally decrease when additional physical frames or page-table pages are allocated. When memory is freed, the available page count should increase.

The test also helps detect potential memory leaks by comparing the free-page count before and after a process completes. Measurements must account for other allocations, including process structures and page tables.

---

## Question 3: Page Table Inspection

### Objective

Implement a `vmprint()` system call that displays the page directory and its valid page table entries.

### Implementation

The kernel traverses the calling process's page directory and prints the valid entries. For each valid page directory entry, it displays the associated page table's physical address and flags.

The second level of the output displays individual page table entries, including:

- Page table entry index.
- Virtual address represented by the entry.
- Physical frame address.
- Present, writable, and user-accessible flags.

The output uses indentation to distinguish page directory entries from their corresponding page table entries. Kernel mappings are handled separately from user-space mappings.

### Test Program: `vmprinttest.c`

The program invokes `vmprint()` at three stages:

1. At program startup.
2. After requesting eight additional pages using `sbrk(8 * 4096)`.
3. After shrinking the heap by one page using `sbrk(-4096)`.

### Stack Guard Page

The stack guard page is a mapped page whose user-access permission is disabled. It helps prevent the user process from accessing memory below the intended stack region.

The page table dump can be used to identify this mapping and distinguish mapped pages from user-accessible pages. The process's virtual size and the number of accessible pages must be interpreted carefully because the address space may contain unmapped lazy pages.

### Expected Observation

The page table hierarchy should reflect changes in the process's address space. New mappings may appear after memory is accessed, while mappings should disappear when allocated pages are properly released.

---

## Question 4: Lazy Memory Allocation

### Objective

Modify xv6 so that increasing the process's heap size does not immediately allocate physical memory for every requested page.

This technique is known as **lazy allocation** or **demand paging**.

### Default Allocation Behavior

In the default xv6 implementation, increasing the heap size through `sbrk()` typically allocates and maps physical pages immediately.

For large memory requests, this may consume physical memory even when the process never accesses most of the requested region.

### Modified Allocation Behavior

The modified implementation defers physical page allocation until a valid access triggers a page fault.

The process's virtual size is increased without eagerly allocating all requested physical frames. When a suitable page fault occurs, the kernel:

1. Retrieves the faulting virtual address using `rcr2()`.
2. Checks whether the address belongs to a valid region of the process's address space.
3. Allocates a physical page using `kalloc()`.
4. Clears the allocated page to provide zero-initialized memory.
5. Maps the frame into the process's page table using `mappages()`.
6. Resumes execution when the fault has been resolved.

Invalid accesses, addresses outside the process's valid range, and allocation failures must be handled without bringing down the kernel.

### Important Edge Cases

The implementation must also consider:

- Heap shrinking and reclamation of allocated pages.
- Allocation failure when physical memory is exhausted.
- Invalid addresses beyond the process's permitted memory range.
- Accesses to untouched lazy pages through kernel system calls such as `read()` and `write()`.
- `fork()` handling page table entries that are not present.
- Correct cleanup when a process exits or is terminated after a fault.

These cases are important because user programs can access memory indirectly through system calls, not only through ordinary load and store instructions.

### Test Program: `lazytest.c`

The test program requests a large virtual region, such as 32 MB, and touches only a few widely separated pages.

Memory statistics are collected before and after the request and after selected pages are accessed. Additional tests examine `fork()`, process termination, and system calls involving user buffers.

### Expected Observation

A large memory request should increase the process's virtual size without immediately consuming an equivalent amount of physical memory. Accessing previously untouched pages should trigger allocation as needed.

The implementation should also demonstrate safe handling of invalid addresses and physical memory exhaustion.

---

## Testing and Validation

The xv6 `usertests` suite was executed to check compatibility with existing operating system functionality.

The observed output included successful results for tests such as:

- File creation and deletion.
- File reading and writing.
- Process creation using `fork()`.
- Process replacement using `exec()`.
- Pipes and process synchronization.
- Heap allocation through `sbrk()`.

The test run ultimately printed `ALL TESTS PASSED`. However, the output also contained segmentation-fault messages during the `sbrk` test and a lazy-allocation out-of-memory message during the memory test.

These messages require investigation before all memory-management edge cases can be considered validated.

Dedicated assignment programs should also be executed independently to verify address translation, memory accounting, page table output, and lazy allocation behavior.

### Test Programs

| Program | Purpose |
|---|---|
| `vatest.c` | Verify virtual-to-physical address translation |
| `memtest.c` | Examine physical memory statistics |
| `vmprinttest.c` | Inspect page directory and page table entries |
| `lazytest.c` | Evaluate demand paging and deferred allocation |

Additional assignment tests may cover copy-on-write, memory protection, shared memory, and page replacement.

---

## Build and Execution

The following commands illustrate a typical workflow for a compatible xv6 x86 source tree.

### 1. Obtain the Source Code

Clone the repository containing the xv6 implementation:

```bash
git clone <repository-url>
cd <repository-directory>
```

Replace the placeholders with the actual repository URL and directory name.

### 2. Build xv6

Use the build command supported by the supplied xv6 source tree and installed toolchain. A typical configuration may use:

```bash
make clean
make
```

### 3. Launch xv6

For an xv6 version configured to use QEMU on x86:

```bash
make qemu-nox
```

The exact target depends on the Makefile and available QEMU configuration.

### 4. Run the Test Programs

After adding the test programs to the Makefile's user-program list and rebuilding xv6, execute the corresponding programs from the xv6 shell.

For example:

```text
$ vatest
$ memtest
$ vmprinttest
$ lazytest
$ usertests
```

The executable names depend on how they are registered in the Makefile.

---

## Implementation Challenges

### 1. Kernel and User-Space Interfaces

New system calls require consistent declarations, syscall numbers, dispatch-table entries, and user-space stubs. Shared structures such as `struct meminfo` must be defined consistently.

### 2. Synchronization in the Physical Allocator

The free-page counter must remain consistent with the allocator's operations. Appropriate locking is necessary when updating shared allocator state.

### 3. Page Table Permissions

Address translation must distinguish between present mappings and pages accessible from user mode. A mapped page is not necessarily a valid user-accessible page.

### 4. Lazy Allocation and Page Faults

The page-fault handler must distinguish valid demand-paging faults from invalid memory accesses. It must also handle allocation failures and avoid duplicate or conflicting page mappings.

### 5. Memory Reclamation

Memory accounting must consider user frames, page-table pages, process cleanup, and memory freed during shrinking or termination.

---

## Repository Contents

The repository is intended to include:

- Modified xv6 kernel source files.
- System call declarations and implementations.
- User-space test programs.
- Terminal screenshots demonstrating test execution.
- A report describing design decisions, implementation challenges, and observed results.

Additional files should be included as their corresponding assignment components are implemented and tested.

## Conclusion

This assignment provides practical experience with the internal mechanisms of memory management in xv6. By implementing address translation, memory statistics, page table inspection, and lazy allocation, the project explores the relationship between virtual address spaces, physical frames, page permissions, and page faults.

The testing process also highlights the importance of validating edge cases, memory reclamation, and compatibility with existing operating system tests.

## Author

**Ranveer Gupta**  
**Roll Number:** 2401MC20

*Operating Systems — Assignment 8*  
*xv6 Memory Management*
