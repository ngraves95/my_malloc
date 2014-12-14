#ifndef LIST_H
#define LISTH

typedef struct node {
	int data;
	struct node *next;
	struct node *prev;
} NODE;

typedef struct list {
	NODE *head;
	NODE *tail;
	int size;
} LIST;

typedef void(*list_op)(int);

void pushFront(LIST *list, int data);
void pushBack(LIST *list, int data);
int popFront(LIST *list);
int popBack(LIST *list);
void traverse(LIST* lLIST, list_op do_func);
LIST* createList(void);
int is_empty(LIST*);

#endif
