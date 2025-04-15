/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "22B1NUM7261 + 22B1NUM7226",
    /* First member's full name */
    "Munkhdul Dorjderem",
    /* First member's email address */
    "munhdul1601@gmail.com",
    /* Second member's full name (leave blank if none) */
    "Batenkh Badarch",
    /* Second member's email address (leave blank if none) */
    "batenkhbadarch@gmail.com"
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define MIN_PAYLOAD 1
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

void *root = NULL;
void *heap_end = NULL;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void){
    return 0;
}

int check_allocated(void *ptr){
    return *(size_t *)ptr & 1;
}

int check_previous_allocated(void *ptr){
    return *(size_t *)ptr & 2;
}

size_t get_block_size(void *ptr){
    return *(size_t *)ptr & ~3;
}

void *move_ptr(void *ptr, size_t value, int forward){
    return (void *)((char *)ptr + (long)value * forward);
}

void set_header_footer_allocated(void *ptr, size_t value){
    set_as_allocated(ptr, value);
    *(size_t *)((char *)ptr + value - SIZE_T_SIZE) = value;
}

void set_as_allocated(void *ptr, size_t value){
    *(size_t *)ptr = value | 1;
}

void mark_previous_as_allocated(void *ptr){
    *(size_t *)ptr |= 2;
}

void set_header_footer_free(void *ptr, size_t value){
    set_as_free(ptr, value);
    *(size_t *)((char *)ptr + value - SIZE_T_SIZE) = value;
}

void set_as_free(void *ptr, size_t value){
    *(size_t *)ptr = value & ~1;
}

void mark_previous_as_free(void *ptr){
    *(size_t *)ptr &= ~2;
}

//return header
void *find_header(void * ptr){
    return (void *)((char *)ptr - SIZE_T_SIZE);
}
/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + 2 * SIZE_T_SIZE);
    // printf("Newsize: %d\n", newsize);
    void *p = root;
    if (!root){
        // printf("\naaaa\n");
        root = mem_sbrk(SIZE_T_SIZE);
        heap_end = (char *)root + SIZE_T_SIZE - 1;
        if (root == (void *)-1){
            // printf("\nbbbb\n");
            return NULL;
        }else{
            set_as_allocated(root, SIZE_T_SIZE);
            p = move_ptr(root, SIZE_T_SIZE, 1);
            // printf("\ncccc\n");
        }
    }
    // printf("\ndddd\n");

    char *current = root;
    int previous_allocated_flag = 0;

    while (current < (char *)heap_end) {
        // printf("current: %p\n", (void *)current);
        // printf("heap_end: %p\n", (void *)heap_end);
        size_t block_size = get_block_size(current);
        // printf("pointer to next value: %zu\n", *(size_t *)current);
        // printf("Block size: %zu\n\n", block_size);
        // sleep(1);
        // printf("\neeee\n");

        if (!check_allocated(current) && block_size >= newsize) {
            // printf("\nffff\n");
            if (block_size - newsize >= ALIGN(SIZE_T_SIZE + MIN_PAYLOAD)) {
                set_header_footer_allocated(current, newsize);
                p = move_ptr(current, newsize, 1);
                set_header_footer_free(p, block_size - newsize);
                mark_previous_as_allocated(p);
            } else {
                set_header_footer_allocated(current, block_size);
            }
            if (previous_allocated_flag) mark_previous_as_allocated(current);
            previous_allocated_flag = 1;
            return move_ptr(current, SIZE_T_SIZE, 1);
        } else if (check_allocated(current)) previous_allocated_flag = 1;
        else previous_allocated_flag = 0;
        current += block_size;

    }
    // printf("\ngggg\n");

    p = mem_sbrk(newsize);
    if (p == (void *)-1) return NULL;
    heap_end = (char *)p + newsize - 1;
    set_header_footer_allocated(p, newsize);
    return move_ptr(p, SIZE_T_SIZE, 1); 
}

// void *mm_malloc(size_t size)
// {
//     int newsize = ALIGN(size + SIZE_T_SIZE);
//     void *p = mem_sbrk(newsize);
//     if (p == (void *)-1)
// 	return NULL;
//     else {
//         *(size_t *)p = size;
//         return (void *)((char *)p + SIZE_T_SIZE);
//     }
// }

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}