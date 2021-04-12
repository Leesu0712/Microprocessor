/*
 * Segregated Fits
 *
 * The allocator is based on segregated fits implementation.
 * With this approach, the allocator maintains head pointers for each free list.
 * The allocator partition the block sizes by powers of 2 and each free list is associated with a size class.
 * Each list contains potentially different-size blocks whose sizes are members of the size class.
 *
 * In addition, each free list is organized as explicit list.
 * Each free block has its own boundary tags(i.e. header and footer), and pred/succ pointer.
 * Using the pred and succ pointers, the heap is organized as a doubly linked free list.
 * Also, the allocator maintain the lists in address order (not LIFO).
 *
 * To handle the edge condition, the allocator maintain its prologue and epilogue blocks that are
 * always marked as allocated. Especially, epilogue block has zero size.
 *
 * Since the footer is needed only when the previous block is free, allocated blocks don't need footers.
 * Instead, the allocated/free bit is stored in second low-order bit of header of the current block.
 *
 * Struct of a free block
 * -------------------------------------------------------
 * |	    |			|	|	|	 |
 * | header |			| pred 	| succ	| footer |
 * |	    |			|	|	|	 |
 * -------------------------------------------------------
 *
 * Struct of a allocated block
 * -------------------------------------------------------
 * |	    |						 |
 * | header |		payload				 |
 * |	    |						 |
 * -------------------------------------------------------
 *
 * Form of header
 * 31					  3   2   1   0
 * ------------------------------------------------------
 * |					    |	|   |   |
 * ------------------------------------------------------
 *  <--------------------------------------->
 *  			size			<--->
 *  				allocated/free bit of the previous block
 *  						    <--->
 *  					allocated/free bit of the current block
 * Form of footer
 * 31					  3   2   1   0
 * ------------------------------------------------------
 * |					    |	|   |   |
 * ------------------------------------------------------
 *  <------------------------------------------->
 *  			size			    <--->
 *  				allocated/free bit of the current block
 *
 * Struct of the heap
 *
 * -------------------------------------------------------------------------
 * |     Free list	|	   |				|	   |
 * |    headers for	| prologue | 	blocks . . . 		| epilogue |
 * |  each size class	|	   |				|	   |
 * -------------------------------------------------------------------------
 *
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
    "Leesu",
    /* First member's full name */
    "Suhyun Lee",
    /* First member's email address */
    "hyunee0712@yonsei.ac.kr",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

//#define HEAPCHECK

/* Basic constants and macros */
#define WSIZE 4			/* Word and header/footer size (bytes) */
#define DSIZE 8			/* Double word size (bytes) */
#define ALIGNMENT 8		/* Double word alignment */	
#define CHUNKSIZE (1<<8)	/* Extend heap by this amount (bytes) */

/* Variables for size classes */
#define NUMOFCLASS 14		/* Number of size classes */
#define LARGESTPOWEROFTWO 18	
#define SMALLESTPOWEROFTWO (LARGESTPOWEROFTWO - NUMOFCLASS + 2)
#define LARGESTSIZE (1 << LARGESTPOWEROFTWO)		/* The last class has size [(LARGESTSIZE+1) ~ infinity] */ 
#define SMALLESTSIZE (1 << SMALLESTPOWEROFTWO)		/* The first class has size [1 ~ SMALLESTSIZE] */

/* Pack a size, previous allocated bit, and current allocated bit into a word for header */
#define PACK_HDR(size, prev_alloc, alloc) 	((size) | (prev_alloc << 1) | (alloc))
/* Pack a size and allocated bit into a word for footer */
#define PACK_FTR(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)		(*(size_t *)(p))
#define PUT(p, val)	((*(size_t *)(p)) = (val))

/* Read and write a pointer at address p */
#define GET_PTR(p)	((char *)(*(size_t *)(p)))
#define PUT_PTR(p, val)	((*(size_t *)(p)) = (size_t)(val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)	(GET(p) & ~0x7)
#define GET_ALLOC(p)	(GET(p) & 0x1)

