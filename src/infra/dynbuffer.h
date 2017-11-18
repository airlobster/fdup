// dynbuffer.h

#ifndef __DYNBUFFER_H_
#define	__DYNBUFFER_H_

typedef struct _dynbuffer {
	char* pch;
	long max;
	char* wr;
} dynbuffer;


dynbuffer* dynbuf_create();
void dynbuf_destroy(dynbuffer* db);
void dynbuf_reset(dynbuffer* db);
long dynbuf_length(dynbuffer* db);
void dynbuf_append_char(dynbuffer* db, char c);
void dynbuf_append_buf(dynbuffer* db, const char* s, long n);
const char* dynbuf_str(dynbuffer* db);

#endif

