#include "my_malloc.h"

/* You *MUST* use this macro when calling my_sbrk to allocate the
 * appropriate size. Failure to do so may result in an incorrect
 * grading!
 */
#define SBRK_SIZE 2048

/* If you want to use debugging printouts, it is HIGHLY recommended
 * to use this macro or something similar. If you produce output from
 * your code then you will receive a 20 point deduction. You have been
 * warned.
 */

//#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x) fprintf(stderr, x)
#else
#define DEBUG_PRINT(x)
#endif

#define FREELIST_LENGTH 8
#define POINTER_SIZE 8

/* make sure this always points to the beginning of your current
 * heap space! if it does not, then the grader will not be able
 * to run correctly and you will receive 0 credit. remember that
 * only the _first_ call to my_malloc() returns the beginning of
 * the heap. sequential calls will return a pointer to the newly
 * added space!
 * Technically this should be declared static because we do not
 * want any program outside of this file to be able to access it
 * however, DO NOT CHANGE the way this variable is declared or
 * it will break the autograder.
 */
void* heap;

/* our freelist structure - this is where the current freelist of
 * blocks will be maintained. failure to maintain the list inside
 * of this structure will result in no credit, as the grader will
 * expect it to be maintained here.
 * Technically this should be declared static for the same reasons
 * as above, but DO NOT CHANGE the way this structure is declared
 * or it will break the autograder.
 */
metadata_t* freelist[8];
/**** SIZES FOR THE FREE LIST ****
 * freelist[0] -> 16
 * freelist[1] -> 32
 * freelist[2] -> 64
 * freelist[3] -> 128
 * freelist[4] -> 256
 * freelist[5] -> 512
 * freelist[6] -> 1024
 * freelist[7] -> 2048
 */
#if POINTER_SIZE == 8
	typedef unsigned long int ul;
#else
	typedef unsigned int ul;
#endif

void merge(metadata_t* block);
metadata_t* get_buddy(metadata_t* block);
void remove_head(metadata_t* block, int fl_index);
void remove_meta(metadata_t* block);
void add_head(metadata_t* block, int index);

/**
 * Calculates the log_2 of a number.
 */
int log_2(int n)
{
	int i = 0;

	while (n >> (i + 1))
	{
		i++;
	}

	return i;
}

void split(metadata_t* chunk, int currentIndex)
{
	int new_size = chunk->size / 2;
	metadata_t *orig, *new;

	orig = chunk;
	new = (metadata_t*)(((char*)chunk) + new_size);

	freelist[currentIndex] = chunk->next;

	if (freelist[currentIndex] != NULL)
	{
		freelist[currentIndex]->prev = NULL;
	}

	// Make blocks point to each other
	orig->next = new;
	new->prev = orig;
	// Doesnt hurt to set exterior pointers to NULL
	orig->prev = NULL;
	new->next= NULL;
	// Set blocks to be free
	orig->in_use = 0;
	new->in_use = 0;
	// Set block size
	orig->size = new_size;
	new->size = new_size;

	freelist[currentIndex - 1] = orig;
}


void* my_malloc(size_t size)
{
	static int init_flag = 1;

	int adj_size = size + sizeof(metadata_t);

	// Checks for bad size requests
	if (size <= 0)
	{
		return NULL;
	}

	// Makes sure user doesnt request a size too large
	if (adj_size > SBRK_SIZE)
	{
		ERRNO = SINGLE_REQUEST_TOO_LARGE;
		return NULL;
	}

	// Block below gets the freelist index
	short block_size = 16;
	int i = 0;

	while (block_size < adj_size)
	{
		block_size *= 2;
		i++;
	}

	// Execute this branch if there is no sufficient blocks at the needed index
	if (freelist[i] == NULL)
	{
		//determine whether out of memory or can split some
		int j = i;
		while (j < FREELIST_LENGTH && freelist[j] == NULL)
		{
			j++;
		}

		// execute this branch if out of memory
		if (j >= FREELIST_LENGTH)
		{
			// Expand heap if there is not enough memory currently allocated
			metadata_t* breakval = my_sbrk(SBRK_SIZE);
			// Check error codes
			if (breakval == (void*)-1)
			{
				ERRNO = OUT_OF_MEMORY;
				return NULL;
			}
			//breakval should never actually be null
			if (breakval == NULL)
			{
				return NULL;
			}
			// Set up the data that we got from sbrk
			freelist[7] = breakval;
			freelist[7]->next = NULL;
			freelist[7]->prev = NULL;
			freelist[7]->size = SBRK_SIZE;
			freelist[7]->in_use = 0;
		}
		else // found some memory we can use
		{
			split(freelist[j], j);
		}

		//call my_malloc again to work with the new data
		return my_malloc(size);
	}
	else // this branch means we have data at the index where we need it
	{
		metadata_t* user_block = freelist[i];

		if (user_block != NULL)
		{
			remove_head(user_block, i);

			user_block->in_use = 1;
			user_block->next = NULL;
			user_block->prev = NULL;
			user_block++; //increment so the address to the free space is returned
		}

		// set up the heap index after the first call to malloc
		if (init_flag)
		{
			heap = user_block - 1; // Decrement to point to actual starting address
			init_flag = 0;
		}

		ERRNO = NO_ERROR;
		return user_block;
	}
}

