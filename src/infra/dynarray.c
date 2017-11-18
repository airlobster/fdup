// dynarray.h

#include <stdlib.h>
#include "dynarray.h"

#define	ARRAY_PAGE_SIZE	(20)

dynarray* dynarr_create(void (*dtor)(void* e)) {
	dynarray* a = (dynarray*)malloc(sizeof(dynarray));
	a->max = ARRAY_PAGE_SIZE;
	a->elements = (void**)malloc(sizeof(void*) * a->max);
	a->length = 0;
	a->dtor = dtor;
	return a;
}

void dynarr_destroy(dynarray* a) {
	dynarr_reset(a);
	free(a->elements);
	free(a);
}

long dynarr_length(dynarray* a) {
	return a->length;
}

void dynarr_reset(dynarray* a) {
	if( a->dtor ) {
		while( a->length )
			a->dtor(a->elements[--a->length]);
	} else
		a->length = 0;
}

void dynarr_append(dynarray* a, void* e) {
	if( a->length == a->max ) {
		a->max += ARRAY_PAGE_SIZE;
		a->elements = (void**)realloc(a->elements, sizeof(void*) * a->max);
	}
	a->elements[a->length++] = e;
}

void dynarr_set(dynarray* a, long index, void* e) {
	if( index < 0 )
		return;
	// expand array if needed
	if( index >= a->length ) {
		while( a->max < index + 1 )
			a->max += ARRAY_PAGE_SIZE;
		a->elements = (void**)realloc(a->elements, sizeof(void*) * a->max);
		while( a->length < index )
			a->elements[a->length++] = 0;
	}
	// delete old element
	if( a->elements[index] && a->dtor )
		a->dtor(a->elements[index]);
	// assign
	a->elements[index] = e;
}

void* dynarr_get(dynarray* a, long index) {
	if( index >= 0 && index < a->length )
		return a->elements[index];
	return 0;
}

void dynarr_foreach(dynarray* a, int(*f)(void* e, void* ctx), void* ctx) {
	int n;
	for(n=0; n < a->length; ++n) {
		if( ! f(a->elements[n], ctx) )
			break;
	}
}

