// queue.h

#include <stdlib.h>
#include "utils.h"
#include "queue.h"

typedef struct _queue_element {
	struct _queue_element* next;
	struct _queue_element* prev;
	void* v;
} queue_element;


static queue_element* queue_create_element(queue* q, void* v) {
	queue_element* e = (queue_element*)malloc(sizeof(queue_element));
	e->next = e->prev = 0;
	e->v = v;
	return e;
}

static void queue_destroy_element(queue* q, queue_element* e) {
	if( q->fd )
		q->fd(e->v);
	free(e);
	--q->length;
}

queue* queue_create(dtor fd, clone fc) {
	queue* q = (queue*)malloc(sizeof(queue));
	q->first = q->last = 0;
	q->length = 0;
	q->fd = fd;
	q->fc = fc;
	return q;
}

void queue_destroy(queue* q) {
	while( q->first ) {
		queue_element* next = q->first->next;
		queue_destroy_element(q, q->first);
		q->first = next;
	}
	free(q);
}

long queue_length(queue* q) {
	return q->length;
}

void queue_pushhead(queue* q, void* v) {
	queue_element* e = queue_create_element(q, v);
	e->next = q->first;
	if( q->first )
		q->first->prev = e;
	q->first = e;
	if( ! q->last )
		q->last = e;
	++q->length;
}

void* queue_pophead(queue* q) {
	ASSERT(q->first, "Queue underflow");
	queue_element* e = q->first;
	void* v = e->v;
	e->v = 0;
	q->first = q->first->next;
	if( q->first )
		q->first->prev = 0;
	if( q->last == e )
		q->last = e->prev;
	queue_destroy_element(q, e);
	--q->length;
	return v;
}

void* queue_peekhead(queue* q) {
	ASSERT(q->first, "Queue underflow");
	return q->first->v;
}

void queue_pushtail(queue* q, void* v) {
	queue_element* e = queue_create_element(q, v);
	e->prev = q->last;
	if( q->last )
		q->last->next = e;
	q->last = e;
	if( ! q->first )
		q->first = e;
	++q->length;
}

void* queue_poptail(queue* q) {
	ASSERT(q->last, "Queue underflow");
	queue_element* e = q->last;
	void* v = e->v;
	e->v = 0;
	q->last = q->last->prev;
	if( q->last )
		q->last->next = 0;
	if( q->first == e )
		q->first = q->first->next;
	queue_destroy_element(q, e);
	--q->length;
	return v;
}

void* queue_peektail(queue* q) {
	ASSERT(q->last, "Queue underflow");
	return q->last->v;
}

void queue_enum(queue* q, int(*f)(void* e, void* ctx), void* ctx) {
	queue_element* e = q->first;
	while( e ) {
		if( ! f(e->v, ctx) )
			break;
		e = e->next;
	}
}

void queue_select(queue* q, int(*condition)(void* e, void* ctx), void* ctx, queue** pqueue) {
	queue* r = queue_create(q->fd, q->fc);
	queue_element* e = q->first;
	while( e ) {
		if( ! condition || condition(e->v, ctx) )
			queue_pushtail(r, q->fc ? q->fc(e->v) : e->v);
		e = e->next;
	}
	if( pqueue )
		*pqueue = r;
	else
		queue_destroy(r);
}

void* queue_find(queue* q, int(*condition)(void* e, void* ctx), void* ctx) {
	queue_element* e = q->first;
	while( e ) {
		if( condition && condition(e->v, ctx) )
			return e->v;
		e = e->next;
	}
	return 0;
}

int queue_delete_elements(queue* q, int(*condition)(void* e, void* ctx), void* ctx) {
	int n = 0;
	queue_element* e = q->first;
	while( e ) {
		if( condition && condition(e->v, ctx) ) {
			queue_element* old = e;
			if( e->prev )
				e->prev->next = e->next;
			if( e->next )
				e->next->prev = e->prev;
			if( q->first == e )
				q->first = e->next;
			if( q->last == e )
				q->last = e->prev;
			e = e->next;
			queue_destroy_element(q, old);
			++n;
		} else {
			e = e->next;
		}
	}
	return n;
}


