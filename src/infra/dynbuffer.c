// dynbuffer.h

#include <stdlib.h>
#include "dynbuffer.h"

#define	INITIAL_BUF_PAGE_LEN	(128)

dynbuffer* dynbuf_create() {
	dynbuffer* db = (dynbuffer*)malloc(sizeof(dynbuffer));
	db->max = INITIAL_BUF_PAGE_LEN;
	db->pch = (char*)malloc(db->max+1);
	db->wr = db->pch;
	*db->wr = 0;
	return db;
}

void dynbuf_destroy(dynbuffer* db) {
	free(db->pch);
	free(db);
}

void dynbuf_reset(dynbuffer* db) {
	db->wr = db->pch;
}

long dynbuf_length(dynbuffer* db) {
	return db->wr - db->pch;
}

void dynbuf_append_char(dynbuffer* db, char c) {
	long ofs = db->wr - db->pch;
	if( ofs == db->max ) {
		db->max += INITIAL_BUF_PAGE_LEN;
		db->pch = (char*)realloc(db->pch, db->max+1);
		db->wr = db->pch + ofs;
	}
	*db->wr++ = c;
	*db->wr = 0;
}

void dynbuf_append_str(dynbuffer* db, const char* s, long n) {
	while( n-- ) {
		dynbuf_append_char(db, *s++);
	}
}

const char* dynbuf_str(dynbuffer* db) {
	return db->pch;
}


