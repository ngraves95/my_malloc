#include <stdlib.h>
#include "my_malloc.h"
#include "list.h"

#include <stdio.h>

#define assert(func, exp, act) (func(exp, act, __LINE__))

#define print_field(name) printf("--{%s}--\n", name)
#define print_failure_int(exp, act, line) printf("[FAILURE] Expected: %d. Actual: %d. (Line: %d)\n", exp, act, line)
#define print_failure_ptr(exp, act, line) printf("[FAILURE] Expected: %p. Actual: %p. (Line: %d)\n", exp, act, line)

#define header(msg) fprintf(stderr, "[Line: %d] [TEST CASE %d]  %s \n",  __LINE__, ++tc_count, msg);

// Controls for testing
#define MALLOC_TESTS 1
#define CALLOC_FIRST 0
#define MEMMOVE_TESTS 1
#define STRESS_TEST 0
#define VERBOSE 1
#define SIZE 16
#define CALLOC_SIZE 500


#define POINTER_SIZE 8

#if POINTER_SIZE == 8
	typedef unsigned long int ul;
#else
	typedef unsigned int ul;
#endif

extern metadata_t* freelist[];

static int fail_count = 0;
static int pass_count = 0;
static int tc_count = 0;

typedef int(*comp_func)(void*, void*, int);

void ptr_equal(void*, void*, int);
void int_equal(int, int, int);
void metadata_equal(metadata_t*, metadata_t*, int);
void char_equal(char, char, int);
void mem_equal(char[], char[], int);
metadata_t* get_buddy(metadata_t* ptr);

void print_int(int data)
{
	printf("%d\n", data);
}

void print_freelist()
{
#if VERBOSE
	for (int i = 0; i < 8; i++)
	{
		fprintf(stderr, "[FREELIST %d]\n", i);
		metadata_t* current = freelist[i];
		int j = 0;

		while (j < 5 && current != NULL)
		{
			fprintf(stderr, "\t[%d][address: %p in_use: %d SIZE: %d next: %p prev: %p buddy: %p]\n",j,(void*)current, current->in_use, current->size, (void*)current->next, (void*)current->prev, (void*)get_buddy(current));
			current = current->next;
			j++;
		}

		fprintf(stderr, "\n");
	}
#endif
}


void list_data_equal (void* expected, void* disguised_list, int line)
{
	if ((expected == NULL || disguised_list == NULL) && (expected != disguised_list))
	{
		print_failure_ptr(expected, disguised_list, line);
		return;
	}

	int* arr = (int*) expected;
	LIST* list = (LIST*) disguised_list;
	NODE* current = list->head;

	for (int i = 0; i < list->size; i++)
	{
		int_equal(arr[i], current->data, line);
		current = current->next;
	}


}

void mem_equal(char expected[], char actual[], int line)
{
	int exp_len = sizeof(expected);
	int act_len = sizeof(actual);

	if (exp_len != act_len)
	{
		print_failure_int(exp_len, act_len, line);
		return;
	}


	for (int i = 0; i < exp_len; i++)
	{
		fprintf(stderr, "%d\n", i);
		assert(char_equal, expected[i], actual[i]);

	}
}

/**
 * This is the prototype for NickUnit: a shitty C Unit testing framework
 * The primary calling convention is:
 * 			assert(comparing_func, expected, actual);
 */

/**
 * Checks if 2 pointers are the same
 */
void ptr_equal(void* expected, void* actual, int line)
{
	if (expected != actual)
	{
		print_failure_ptr(expected, actual, line);
		fail_count++;
	}
	else
	{
		pass_count++;
	}
}

void int_equal(int expected, int actual, int line)
{
	if (expected != actual)
	{
		print_failure_int(expected, actual, line);
		fail_count++;
	}
	else
	{
		pass_count++;
	}
}

void char_equal(char expected, char actual, int line)
{
	if (expected != actual)
	{
		print_failure_int((int)expected, (int)actual, line);
		fail_count++;
	}
	else
	{
		pass_count++;
	}
}