/* Read and write a previous allocated bit at address p */
#define GET_PREV_ALLOC(p)	(((GET(p) & 0x2)) >> 1)
#define PUT_PREV_ALLOC(p, val)	PUT(p, ((GET(p) & ~0x2) | (val << 1)))

/* Given block ptr, computer address of its header and footer */
#define HDRP(ptr)	((char *)(ptr) - WSIZE)
#define FTRP(ptr)	((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE)

/* Given block ptr, compute address of next and previous blocks */
#define NEXT_BLKP(ptr)	((char *)(ptr) + GET_SIZE(((char *)(ptr) - WSIZE)))
#define PREV_BLKP(ptr)	((char *)(ptr) - GET_SIZE(((char *)(ptr) - DSIZE)))

/* Given block ptr, compute address of its succ and pred pointer */
#define PREDP(ptr)	((char *)(ptr) + GET_SIZE(HDRP(ptr)) - 2*DSIZE)
#define SUCCP(ptr)	((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE - WSIZE)

/* Given block ptr, compute address of some complex pointers */
#define PREDPOFSUCC(ptr)	PREDP(GET_PTR(SUCCP(ptr)))
#define SUCCPOFPRED(ptr)	SUCCP(GET_PTR(PREDP(ptr)))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Points to the appropriate haeder pointer for the current block size */
static char *head_ptr = 0;

/* Always points to the first entry of the free lists */
static char *first_listp = 0;

static void *extend_heap(size_t words);
static void place(void *ptr, size_t newsize);
static void *find_fit(size_t newsize);
static void *coalesce(void *ptr);
static void insert_block(void *predptr, void *ptr);
static void remove_block(void *ptr);
static void *split_block(void *ptr, size_t fsize, size_t lsize);
static void *get_listp(size_t size);

static int mm_check(void);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((first_listp = mem_sbrk((NUMOFCLASS + 2)*WSIZE)) == (void *)-1)
	return -1;

    /* Initialize every free list entry */
    for (int i = 0; i < NUMOFCLASS; i++) {
        PUT_PTR(first_listp + (i*WSIZE), NULL);
    }

    /* Initialize each header for Prologue and Epilogue block */
    PUT(first_listp + ((NUMOFCLASS)*WSIZE), PACK_HDR(DSIZE, 1, 1));	/* Prologue header */
    PUT(first_listp + ((NUMOFCLASS + 1)*WSIZE), PACK_HDR(0, 1, 1));	/* Epilogue header */

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
	return -1;

    return 0;
}
/*
 * mm_malloc - Allocate a block which has payload of at least given size bytes and return the pointer
 */
void *mm_malloc(size_t size)
{
    size_t newsize;
    size_t extendsize;
    char *ptr;

    /* Ignore spurious requests */
    if (size == 0)
	return NULL;

    /* Caculate the new block size which include overhead, alignment and pointer reqs */
    if (size <= SIZE_T_SIZE) {
	newsize = 2*SIZE_T_SIZE;
    }
    else {
	newsize = ALIGN(size + WSIZE);		/* Add overhead for header */
    }

    /* Get the appropriate head pointer for the requested size */
    head_ptr = get_listp(newsize);

    /* Search the free list for a fit */
    if ((ptr = find_fit(newsize)) != NULL) {
	place(ptr, newsize);
	return ptr;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = (newsize > CHUNKSIZE ? newsize : CHUNKSIZE);
    if ((ptr = extend_heap(extendsize/WSIZE)) == NULL)
	return NULL;
    place(ptr, newsize);

#ifdef HEAPCHECK
    mm_check();
#endif

    return ptr;
}

/*
 * mm_free - Free the block pointed to by ptr.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(ptr));

    PUT(HDRP(ptr), PACK_HDR(size, prev_alloc, 0));
    PUT(FTRP(ptr), PACK_FTR(size, 0));

    /* Mark the prev alloc tag into the next block */
    PUT_PREV_ALLOC(HDRP(NEXT_BLKP(ptr)), 0);

    coalesce(ptr);

#ifdef HEAPCHECK
    mm_check();
#endif
}

