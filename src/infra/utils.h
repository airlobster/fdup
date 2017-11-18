// utils.h

#ifndef __UTILS_H_
#define	__UTILS_H_

#include <stdio.h>


typedef void(*dtor)(void* e);
typedef void*(*clone)(void* e);


void set_verbose(int v);
int get_verbose();


#define	TRACE(msg, ...)		if( get_verbose() ) fprintf(stderr, msg"\n", ##__VA_ARGS__)

#define	LOG(fmt, ...)		if( get_verbose() ) fprintf(stderr, "%s,%d,%s,\""fmt"\"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define	ASSERT(expr, msg)	\
	if( !(expr) ) {	\
		TRACE(msg);	\
		exit(1);	\
	}

//#define	TRACE_MEM(msg)		if( get_verbose() ) { TRACE(">>Memory statitics (%s):", msg); malloc_stats(); }
#define	TRACE_MEM(msg)		;

#endif

