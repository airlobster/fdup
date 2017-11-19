// set.h

#ifndef __SET_H_
#define	__SET_H_

#include "type.h"

typedef struct _set set;

set* set_create(const type* t);
void set_destroy(set* sp);
void set_add(set* sp, void* e);
void set_remove(set* sp, void* e);
int set_exists(set* sp, void* e);
int set_foreach(set* sp, int(*f)(void* e, void* ctx), void* ctx);

#endif


