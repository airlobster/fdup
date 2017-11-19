// set.c

#include <stdlib.h>
#include "set.h"


#define	MAX_BUCKETS	(200)


typedef struct _set_element {
	struct _set_element* next;
	struct _set_element* prev;
	void* v;
	long hash;
} set_element;

typedef struct _set {
	set_element** buckets;
	int num_buckets;
	hash_func fhash;
	dtor fd;
	unsigned long opt;
} set;

static int set_find_element(set* sp, void* e, set_element** ret);


set* set_create(hash_func f, dtor fd, int max_buckets) {
	int i;
	set* sp = (set*)malloc(sizeof(set));
	sp->fhash = f;
	sp->fd = fd;
	sp->opt = 0;
	sp->num_buckets = max_buckets ? max_buckets : MAX_BUCKETS;
	sp->buckets = (set_element**)malloc(sizeof(set_element*) * sp->num_buckets);
	for(i=0; i < sp->num_buckets; ++i)
		sp->buckets[i] = 0;
	return sp;
}

void set_destroy(set* sp) {
	int i;
	for(i=0; i < sp->num_buckets; ++i) {
		while( sp->buckets[i] ) {
			set_element* next = sp->buckets[i]->next;
			if( sp->fd )
				sp->fd(sp->buckets[i]->v);
			free(sp->buckets[i]);
			sp->buckets[i] = next;
		}
	}
	free(sp->buckets);
	free(sp);
}

void set_add(set* sp, void* e) {
	if( set_exists(sp, e) )
		return; // already exists
	set_element* enew = (set_element*)malloc(sizeof(set_element));
	enew->v = e;
	enew->hash = sp->fhash(e);
	enew->prev = 0;
	int bucket = enew->hash % sp->num_buckets;
	enew->next = sp->buckets[bucket];
	if( enew->next )
		enew->next->prev = enew;
	sp->buckets[bucket] = enew;
}

void set_remove(set* sp, void* e) {
	set_element* se = 0;
	if( ! set_find_element(sp, e, &se) )
		return;
	if( se->prev )
		se->prev->next = se->next;
	else
		sp->buckets[se->hash % sp->num_buckets] = se->next;
	if( se->next )
		se->next->prev = se->prev;
	if( sp->fd )
		sp->fd(se->v);
	free(se);
}

static int set_find_element(set* sp, void* e, set_element** ret) {
	int hash = sp->fhash(e);
	set_element* p = sp->buckets[hash % sp->num_buckets];
	while( p ) {
		if( hash == p->hash ) {
			if( ret )
				*ret = p;
			return 1;
		}
		p = p->next;
	}
	return 0;
}

int set_exists(set* sp, void* e) {
	return set_find_element(sp, e, 0);
}

int set_enum(set* sp, int(*f)(void* e, void* ctx), void* ctx) {
	int n = 0;
	int i = 0;
	for(i=0; i < sp->num_buckets; ++i) {
		set_element* e = sp->buckets[i];
		while( e ) {
			++n;
			if( f && f(e->v, ctx) )
				return n;
			e = e->next;
		}
	}
	return n;
}