/*
 * mm_realloc - Reallocate block at the same address as the given address as possible.
 * 		A block is allcated using mm_malloc and mm_free only when it isn't possible
 * 		to reallocate at the same address because there's no usable free space.
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr = NULL;
    void *extendptr, *retptr;
    size_t oldsize, newsize, prev_alloc, next_alloc, next_size, sizediff, extendsize;

    /* Caculate the new block size which include overhead, alignment and pointer reqs */
    if ((ptr == NULL) && (!size))
	return NULL;

    if (size <= SIZE_T_SIZE) {
	newsize = 2*SIZE_T_SIZE;
    }
    else {
	newsize = ALIGN(size + WSIZE);		/* Add overhead for header */
    }

    oldsize = GET_SIZE(HDRP(oldptr));
    prev_alloc = GET_PREV_ALLOC(HDRP(oldptr));
    next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(oldptr)));
    next_size = GET_SIZE(HDRP(NEXT_BLKP(oldptr)));

    sizediff = (oldsize > newsize) ? (oldsize - newsize) : (newsize - oldsize); 

    if (oldsize >= newsize) {			/* The block is downsized through reallocating */
        /* The size of free block need to be splited */
	if (sizediff >= 2*SIZE_T_SIZE) {
	    ptr = split_block(oldptr, newsize, sizediff);
	    coalesce(ptr);
	    retptr = oldptr;
	}
        /* The size of free block doesn't need to be splited */
	else {
	    retptr = oldptr;
	}
    }
    /* The block is extended through reallocating */
    else if (!GET_SIZE(HDRP(NEXT_BLKP(oldptr)))) {		/* Given block is the last block of the heap */
	/* Extend the heap */
        extendsize = (sizediff > CHUNKSIZE ? sizediff : CHUNKSIZE);
        if ((long)(extendptr = mem_sbrk(extendsize)) == -1)
	    return NULL;

        /* Initialize free block header/footer */
        PUT(HDRP(extendptr), PACK_HDR(extendsize, 1, 0));		/* Extended block header */
        PUT(FTRP(extendptr), PACK_FTR(extendsize, 0)); 			/* Extended block footer */
        PUT(HDRP(NEXT_BLKP(extendptr)), PACK_HDR(0, 0, 1));		/* New epilogue header */

        /* The size of free block need to be splited */
	if ((extendsize - sizediff) > 2*SIZE_T_SIZE) {
	    ptr = split_block(oldptr, newsize, extendsize - sizediff);
	    coalesce(ptr);
	    retptr = oldptr;
	}
        /* The size of free block doesn't need to be splited */
	else {
	    newsize = oldsize + extendsize;
	    PUT(HDRP(oldptr), PACK_HDR(newsize, prev_alloc, 1));
	    retptr = oldptr;
	}
    }	
    else {	
        /* Next block can be used to extend the size */	    
	if ((!next_alloc) && (next_size >= sizediff)) {
	    /* Remove old block from the appropriate free list */
	    remove_block(NEXT_BLKP(oldptr));
            /* The size of free block need to be splited */
	    if ((next_size - sizediff) >= 2*SIZE_T_SIZE) {
	        /* The former block is allocated*/
		ptr = split_block(oldptr, newsize, next_size - sizediff);
		coalesce(ptr);
		retptr = oldptr;
	    }
            /* The size of free block doesn't need to be splited */
	    else {
		PUT(HDRP(oldptr), PACK_HDR(oldsize + next_size, prev_alloc, 1));
		retptr = oldptr;
	    }
	}
	/* Default: Just allocate new block and free the old block */
	else {
	    newptr = mm_malloc(size);
	
	    memcpy(newptr, oldptr, (newsize - WSIZE));
	    mm_free(oldptr);

	    retptr = newptr;
	}
    }

