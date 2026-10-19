# Operating Systems Assignment 8 — xv6 Memory Management

**Submitted by:** Ranveer Gupta  
**Roll Number:** 2401MC20

## About the Project

This repository contains the implementation and analysis of memory management mechanisms in the xv6 operating system. It explores how virtual addresses are translated into physical addresses, how memory usage is monitored, how page tables are organized, and how demand paging can improve memory utilization.

The assignment also covers extended memory management techniques and page replacement policies through kernel-level modifications and user-space test programs.

## Assignment Objectives

- Examine virtual-to-physical address mapping and page permissions.
- Track free physical pages and per-process memory usage.
- Inspect page directories, page tables, and memory mappings.
- Implement lazy allocation for heap memory.
- Investigate additional memory management mechanisms, including copy-on-write, memory protection, and shared memory.
- Compare FIFO, LRU, and Optimal page replacement algorithms.

## Repository Structure

The repository is organized around the implementation, testing, and documentation of the assignment.

| Component | Description |
|---|---|
| Kernel source modifications | Changes to xv6 memory management, system calls, and trap handling |
| `vatest.c` | Tests virtual-to-physical address translation |
| `memtest.c` | Examines physical memory statistics |
| `vmprinttest.c` | Displays the process page table hierarchy |
| `lazytest.c` | Tests demand paging and deferred allocation |
| `cowtest.c` | Tests copy-on-write behavior |
| `protecttest.c` | Tests memory protection |
| `shmtest.c` | Tests shared memory |
| `pagerepl.c` | Evaluates page replacement policies |
| Screenshots and report | Records test results, observations, and implementation decisions |

## Implemented Memory Management Components

### 1. Virtual-to-Physical Address Translation

The `va2pa()` system call examines the calling process's page tables to translate a virtual address into a physical address.

The implementation checks whether the page is present and accessible from user mode. It also reports the page directory index, page table index, offset, and relevant permission bits.

**Testing:** Global, local, and dynamically allocated variables are examined. A `fork()` test compares the virtual and physical addresses of a global variable in the parent and child processes.

### 2. Physical Memory Statistics

Memory allocation tracking is added to xv6's physical page allocator in `kalloc.c`.

The `getmeminfo()` system call reports:

- Total physical pages.
- Currently available physical pages.
- The calling process's virtual address-space size in pages.
- Pages occupied by the process's page directory and page tables.

**Testing:** Memory statistics are collected before and after allocation, process creation, and child termination to investigate physical memory consumption and reclamation.

### 3. Page Table Inspection

The `vmprint()` system call displays the page directory and its valid page table entries.

The output includes entry indices, mapped virtual addresses, physical frame addresses, and permission flags. The dump distinguishes user-space mappings from kernel-space mappings and helps identify the stack guard page.

**Testing:** The page table is inspected before heap expansion, after allocating additional virtual memory, and after shrinking the heap.

### 4. Lazy Heap Allocation

The default heap expansion mechanism is modified to defer physical memory allocation until a page is accessed.

The implementation updates the process's virtual size during `sbrk()` and handles page faults by allocating and mapping physical frames when appropriate. Invalid accesses and allocation failures are handled without unnecessarily crashing the kernel.

Special attention is given to memory shrinking, process termination, `fork()`, and kernel operations that access user buffers.

**Testing:** A large virtual memory region is requested, only selected pages are accessed, and memory statistics are compared to observe the difference between reserved virtual memory and allocated physical memory.

### 5. Extended Memory Management Experiments

Additional test programs examine copy-on-write, memory protection, shared memory, and page replacement.

These experiments investigate how physical pages can be shared, how access permissions affect processes, and how different page replacement policies respond to a sequence of memory references.

### 6. Page Replacement Analysis

The page replacement simulator evaluates three policies:

- **FIFO (First-In, First-Out):** Replaces the page that entered memory earliest.
- **LRU (Least Recently Used):** Replaces the page that has not been accessed for the longest time.
- **Optimal:** Replaces the page whose next use is farthest in the future.

The report compares page-fault counts for the same reference string and frame capacity. Results should be generated from the submitted implementation and presented in a comparison table.

## Testing and Validation

The user-space test programs are intended to validate the behavior of each implemented feature. Testing focuses on:

- Correct address translation and permission checks.
- Consistent physical memory accounting.
- Accurate page table output.
- Deferred allocation of untouched heap pages.
- Correct memory reclamation after process termination.
- Expected behavior during `fork()`, `exit()`, and `wait()`.
- Page-fault comparisons under different replacement policies.

Terminal screenshots and observed results should be included with the submission to document the implementation's behavior.

## Implementation Notes

The kernel modifications involve relevant xv6 source files, including `vm.c`, `kalloc.c`, `trap.c`, `sysproc.c`, `proc.c`, and the system call interface files.

Shared structures and function declarations must be placed in the appropriate headers to maintain consistency between kernel components and user programs. Memory allocation counters must be synchronized properly, and page-fault handling must account for missing mappings and invalid addresses.

## Report Contents

The accompanying report documents:

1. Implementation approach and data structures.
2. Design decisions for each memory management feature.
3. Problems encountered and their resolutions.
4. Memory usage measurements before and after relevant operations.
5. Test output and screenshots.
6. Page-fault comparison and analysis for FIFO, LRU, and Optimal.

## Author

**Ranveer Gupta**  
**Roll Number:** 2401MC20

*Operating Systems — Assignment 8*  
*xv6 Memory Management*
