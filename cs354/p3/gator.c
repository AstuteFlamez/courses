/*======================================================================
 * PROJECT: Alloc Gator
 *----------------------------------------------------------------------
 * AUTHOR: BILAL USMAN
 * EMAIL: busman4@wisc.edu
 * ADDITIONAL SOURCES: NONE
 * provided by the course staff) that were used to complete this code here>
 * FILE: gator.c
 * COURSE: COMP SCI 354 - Fall 2026
 * INSTRUCTOR: Dahl
 * COPYRIGHT: 2026, Dahl
 * Posting or sharing this file with anyone outside of course staff prohibited.
 */

#include <stdio.h>
#include <sys/mman.h>
#include "gator.h"

void* heap_start = NULL; // beginning of heap prologue block
void* heap_brk = NULL;   // first invalid address beyond the end of heap
BlockHeader* first_block = NULL;

/*
 * Allocates a page (4096 bytes) of memory, and initialized to be managed
 * and used for dynamic allocations via ag_alloc() and ag_free().
 *
 * This function can be called explicitly, or if not will automatically
 * be called by the first ag_alloc() call.
 */
void ag_init() {
  if(heap_start != NULL) {
    fprintf(stderr,"ERROR: can only allocate Alloc Gator heap once.\n");
    exit(1);
  }

  // allocate space for heap memory:
  // - using mmap rather than sbrk to avoid standard library confounds
  //   for example, printf calls may call malloc and change the heap
  // - not calling munmap, memory to be freed when program ends
  heap_start = mmap(NULL,PAGE_SIZE,
		    PROT_READ|PROT_WRITE,
		    MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
  heap_brk = heap_start + PAGE_SIZE;

  // initialize the initial padding and prologue block
  // initialize the header and footer for the single large unallocated block
  // initialize the epilogue block header
  *(int *)heap_start = 354;
  ((int *)heap_start)[1] = 8 | 1;
  ((int *)heap_start)[2] = 354;
  first_block = (BlockHeader *)((char *)heap_start + 12);
  *first_block = 4080 | 2;
  *(BlockHeader *)((char *)heap_brk - 8) = 4080;
  *(BlockHeader *)((char *)heap_brk - 4) = 1;
}

/*
 * Loops through and displays information about each of the dynamic memory
 * blocks being managed by this code.  This is primarily helpful for debugging.
 */
void ag_visualize() {
  int counter = 0;
  uint32_t size = 0;
  void* start = NULL;
  void* end = NULL;
  int a = 0;
  int p = 0;
	
  printf("Block#: A P Size Start          End\n");
  printf("==============================================\n");
  for(BlockHeader* block=first_block;
      *block!=1  && *block!=3 && ((void*)block < heap_brk);
      block+=size/sizeof(BlockHeader)) {
    counter++;
    size = *block & ~7;
    start = block;
    end = start + size;
    int footer_size = *((int*)end-1);    
    a = *block & 1;
    p = (*block & 2) >> 1;
    printf("%6d: %1d %1d %4d %p %p\n",counter,a,p,size,start,end);
    if(a == 0 && size != footer_size)
      printf("              * Different footer size: %d\n",footer_size);
  }
}

/*
 * This function can be used to dynamically retrieve a block of contiguous
 * memory that is aligned to a 16 byte boundary.  The specified size refers to
 * the payload size of usable memory, and does not include any related headers
 * or padding.  If no block of the requested size can be allocated, this
 * function instead returns a NULL pointer.
 */
void* ag_alloc(size_t size) {
  if(heap_start == NULL) ag_init();

  // Reject empty requests and requests larger than the heap can hold
  if (size == 0 || size > 4076) {
      return NULL;
  }

  // Include the header and round up to a multiple of 16 bytes
  size_t needed = (size + sizeof(BlockHeader) + 15) & ~(size_t)15;
  BlockHeader *current = first_block;
  BlockHeader *best = NULL;
  size_t best_size = PAGE_SIZE;

  while ((*current & ~3u) != 0) {
    size_t current_size = *current & ~3u;

    // Remember the smallest free block that can satisfy this request
    if ((*current & 1u) == 0 &&
        current_size >= needed &&
        current_size < best_size) {
        best = current;
        best_size = current_size;
    }

    current = (BlockHeader *)((char *)current + current_size);
  }

  // No free block is large enough for this request
  if (best == NULL) {
      return NULL;
  }

  size_t remaining = best_size - needed;
  BlockHeader prev_alloc = *best & 2u;

  // Split if the leftover space can form a block of at least 16 bytes
  if (remaining >= ALIGNMENT) {
    *best = needed | prev_alloc | 1u;
    BlockHeader *new_free = (BlockHeader *)((char *)best + needed);
    // The remainder is free, and its preceding block is allocated
    *new_free = remaining | 2u;

    // A free block's footer stores only its size
    *(BlockHeader *)((char *)new_free + remaining - sizeof(BlockHeader)) = remaining;
  } else {
    // Allocate the entire chosen block
    *best = best_size | prev_alloc | 1u;

    // Tell the following block that its predecessor is allocated
    BlockHeader *next = (BlockHeader *)((char *)best + best_size);
    *next |= 2u;
  }

  // Return the payload address, immediately after the header
  return (char *)best + sizeof(BlockHeader);
}

/*
 * This function is used to free a block of dynamica memory that was previously
 * allocated using ag_alloc().  Note that the ptr argument refers to the
 * payload address that bp_alloc returned, rather than the block header.
 */
int ag_free(void* ptr) {
	
  // Reject pointers outside the managed heap
  if (heap_start == NULL || ptr == NULL ||
      (uintptr_t)ptr < (uintptr_t)heap_start ||
      (uintptr_t)ptr >= (uintptr_t)heap_brk) {
      return 1;
  }

  // Reject addresses before the first payload or without proper alignment
  if ((uintptr_t)ptr < (uintptr_t)first_block + sizeof(BlockHeader) ||
      (uintptr_t)ptr % ALIGNMENT != 0) {
      return 1;
  }

  BlockHeader *header = (BlockHeader *)((char *)ptr - sizeof(BlockHeader));

  // Reject an attempt to free an already-free block
  if ((*header & 1u) == 0) {
      return 2;
  }

  size_t block_size = *header & ~3u;

  // Clear the allocated bit while preserving the size and previous-block bit
  *header &= ~1u;

  // Free blocks need a footer containing their size
  *(BlockHeader *)((char *)header + block_size - sizeof(BlockHeader)) = block_size;

  // Tell the following block that its predecessor is now free
  BlockHeader *next = (BlockHeader *)((char *)header + block_size);
  *next &= ~2u;

  ag_coalesce(header);
	
  return 0;
}

/*
 * This helper function should be called from ag_free() to perform immediate
 * coalescing that combines the provided block with either the previous or
 * next or both, when those neighboring blocks are free.
 */
void ag_coalesce(BlockHeader* header) {

  // Start with the freed block's size and locate its next neighbor
  size_t total_size = *header & ~3u;
  BlockHeader *next = (BlockHeader *)((char *)header + total_size);

  // Include the next block if it is free
  if ((*next & 1u) == 0) {
      total_size += (*next & ~3u);
  }

  // Use the previous free block's footer to find its size and header
  if ((*header & 2u) == 0) {
      size_t prev_size = *(BlockHeader *)((char *)header - sizeof(BlockHeader));
      header = (BlockHeader *)((char *)header - prev_size);
      total_size += prev_size;
  }

  // Store the combined size, preserving the previous-block bit
  *header = total_size | (*header & 2u);

  // Store the combined size in the footer
  *(BlockHeader *)((char *)header + total_size - sizeof(BlockHeader)) = total_size;

  // The block after the combined block now has a free predecessor
  BlockHeader *after = (BlockHeader *)((char *)header + total_size);
  *after &= ~2u;

}