#ifdef HEAPCHECK
    mm_check();
#endif

    return retptr;
}

/*
 * coalesce - If the given block doesn't need to be coalesced, just insert the block to the appropriate free list.
 * 	      It the given block need to be coalesced, 1) Remove old block from the list
 * 	      					       2) Create new, large coalesced block
 * 	      					       3) Add new block to free list (Insertion policy: address order)
 */
static void *coalesce(void *ptr)
{
    void *searchptr, *insertptr;
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(ptr));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t size = GET_SIZE(HDRP(ptr));

    /* Case 1: Given block doesn't need to be coalesced */
    if (prev_alloc && next_alloc) {
    }
    /* Case 2: Given block need to be coalesced wih next block */
    else if (prev_alloc && !next_alloc) {		/* Case 2 */
	/* Remove old block from the appropriate free list */
	remove_block(NEXT_BLKP(ptr));

	size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
	PUT(HDRP(ptr), PACK_HDR(size, prev_alloc, 0));
	PUT(FTRP(ptr), PACK_FTR(size, 0));
    }
    /* Case 3: Given block need to be coalesced wih prev block */
    else if (!prev_alloc && next_alloc) {
	/* Remove old block from the appropriate free list */
	remove_block(PREV_BLKP(ptr));

	size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
	PUT(FTRP(ptr), PACK_FTR(size, 0));
	prev_alloc = GET_PREV_ALLOC(HDRP(PREV_BLKP(ptr)));
	PUT(HDRP(PREV_BLKP(ptr)), PACK_HDR(size, prev_alloc, 0));
	ptr = PREV_BLKP(ptr);
    }
    /* Case 4: Given block need to be coalesced with prev and next block */
    else {
	/* Remove old block from the appropriate free list */
	remove_block(NEXT_BLKP(ptr));
	remove_block(PREV_BLKP(ptr));

	size += GET_SIZE(HDRP(PREV_BLKP(ptr))) + GET_SIZE(FTRP(NEXT_BLKP(ptr)));
	prev_alloc = GET_PREV_ALLOC(HDRP(PREV_BLKP(ptr)));
	PUT(HDRP(PREV_BLKP(ptr)), PACK_HDR(size, prev_alloc, 0));
	PUT(FTRP(NEXT_BLKP(ptr)), PACK_FTR(size, 0));
	ptr = PREV_BLKP(ptr);
    }
    
    /* Get the appropriate free list */
    head_ptr = get_listp(size);
    
    /* Insert the new free block in address order */
    /* Case 1: the block will be inserted to the first of the free list */
    if (GET_PTR(head_ptr) == NULL) {
	insertptr = head_ptr;
    }
    else {
	if ((void *)GET_PTR(head_ptr) > ptr) {
	   insertptr = head_ptr;
	}
	/* Case 2: the block will be inserted to the appropriate location */
	else {
	    for (searchptr = GET_PTR(head_ptr); GET_PTR(SUCCP(searchptr)) != NULL; searchptr = GET_PTR(SUCCP(searchptr))) {
		if (searchptr > ptr) {
		    break;
		}
	    }
	    if (searchptr > ptr) {
		insertptr = GET_PTR(PREDP(searchptr));
	    }
	    else {
		insertptr = searchptr;
	    }
	}
    }
    insert_block(insertptr, ptr);

    return ptr;
}

/*
 * insert_block - Insert the given ptr block to the next of the predptr block 
 * 		  (i.e., the given ptr block will be the successor of the predptr block)
 */