void metadata_equal(metadata_t* expected, metadata_t* actual, int line)
{

	int retval = 1;

	if (expected->in_use != actual->in_use)
	{
		retval = 0;
		print_field("metadata_t->in_use");
		print_failure_int(expected->in_use, actual->in_use, line);
	}

	if (expected->size != actual->size)
	{
		retval = 0;
		print_field("metadata_t->size");
		print_failure_int(expected->size, actual->size, line);
	}

	if (expected->next != actual->next)
	{
		retval = 0;
		print_field("metadata_t->next");
		print_failure_ptr((void*)expected->next, (void*)actual->next, line);
	}

	if (expected->prev != actual->prev)
	{
		retval = 0;
		print_field("metadata_t->prev");
		print_failure_ptr((void*)expected->prev, (void*)actual->prev, line);
	}

	if (retval)
	{
		pass_count++;
	}
	else
	{
		fail_count++;
	}
}


int main(void)
{
	/*
	 * What this testing has shown me so far:
	 * [1] pushFront, popFront works
	 * [2] pushBack, popBack work
	 * [3] pushBack, popFront causes a segfault; memory is not freed correctly
	 * [4] pushFront, popBack causes a segfault; memory is not freed correctly
	 */

#if CALLOC_FIRST

	int calloc_type = 0;
	header("Testing my_calloc");
	// my_calloc seems to max out around 26 elements of SIZE 4.
	// maxSIZE  = 26 * 4 + 18 = 80 + 24 + 18 = 104 + 18 = 122. why does 126 fail though?
	//Must be a remnant of
	// my_malloc previously and items not being freed all the way
	// ever after stress testing
	int calloc_exp[CALLOC_SIZE];
	for (int i = 0; i < CALLOC_SIZE; i++)
	{
		// Set to 0 to simulate static initialization
		// Do the pseudo-static so SIZE can be changed easier
		calloc_exp[i] = 0;

	}

	int* calloc_act = my_calloc(CALLOC_SIZE, sizeof(calloc_type));
	if (calloc_act != NULL)
	{
		for (int i = 0; i < CALLOC_SIZE; i++)
		{
			assert(int_equal, 0, calloc_act[i]);
			assert(int_equal, calloc_exp[i], calloc_act[i]);
		}
	}
	else
	{
		fprintf(stderr, "ERRNO = %d\n", ERRNO);
		fprintf(stderr, "my_calloc returned null");
	}

	my_free(calloc_act);
#endif


#if MALLOC_TESTS
	LIST concrete_list = {NULL, NULL, 0};
	LIST* list = &concrete_list;
	//TODO go here for SIZEs and stuff
	int expected[SIZE];

	header("Pushing elements to front...");
	for (int i = 0; i < SIZE; i++)
	{
		pushFront(list, i);
		expected[i] = SIZE - 1 - i;
	}

	print_freelist();

	header("Removing elements from the front...");
	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	while (!is_empty(list))
	{
		popFront(list);
	}

	print_freelist();

	assert(int_equal, 0, list->size);
//#if 0
	header("Adding more elements to the front...");
	for (int i = 0; i < SIZE; i++)
	{
		pushFront(list, i);
		expected[i] = SIZE - 1 - i;
	}


	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	print_freelist();

	header("Removing elements from the front...");
	while (!is_empty(list))
	{
		popFront(list);
	}

	assert(int_equal, 0, list->size);

	print_freelist();

	header("Pushing elements to front...");
	for (int i = 0; i < SIZE; i++)
	{
		pushFront(list, i);
		expected[i] = SIZE - 1 - i;
	}
	// Tests fail when pushing to back, popping front, then adding
	// more elements

	// Pushing front, popping front, then pushing front is good
	// These can be done in any order in any quantity and pass

	assert(list_data_equal, expected, list);

	print_freelist();

	header("Removing the front half of elements...");
	for (int i = 0; i < SIZE / 2; i++)
	{
		popFront(list);
		expected[i] = SIZE - (SIZE / 2) - i - 1;
	}

	assert(int_equal, (SIZE - SIZE / 2), list->size);
	assert(list_data_equal, expected, list);

	header("Removing the rest of the elements...");
	while (!is_empty(list))
	{
		popFront(list);
	}

	assert(int_equal, 0, list->size);

	print_freelist();

	/*
	 * === Test Popping and pushing back
	 */
	header("Pushing elements to the back...");
	for (int i = 0; i < SIZE; i++)
	{
		pushBack(list, i);
		expected[i] = i;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	print_freelist();

	header("Removing all elements from the back...");
	while (!is_empty(list))
	{
		popBack(list);
	}

	assert(int_equal, 0, list->size);

	print_freelist();

	header("Pushing elements to the back...");
	for (int i = 0; i < SIZE; i++)
	{
		pushBack(list, i);
		expected[i] = i;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);
	print_freelist();
	header("Removing all elements from the back...");
	while (!is_empty(list))
	{
		popBack(list);
	}


	assert(int_equal, 0, list->size);
	print_freelist();

	header("Pushing elements to the back...");
	for (int i = 0; i < SIZE; i++)
	{
		pushBack(list, i);
		expected[i] = i;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	header("Removing all items from the back...");
	while (!is_empty(list))
	{
		popBack(list);
	}
	assert(int_equal, 0, list->size);

	print_freelist();

	/*
	 * ==================================
	 * === Test pushBack and popFront ===
	 * ==================================
	 */

	header("Pushing elements to the back...");
	for (int i = 0; i < SIZE; i++)
	{
		pushBack(list, i);
		expected[i] = i;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	print_freelist();

	header("Removing elements from the front...");
	while (!is_empty(list))
	{
		popFront(list);
	}

	assert(int_equal, 0, list->size);

	print_freelist();

	header("Pushing elements to the back...");
	// Seg fault in this block; memory is not freed correctly
	// Does indeed segfault here

	for (int i = 0; i < SIZE; i++)
	{
		pushBack(list, i);
		expected[i] = i;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	print_freelist();

	header("Removing elements from the front...");
	while (!is_empty(list))
	{
		popFront(list);
	}

	assert(int_equal, 0, list->size);

	print_freelist();

	/*
	 * ==================================
	 * === Test pushFront and popBack ===
	 * ==================================
	 */

	header("Pushing elements to the Front...");
	for (int i = 0; i < SIZE; i++)
	{
		pushFront(list, i);
		expected[i] = SIZE - i - 1;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	header("Removing elements from the Back...");
	while (!is_empty(list))
	{
		popBack(list);
	}

	assert(int_equal, 0, list->size);

	header("Pushing elements to the Front...");
	// Seg fault in this block; memory is not freed correctly
	for (int i = 0; i < SIZE; i++)
	{
		pushFront(list, i);
		expected[i] = SIZE - i - 1;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	header("Removing elements from the Back...");
	while (!is_empty(list))
	{
		popBack(list);
	}

	/*
	 * Stress testing
	 */
#if STRESS_TEST
	header("Pushing elements to the Front...");
	// Seg fault in this block; memory is not freed correctly
	for (int i = 0; i < SIZE; i++)
	{
		pushFront(list, i);
		expected[i] = SIZE - i - 1;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	header("Removing elements from the Back...");
	while (!is_empty(list))
	{
		popBack(list);
	}

	header("Pushing elements to the Front...");
	// Seg fault in this block; memory is not freed correctly
	for (int i = 0; i < SIZE; i++)
	{
		pushFront(list, i);
		expected[i] = SIZE - i - 1;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	header("Removing elements from the Back...");
	while (!is_empty(list))
	{
		popBack(list);
	}

	header("Pushing elements to the Front...");
	// Seg fault in this block; memory is not freed correctly
	for (int i = 0; i < SIZE; i++)
	{
		pushFront(list, i);
		expected[i] = SIZE - i - 1;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	header("Removing elements from the Back...");
	while (!is_empty(list))
	{
		popBack(list);
	}

	header("Pushing elements to the Front...");
	// Seg fault in this block; memory is not freed correctly
	for (int i = 0; i < SIZE; i++)
	{
		pushFront(list, i);
		expected[i] = SIZE - i - 1;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	header("Removing elements from the Back...");
	while (!is_empty(list))
	{
		popBack(list);
	}

	header("Pushing elements to the Front...");
	// Seg fault in this block; memory is not freed correctly
	for (int i = 0; i < SIZE; i++)
	{
		pushFront(list, i);
		expected[i] = SIZE - i - 1;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	header("Removing elements from the Back...");
	while (!is_empty(list))
	{
		popBack(list);
	}

	header("Pushing elements to the Front...");
	// Seg fault in this block; memory is not freed correctly
	for (int i = 0; i < SIZE; i++)
	{
		pushFront(list, i);
		expected[i] = SIZE - i - 1;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	header("Removing elements from the Back...");
	while (!is_empty(list))
	{
		popBack(list);
	}

	assert(int_equal, 0, list->size);

	header("Pushing elements to the back...");
	for (int i = 0; i < SIZE; i++)
	{
		pushBack(list, i);
		expected[i] = i;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	header("Removing elements from the front...");
	while (!is_empty(list))
	{
		popFront(list);
	}

	assert(int_equal, 0, list->size);

	header("Pushing elements to the back...");
	// Seg fault in this block; memory is not freed correctly
	for (int i = 0; i < SIZE; i++)
	{
		pushBack(list, i);
		expected[i] = i;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	header("Removing elements from the front...");
	while (!is_empty(list))
	{
		popFront(list);
	}

	assert(int_equal, 0, list->size);

	for (int i = 0; i < SIZE; i++)
	{
		pushBack(list, i);
		expected[i] = i;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	header("Removing elements from the front...");
	while (!is_empty(list))
	{
		popFront(list);
	}

	assert(int_equal, 0, list->size);

	header("Pushing elements to the back...");
	// Seg fault in this block; memory is not freed correctly
	for (int i = 0; i < SIZE; i++)
	{
		pushBack(list, i);
		expected[i] = i;
	}

	assert(int_equal, SIZE, list->size);
	assert(list_data_equal, expected, list);

	//print_freelist();

	header("Removing elements from the front...");
	while (!is_empty(list))
	{
		popFront(list);
	}

	assert(int_equal, 0, list->size);
#endif
#endif

	// Some miscellaneous malloc tests
	LIST* a  = my_malloc(sizeof(LIST));
	LIST* c = my_malloc(sizeof(LIST));
	LIST* b = my_malloc(1000);
	c++;
	my_free(a);
	my_free(b);
	print_freelist();


	//print_freelist();

#if !CALLOC_FIRST
	/*
	 * ===========================================
	 * === Test my_calloc; ensure all set to 0 ===
	 * ===========================================
	 */
	int calloc_type = 0;


	header("Testing my_calloc");
	// my_calloc seems to max out around 26 elements of SIZE 4.
	// maxSIZE  = 26 * 4 + 18 = 80 + 24 + 18 = 104 + 18 = 122. why does 126 fail though?
	//Must be a remnant of
	// my_malloc previously and items not being freed all the way
	// ever after stress testing
	int calloc_exp[CALLOC_SIZE];
	for (int i = 0; i < CALLOC_SIZE; i++)
	{
		// Set to 0 to simulate static initialization
		// Do the pseudo-static so SIZE can be changed easier
		calloc_exp[i] = 0;

	}

	int* calloc_act = my_calloc(CALLOC_SIZE, sizeof(calloc_type));
	if (calloc_act != NULL)
	{
		for (int i = 0; i < CALLOC_SIZE; i++)
		{
			assert(int_equal, 0, calloc_act[i]);
			assert(int_equal, calloc_exp[i], calloc_act[i]);
		}
	}
	else
	{
		fprintf(stderr, "ERRNO = %d\n", ERRNO);
		fprintf(stderr, "my_calloc returned null");
	}


#endif
	/*
	 * =======================
	 * === Test my_memmove ===
	 * =======================
	 */
#if MEMMOVE_TESTS

	header("Basic my_memmove test");
	int src = 0xFF55FF00;
	int out = 0xFFFF0000;
	my_memmove((((char*)&src) + 1), ((char*)&src), 2);
	assert(int_equal, out, src);

#endif

	printf("\nSummary: %d passes, %d failures.\n", pass_count, fail_count);

	if (!fail_count)
	{
		printf("All tests passed!\n");
	}
}