void* my_calloc(size_t num, size_t size)
{
  int total_size = num * size;

  // Use a char* for size 1. Not most efficient but most reliable
  char* arr = my_malloc(total_size);

  if (arr == NULL)
  {
	  return NULL;
  }

  for (int i = 0; i < total_size; i++)
  {
	  // Set all data to 0.
	  arr[i] = 0;
  }

  return arr;
}

void my_free(void* ptr)
{
	if (ptr == NULL)
	{
		return;
	}

	metadata_t* realptr =((metadata_t*) ptr) - 1;

	if (!realptr->in_use)
	{
		ERRNO = DOUBLE_FREE_DETECTED;
		return;
	}

	realptr->in_use = 0; // Set flag to indicate block is freed
	realptr->next = NULL;
	realptr->prev = NULL;

	merge(realptr);
}

metadata_t* get_buddy(metadata_t* block)
{
	int n = log_2(block->size); // get the 'n' to find the buddy
	ul adj_ptr = (ul)(((char*) block) - (char*)heap);
	ul buddy_mask = (ul)(0x1 << n); // treat as a pointer so can xor with other ptrs

	return (metadata_t*)((adj_ptr ^ buddy_mask) + (char*)heap); //adjust pointer back
}

void remove_head(metadata_t* block, int fl_index)
{
	freelist[fl_index] = block->next;

	if (block->next)
	{
		block->next->prev = NULL;
		block->next = NULL;
	}
}

void remove_meta(metadata_t* block)
{
	if (block->prev)
	{
		block->prev->next = block->next;
	}

	if (block->next)
	{
		block->next->prev = block->prev;
	}

	block->prev = NULL;
	block->next = NULL;
}

void add_head(metadata_t* block, int index)
{
	if (freelist[index] == NULL)
	{
		freelist[index] = block;
		block->next = NULL;
		block->prev = NULL;
	}
	else
	{
		block->next = freelist[index];
		freelist[index]->prev = block;
		block->prev = NULL;
		freelist[index] = block;
	}
}

void merge(metadata_t* block)
{
	int fl_index = log_2(block->size) - 4;
	metadata_t* buddy = get_buddy(block);

	// Base cases where we dont merge
	if (block->size >= SBRK_SIZE)
	{
		add_head(block, 7);
		return;
	}
	else if (buddy->in_use || (buddy->size != block->size))
	{
		add_head(block, fl_index);
		return;
	}

	// Ensure buddy and block are completely removed from the freelist
	while (buddy == freelist[fl_index] || block == freelist[fl_index])
	{
		if (buddy == freelist[fl_index])
		{
			remove_head(buddy, fl_index); // Will make buddy->next NULL
		}

		if (block == freelist[fl_index])
		{
			remove_head(block, fl_index); // Will make block->next NULL
		}

	}

	remove_meta(block); // Take block completely out of the linked list
	remove_meta(buddy); // Take buddy completely out of the linked list

	// Decide which one comes first, then recurse
	if (block > buddy)
	{
		// Buddy comes first
		buddy->size *= 2;
		merge(buddy);
	}
	else
	{
		// Block comes first
		block->size *= 2;
		merge(block);
	}
}


void* my_memmove(void* dest, const void* src, size_t num_bytes)
{
	// if dst > src: go from (src + size) -> src
	// if src > dst: go from src -> (src + size)


	if (((unsigned long int)dest) > ((unsigned long int)src))
	{
		for (int i = (num_bytes - 1); i >= 0; i--)
		{
			*(((char*)dest) + i) = *(((char*)src) + i);
		}

	}
	else
	{
		for (int i = 0; i < num_bytes; i++)
		{
			*(((char*)dest) + i) = *(((char*)src) + i);
		}
	}

	return dest;
}