static void insert_block(void *predptr, void *ptr)
{
    if (predptr == head_ptr) {
	if (GET_PTR(head_ptr) == NULL) {
	    PUT_PTR(PREDP(ptr), NULL);
	    PUT_PTR(SUCCP(ptr), NULL);
	    PUT_PTR(head_ptr, ptr);
	}
	else {
	    PUT_PTR(PREDP(ptr), NULL);
	    PUT_PTR(SUCCP(ptr), GET_PTR(head_ptr));
	    PUT_PTR(PREDP(GET_PTR(head_ptr)), ptr);
	    PUT_PTR(head_ptr, ptr);
	}   
    }
    else if (GET_PTR(SUCCP(predptr)) == NULL) {
	PUT_PTR(PREDP(ptr), predptr);
	PUT_PTR(SUCCP(ptr), NULL);
	PUT_PTR(SUCCP(predptr), ptr);
    }
    else {
	PUT_PTR(PREDP(ptr), predptr);
	PUT_PTR(SUCCP(ptr), GET_PTR(SUCCP(predptr)));
	PUT_PTR(PREDPOFSUCC(predptr), ptr);
	PUT_PTR(SUCCP(predptr), ptr);
    }	
}

/*
 * remove_block - Remove the given block from the appropriate free list
 */
static void remove_block(void *ptr)
{
    /* Get the appropriate free list for the given block */
    head_ptr = get_listp(GET_SIZE(HDRP(ptr)));

    if ((GET_PTR(PREDP(ptr)) == NULL) && (GET_PTR(SUCCP(ptr)) == NULL))
	PUT_PTR(head_ptr, GET_PTR(SUCCP(ptr)));
    else if (GET_PTR(PREDP(ptr)) == NULL) {
	PUT_PTR(PREDPOFSUCC(ptr), NULL);
	PUT_PTR(head_ptr, GET_PTR(SUCCP(ptr)));
    }
    else if (GET_PTR(SUCCP(ptr)) == NULL)
	PUT_PTR(SUCCPOFPRED(ptr), NULL);
    else {
	PUT_PTR(PREDPOFSUCC(ptr), GET_PTR(PREDP(ptr)));
	PUT_PTR(SUCCPOFPRED(ptr), GET_PTR(SUCCP(ptr)));
    }
	
}

/*
 * split_block - Split the free block into two blocks. 
 * 		 The former is used as allocated block with fsize and the latter is left to be free block with lsize.
 * 		 After the spliting, return the pointer of free block.
 */
static void *split_block(void *ptr, size_t fsize, size_t lsize)
{
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(ptr));

    PUT(HDRP(ptr), PACK_HDR(fsize, prev_alloc, 1));

    ptr = NEXT_BLKP(ptr);
    PUT(HDRP(ptr), PACK_HDR(lsize, 1, 0));
    PUT(FTRP(ptr), PACK_FTR(lsize, 0));
    
    return ptr;
}

/*
 * get_listp - Calculate the appropriate head pointer for the given block according to the size class
 */
static void *get_listp(size_t size)
{
    int index = 0;

    if (size < SMALLESTSIZE) {		/* The first class */
	return first_listp;
    }
    else if (size >= LARGESTSIZE) {	/* The last class */
	return first_listp + ((NUMOFCLASS - 1)*WSIZE);
    }
    else {
	for (int i = SMALLESTSIZE; i < size; i *= 2) { 
	    index++;
	}
    return first_listp + (index * WSIZE);
    }
}

/*
 * extend_heap - Extend the heap by at least given size and return the coalesced block pointer
 */
static void *extend_heap(size_t words)
{
    char *ptr;
    size_t size, prev_alloc;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(ptr = mem_sbrk(size)) == -1)
	return NULL;

    /* Get prev alloc tag from old epilogue header */
    prev_alloc = GET_PREV_ALLOC(HDRP(ptr));

    /* Initialize free block header/footer */
    PUT(HDRP(ptr), PACK_HDR(size, prev_alloc, 0));	/* Free block header */
    PUT(FTRP(ptr), PACK_FTR(size, 0)); 			/* Free block footer */
    PUT(HDRP(NEXT_BLKP(ptr)), PACK_HDR(0, 0, 1));	/* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(ptr);
}

/*
 * find_fit - Find the free block which can hold the given size from the free list. 
 * 	      Use First fit manner.
 */
