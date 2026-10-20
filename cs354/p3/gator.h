/*======================================================================
 * FILE: gator.h
 * PROJECT: Alloc Gator
 * COURSE: COMP SCI 354 - Fall 2026
 * INSTRUCTOR: Dahl
 * COPYRIGHT: 2026, Dahl
 * Posting or sharing this file with anyone outside of course staff prohibited.
 *----------------------------------------------------------------------
 * DO NOT MAKE ANY CHANGES TO THIS FILE.
 *----------------------------------------------------------------------
 */

#include <stdlib.h>
#include <stdint.h>

#ifndef _GATOR_H_
#define _GATOR_H_

#define PAGE_SIZE ((size_t)4096)
#define ALIGNMENT 16

typedef uint32_t BlockHeader;

void ag_init();
void ag_visualize();

void* ag_alloc(size_t size);
int ag_free(void* ptr);
void ag_coalesce(BlockHeader* ptr);

#endif // _GATOR_H_
