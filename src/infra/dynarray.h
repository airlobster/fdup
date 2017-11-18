// dynarray.h

#ifndef __DYNARRAY_H_
#define	__DYNARRAY_H_

typedef struct _dynarray {
	void** elements;
	long max;
	long length;
	void (*dtor)(void* e);
} dynarray;


dynarray* dynarr_create(void (*dtor)(void* e));
void dynarr_destroy(dynarray* a);
long dynarr_length(dynarray* a);
void dynarr_reset(dynarray* a);
void dynarr_append(dynarray* a, void* e);
void dynarr_set(dynarray* a, long index, void* e);
void* dynarr_get(dynarray* a, long index);
void dynarr_foreach(dynarray* a, int(*f)(void* e, void* ctx), void* ctx);

#endif


