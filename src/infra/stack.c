// stack.c

#include <stdlib.h>
#include "utils.h"
#include "stack.h"


stack* stack_create() {
	stack* s = (stack*)malloc(sizeof(stack));
	s->a = queue_create(0, 0);
	return s;
}

void stack_destroy(stack* s) {
	queue_destroy(s->a);
	free(s);
}

int stack_size(stack* s) {
	return queue_length(s->a);
}

void stack_push(stack* s, void* e) {
	queue_pushhead(s->a, e);
}

void* stack_pop(stack* s) {
	return queue_pophead(s->a);
}

void* stack_peek(stack* s) {
	return queue_peekhead(s->a);
}


