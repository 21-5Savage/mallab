#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

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
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define GET_VALUE(p) (*(size_t *)(p))
#define PUT_VALUE(p, val) (*(size_t *)(p) = (val))
#define PACK(size, alloc, prev_alloc) ((size) | ((alloc) ? 1 : 0) | ((prev_alloc) ? 2 : 0))
#define GET_SIZE(p)  (GET_VALUE(p) & ~7)
#define GET_ALLOC(p) (GET_VALUE(p) & 1)
#define GET_PREV_ALLOC(p) (GET_VALUE(p) & 2)

void *root = NULL;


static inline int check_allocated(void *ptr) {
    return GET_VALUE(ptr) & 1;
}

static inline int check_previous_allocated(void *ptr) {
    return GET_VALUE(ptr) & 2;
}

static inline size_t get_block_size(void *ptr) {
    return GET_VALUE(ptr) & ~3;
}

static inline void *move_ptr(void *ptr, size_t value, int forward) {
    return (void *)((char *)ptr + (long)value * forward);
}

static inline void set_as_allocated(void *ptr, size_t value) {
    PUT_VALUE(ptr, value | 1);
}

static inline void set_header_footer_allocated(void *ptr, size_t value) {
    set_as_allocated(ptr, value);
    PUT_VALUE((char *)ptr + value - SIZE_T_SIZE, value);
}

static inline void mark_previous_as_allocated(void *ptr) {
    PUT_VALUE(ptr, GET_VALUE(ptr) | 2);
}

static inline void mark_previous_as_free(void *ptr) {
    PUT_VALUE(ptr, GET_VALUE(ptr) & ~2);
}

static inline void *find_header(void *ptr_payload) {
    return (void *)((char *)ptr_payload - SIZE_T_SIZE);
}

static inline void *get_next_ptr(void *ptr) {
    size_t size = get_block_size(ptr);
    if ((char *)ptr + size >= (char *)mem_sbrk(0))
        return NULL;
    return move_ptr(ptr, size, 1);
}

static inline void *get_prev_ptr(void *ptr) {
    if (ptr <= root) return NULL;
    ptr = move_ptr(ptr, SIZE_T_SIZE, -1);
    size_t block_size = get_block_size(ptr);
    return move_ptr(ptr, block_size - SIZE_T_SIZE, -1);
}

static inline void set_as_free(void *ptr, size_t value) {
    PUT_VALUE(ptr, value & ~1);
    ptr = get_next_ptr(ptr);
    if (ptr) PUT_VALUE(ptr, GET_VALUE(ptr) & ~2);
}

static inline void set_header_footer_free(void *ptr, size_t value) {
    set_as_free(ptr, value);
    PUT_VALUE((char *)ptr + value - SIZE_T_SIZE, value);
}


static inline void mm_checkheap(int lineno) {
    printf("checkheap called from %d\n", lineno);
}

static inline void print_heap(){
    char *current = root;
    char *heap_end = (char *)mem_sbrk(0);

    while (current < heap_end) {
        printf("ptr: %lu\n", (unsigned long)(uintptr_t)current - (unsigned long)(uintptr_t)root);
        printf("value: %zu\n", get_block_size(current));
        printf("flags: %zu\n\n", GET_VALUE(current) % 8);
        size_t block_size = get_block_size(current);
        current += block_size;
    }
    printf("heap_end: %lu\n", (unsigned long)(uintptr_t)mem_sbrk(0) - (unsigned long)(uintptr_t)root);

}

/*
* mm_init - initialize the malloc package.
*/
int mm_init(void) {
    mem_reset_brk();
    root = mem_sbrk(SIZE_T_SIZE);
    if (root == (void *)-1) return -1;
    set_as_allocated(root, SIZE_T_SIZE);
    // printf("\nafter innit\n\n");
    // print_heap();
    return 0;
}

/*
* mm_malloc - Allocate a block by incrementing the brk pointer.
*     Always allocate a block whose size is a multiple of the alignment.
*/
void *mm_malloc(size_t size) {

    int newsize = ALIGN(size + 2 * SIZE_T_SIZE);
    void *p = root;
    char *current = root;
    int previous_allocated_flag = 1;
    char *heap_end = (char *)mem_sbrk(0);

    while (current < heap_end) {
        size_t block_size = get_block_size(current);
        if (!check_allocated(current) && block_size >= newsize) {
            if (block_size - newsize >= ALIGN(2 * SIZE_T_SIZE + MIN_PAYLOAD)) {
                set_header_footer_allocated(current, newsize);
                p = move_ptr(current, newsize, 1);
                set_header_footer_free(p, block_size - newsize);
                mark_previous_as_allocated(p);
            } else {
                set_header_footer_allocated(current, block_size);
            }
            if (previous_allocated_flag) mark_previous_as_allocated(current);
            previous_allocated_flag = 1;
            // printf("\nafter malloc fit\n\n");
            // print_heap();
            return move_ptr(current, SIZE_T_SIZE, 1);
        } else if (check_allocated(current)) {
            previous_allocated_flag = 1;
        } else {
            previous_allocated_flag = 0;
        }
        current += block_size;
    }

    p = mem_sbrk(newsize);
    if (p == (void *)-1) return NULL;
    set_header_footer_allocated(p, newsize);
    void *prev = get_prev_ptr(p);
    if (check_allocated(prev)) mark_previous_as_allocated(p);
    // printf("\nafter malloc sbrk\n\n");
    // print_heap();
    return move_ptr(p, SIZE_T_SIZE, 1);
}

