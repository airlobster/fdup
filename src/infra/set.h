// set.h

#ifndef __SET_H_
#define	__SET_H_

#include "utils.h"

typedef struct _set set;
typedef long (*hash_func)(void* e);

set* set_create(hash_func f, dtor fd, int max_buckets);
void set_destroy(set* sp);
void set_add(set* sp, void* e);
void set_remove(set* sp, void* e);
int set_exists(set* sp, void* e);
int set_enum(set* sp, int(*f)(void* e, void* ctx), void* ctx);

#endif


