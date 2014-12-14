#include <stdlib.h>
#include "./list.h"
#include "my_malloc.h"

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x) fprintf(stderr, x)
#else
#define DEBUG_PRINT(x)
#endif

static NODE* create_node(int data)
{
	NODE* node = my_malloc(sizeof(NODE));

	if (node != NULL)
	{
		node->next = NULL;
		node->prev = NULL;
		node->data = data;
	}

	return node;
}

int is_empty(LIST* llist)
{
    return (!llist->size && llist->head == NULL);
}

LIST* createList(void)
{
	LIST* linked = my_malloc(sizeof(LIST));

	if(linked != NULL)
	{
		linked->size = 0;
		linked->head = NULL;
		linked->tail = NULL;
	}

    return linked;
}

/**
 * pushFront
 * Takes in ptr to a list and data to add
 * Should add new node containing the data to the head of the list, and increment size
 */
void pushFront(LIST* list, int data)
{
	NODE* new_head = create_node(data);

	if (new_head == NULL)
	{
		DEBUG_PRINT("Malloc failed -pushFront\n");
		return;
	}
	else if (is_empty(list))
	{
		list->head = new_head;
		list->tail = new_head;
		list->size++;
	}
	else
	{
		new_head->next = list->head;
		new_head->prev = NULL;
		list->head->prev = new_head;
		list->head = new_head;
		list->size++;
	}
}

/**
 * pushBack
 * Takes in ptr to a list and data to add
 * Should add new node containing the data to the tail of the list, and increment size
 */
void pushBack(LIST* list, int data)
{
	NODE* new_tail = create_node(data);

	if (new_tail == NULL)
	{
		DEBUG_PRINT("Malloc failed -pushBack\n");
		return;
	}
	else if (is_empty(list))
	{
		list->head = new_tail;
		list->tail = new_tail;
		list->size++;
	}
	else
	{
		new_tail->next = NULL;
		new_tail->prev = list->tail;
		list->tail->next = new_tail;
		list->tail = new_tail;
		list->size++;
	}
}

/**
 * popFront
 * Takes in ptr to a list
 * Remove and free node at the front of the list, and decrement size
 * Return the value of that node
 * Return 0 if the size of the list is 0
 */
int popFront(LIST* list)
{
	int retval = 0;

	if (is_empty(list))
	{
		// Do nothing
	}
	else if (list->head == list->tail)
	{
		NODE* temp = list->head;
		retval = temp->data;
		temp->next = NULL;
		temp->prev = NULL;
		my_free(temp);

		list->head = NULL;
		list->tail = NULL;
		list->size--;
	}
	else
	{
		retval = list->head->data;
		list->head = list->head->next;		// Move head pointer to next node
		list->head->prev->next = NULL;		// Null out old head's next ptr
		my_free(list->head->prev);			// Free old head node
		list->head->prev = NULL; 			// Point new head's prev to NULL
		list->size--;
	}

	return retval;
}

/**
 * popBack
 * Takes in ptr to a list
 * Remove and free node at the back of the list, and decrement size
 * Return the value of that node
 * Return 0 if the size of the list is 0
 */
int popBack(LIST* list)
{
	int retval = 0;

	if (is_empty(list))
	{
		// Do nothing
	}
	else if (list->size == 1)//(list->head == list->tail)
	{
		NODE* temp = list->head;
		retval = temp->data;
		temp->next = NULL;
		temp->prev = NULL;
		my_free(temp);

		list->head = NULL;
		list->tail = NULL;
		list->size--;
	}
	else
	{
		retval = list->tail->data;
		list->tail = list->tail->prev;		// Move tail pointer to previous node
		list->tail->next->prev = NULL;		// Null out old tail's prev ptr
		my_free(list->tail->next);			// Free old tail node
		list->tail->next = NULL; 			// Point new tail's next to NULL
		list->size--;
	}

	return retval;
}

void traverse(LIST* lLIST, list_op do_func)
{
	NODE* current = lLIST->head;

    while (current != NULL)
    {
    	do_func(current->data);
    	current = current->next;
    }
}