static inline void coalesce(void *ptr) {
    size_t size = get_block_size(ptr);
    void *prev = get_prev_ptr(ptr);
    void *next = get_next_ptr(ptr);
    int prev_alloc = (prev == NULL) || check_allocated(prev);
    int next_alloc = (next == NULL) || check_allocated(next);

    if (prev_alloc && next_alloc) {
        if (next != NULL) mark_previous_as_free(next);
        set_header_footer_free(ptr, size);
        mark_previous_as_allocated(ptr);
    } else if (prev_alloc && !next_alloc) {
        size += get_block_size(next);
        PUT_VALUE(next, 0);
        set_header_footer_free(ptr, size);
        mark_previous_as_allocated(ptr);
    } else if (!prev_alloc && next_alloc) {
        size += get_block_size(prev);
        PUT_VALUE(prev, 0);
        if (next != NULL) mark_previous_as_free(next);
        set_header_footer_free(prev, size);

        void *prev_prev = get_prev_ptr(prev);
        int prev_prev_alloc = (prev_prev == NULL) || check_allocated(prev_prev);
        if (prev_prev_alloc) mark_previous_as_allocated(prev);

    } else {
        size += get_block_size(prev);
        size += get_block_size(next);
        PUT_VALUE(ptr, 0);
        PUT_VALUE(next, 0);
        set_header_footer_free(prev, size);

        void *prev_prev = get_prev_ptr(prev);
        int prev_prev_alloc = (prev_prev == NULL) || check_allocated(prev_prev);
        if (prev_prev_alloc) mark_previous_as_allocated(prev);

        void *next_next = get_next_ptr(next);
        if (next_next != NULL) mark_previous_as_free(next_next);

    }
}

/*
* mm_free - Free a block and coalesce it with adjacent blocks.
*/
void mm_free(void *ptr_payload) {
    if (ptr_payload == NULL) return;

    void *ptr = move_ptr(ptr_payload, SIZE_T_SIZE, -1);
    size_t block_size = get_block_size(ptr);
    set_header_footer_free(ptr, block_size);
    // printf("\nfree before coalesce\n\n");
    // print_heap();
    coalesce(ptr);
    // printf("\nfree after coalesce\n\n");
    // print_heap();
}

/*
* mm_realloc - Implement realloc using malloc and free.
*/

// void *mm_realloc(void *ptr, size_t size)
// {
//     void *oldptr = ptr;
//     void *newptr;
//     size_t copySize;
    
//     newptr = mm_malloc(size);
//     if (newptr == NULL)
//     return NULL;
//     copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
//     if (size < copySize)
//     copySize = size;
//     memcpy(newptr, oldptr, copySize);
//     mm_free(oldptr);
//     return newptr;
// }

void *mm_realloc(void *ptr_payload, size_t size) {
    if (ptr_payload == NULL) {
        return mm_malloc(size);
    }
    if (size == 0) {
        mm_free(ptr_payload);
        return NULL;
    }

    void *ptr = find_header(ptr_payload);
    size_t old_size = get_block_size(ptr);
    size_t new_size = ALIGN(size + 2 * SIZE_T_SIZE);

    if (new_size <= old_size) {
        if (old_size - new_size >= ALIGN(SIZE_T_SIZE + MIN_PAYLOAD)) {
            set_header_footer_allocated(ptr, new_size);
            void *next = move_ptr(ptr, new_size, 1);
            set_header_footer_free(next, old_size - new_size);
            mark_previous_as_allocated(next);
        }
        return ptr_payload;
    }

    void *next = get_next_ptr(ptr);
    size_t extra_size = 0;
    if (next && !check_allocated(next)) {
        extra_size = get_block_size(next);
        if (old_size + extra_size >= new_size) {
            set_header_footer_allocated(ptr, old_size + extra_size);
            if (old_size + extra_size - new_size >= ALIGN(SIZE_T_SIZE + MIN_PAYLOAD)) {
                void *split = move_ptr(ptr, new_size, 1);
                set_header_footer_free(split, old_size + extra_size - new_size);
                mark_previous_as_allocated(split);
            }
            return ptr_payload;
        }
    }

    void *new_ptr = mm_malloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }

    size_t copy_size = old_size - 2 * SIZE_T_SIZE;
    if (size < copy_size) {
        copy_size = size;
    }
    memcpy(new_ptr, ptr_payload, copy_size);
    mm_free(ptr_payload);
    return new_ptr;
}