static void *find_fit(size_t newsize)
{
    /* First fit Search */
    void *ptr;

    for (; head_ptr < (first_listp + (NUMOFCLASS * WSIZE)); head_ptr += WSIZE) {
	for (ptr = GET_PTR(head_ptr); ptr != NULL; ptr = GET_PTR(SUCCP(ptr))) {
	    if (newsize <= GET_SIZE(HDRP(ptr))) {
	        return ptr;
	    }
	}
    }
    return NULL;
}


/*
 * place - Place the given size to the given block.
 * 	   If the size between found free block and the given size is bigger than mimimum block size,
 * 	   split the free block into two blocks and use the former as allocated. The latter is left to be free
 * 	   Otherwise, the entire free block is used for allocation.
 */
static void place(void *ptr, size_t newsize)
{
    size_t foundsize = GET_SIZE(HDRP(ptr));
   
    /* Case 1 : The size of free block need to be splited */
    if ((foundsize - newsize) >= (2*SIZE_T_SIZE)) {
	/* The former block is allocated */
	remove_block(ptr);
	ptr = split_block(ptr, newsize, foundsize-newsize);
	coalesce(ptr);
    }
    /* Case 2 : The size of free block doesn't need to be splited */
    else {
	/* The entire block is allocated */
	remove_block(ptr);
	PUT(HDRP(ptr), PACK_HDR(foundsize, 1, 1));

	/* Mark the prev alloc tag into the next block */
        PUT_PREV_ALLOC(HDRP(NEXT_BLKP(ptr)), 1);
    }
}

/*
 * mm_check - Heap checker. It scans the heap and checks it for consistency.
 */
static int mm_check(void)
{
    void *ptr, *search;
    void *old_head = head_ptr;

    /* Scan the entire heap */
    for(ptr = (first_listp + ((NUMOFCLASS + 2)*WSIZE)); GET_SIZE(HDRP(ptr)) > 0; ptr = NEXT_BLKP(ptr)) {
        /* 1) Check if there are any contiguous free blocks that somehow escaped coalescing */
	if ((GET_ALLOC(HDRP(ptr)) == 0) && (GET_ALLOC(HDRP(NEXT_BLKP(ptr))) == 0)) {
	    printf("Block [%p] is not coalesced with next block [%p]\n", ptr, NEXT_BLKP(ptr));
	    return 0;
	}
	/* 2) Check if every free block is actually in the free list */
	if (GET_ALLOC(HDRP(ptr)) == 0) {
	    head_ptr = get_listp(GET_SIZE(HDRP(ptr)));
	    search = GET_PTR(head_ptr);
	    while (search != ptr) {
		search = GET_PTR(SUCCP(search));
		if (search == NULL) {
		    printf("Block [%p] is free but it's not in the free list\n", ptr);
		    return 0;
		}
	    }
	}
    }
    for (head_ptr = first_listp; head_ptr < (first_listp + (NUMOFCLASS * WSIZE)); head_ptr += WSIZE) {
	for (ptr = GET_PTR(head_ptr); ptr != NULL; ptr = GET_PTR(SUCCP(ptr))) {
            /* 3) Check if every block in the free list is marked as free */
	    if (GET_ALLOC(HDRP(ptr))) {
		printf("Block [%p] is in the free list but isn't marked as free\n", ptr);
		return 0;
	    }
	    /* 4) Check if every block is in the appropriate free list according to its size */
	    if (get_listp(GET_SIZE(HDRP(ptr))) != head_ptr) {
		printf("Block [%p] isn't in the appropriate free list\n", ptr);
		return 0;
	    }
	}
    }

    head_ptr = old_head;

    return 1;
}
