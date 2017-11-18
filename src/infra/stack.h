// stack.h

#ifndef __STACK_H_
#define	__STACK_H_

#include "queue.h"

typedef struct _stack {
	queue* a;
} stack;

stack* stack_create();
void stack_destroy(stack* s);
int stack_size(stack* s);
void stack_push(stack* s, void* e);
void* stack_pop(stack* s);
void* stack_peek(stack* s);

#endif


