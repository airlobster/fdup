// type.h

#ifndef __TYPE__H_
#define	__TYPE__H_


typedef void(*destructor)(void* e);
typedef long(*hasher)(void* e);
typedef int(*comparator)(void* e1, void* e2);

typedef struct _type {
	destructor dtor;
	hasher h;
	comparator c;
} type;


#endif

