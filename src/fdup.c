// fdup.c

#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "getoptex.h"
#include "queue.h"

////////////////////////////////////////////////////////////////////////////////////////////

unsigned long files_scanned = 0;

////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _sizepool {
	unsigned long size;
	queue* q;
} sizepool;

sizepool* create_pool(unsigned long size) {
	sizepool* p = (sizepool*)malloc(sizeof(sizepool));
	p->size = size;
	p->q = queue_create((dtor)free, (clone)strdup);
	return p;
}

void destroy_pool(sizepool* sp) {
	queue_destroy(sp->q);
	free(sp);
}

////////////////////////////////////////////////////////////////////////////////////////////

int scan(const char* dir, int(*f)(const struct dirent* e, const char* fullpath, void* ctx), void* ctx) {
	DIR* dp;
	struct dirent* e;
	struct stat st;
	char fullpath[1024];
	int r = 0;

	dp = opendir(dir);
	if( ! dp ) {
		fprintf(stderr, "Failed to open directory \"%s\"!\n", dir);
		return -1;
	}

	while( (e=readdir(dp)) ) {
		snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, e->d_name);
		lstat(fullpath, &st);
		if( S_ISDIR(st.st_mode) ) {
			if( strcmp(e->d_name, ".")==0 || strcmp(e->d_name, "..")==0 )
				continue;
			if( (r=scan(fullpath, f, ctx)) )
				break;
		} else {
			if( (r=f(e, fullpath, ctx)) )
				break;
		}
	}

	closedir(dp);

	return r;
}

////////////////////////////////////////////////////////////////////////////////////////////

static int pfn_printpoolentry(void* e, void* ctx) {
	const char* path = (const char*)e;
	fprintf(stdout, "\t%s\n", path);
	return 1;
}

static int pfn_printpool(void* e, void* ctx) {
	int* grpIndex = (int*)ctx;
	sizepool* sp = (sizepool*)e;
	fprintf(stdout, "#%d\n", ++*grpIndex);
	queue_enum(sp->q, pfn_printpoolentry, 0);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////

static unsigned long get_file_size(const char* path) {
	struct stat st;
	if( stat(path, &st) == 0 )
		return st.st_size;
	return 0;
}

static int pfn_findpool(void* e, void* ctx) {
	sizepool* sp = (sizepool*)e;
	unsigned long size = (unsigned long)ctx;
	return sp->size == size;
}

static int checkfilebysize(const struct dirent* e, const char* fullpath, void* ctx) {
	queue* pools = (queue*)ctx;
	unsigned long size = get_file_size(fullpath);
	sizepool* sp = queue_find(pools, pfn_findpool, (void*)size);
	if( ! sp ) {
		sp = create_pool(size);
		queue_pushtail(pools, sp);
	}
	queue_pushtail(sp->q, strdup(fullpath));
	++files_scanned;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////

static long checksum(const char* filename) {
	static const size_t bufsize = 1024 * 128;
	FILE* f = 0;
	long cs = 0;
	if( ! (f=fopen(filename,"rb")) )
		return 0;
	char* buf = (char*)malloc(bufsize);
	while( 1 ) {
		int cb = fread(buf, 1, bufsize, f);
		if( cb <= 0 )
			break;
		const char* p = buf;
		for(int i=1; i <= cb; ++i)
			cs ^= *p++ * i;
	}
	free(buf);
	fclose(f);
	return cs;
}

static int pfn_checksum(void* e, void* ctx) {
	queue* q = (queue*)ctx;
	const char* path = (const char*)e;
	long cs = checksum(path);
	sizepool* sp = queue_find(q, pfn_findpool, (void*)cs);
	if( ! sp ) {
		sp = create_pool(cs);
		queue_pushtail(q, sp);
	}
	queue_pushtail(sp->q, strdup(path));
	return 1;
}

static int pfn_scanpool(void* e, void* ctx) {
	sizepool* sp = (sizepool*)e;
	queue_enum(sp->q, pfn_checksum, ctx);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////

GETOPT_BEGIN(_options)
	GETOPT_OPT('d',"directory",required_argument)
	GETOPT_OPT('v',"verbose",no_argument)
GETOPT_END();

static void cmdopt(int c, char* const arg, void* ctx) {
	switch( c ) {
		case 'd': {
			scan(arg, checkfilebysize, ctx);
			break;
		}
		case 'v': {
			set_verbose(1);
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

static int condition_single_element_pool(void* e, void* ctx) {
	sizepool* sp = (sizepool*)e;
	return queue_length(sp->q) < 2;
}

int main(int argc, char* const* argv) {
	queue* pools;
	pools = queue_create((dtor)destroy_pool, 0);
	getopt_ex(argc, argv, _options, cmdopt, pools);
	TRACE("#of files scanned: %lu", files_scanned);
	// delete all pools with a single element
	queue_delete_elements(pools, condition_single_element_pool, 0);
	TRACE("#of groups of files of the same size: %ld", queue_length(pools));
	if( queue_length(pools) ) {
		queue* newpools = queue_create((dtor)destroy_pool, 0);
		queue_enum(pools, pfn_scanpool, newpools);
		queue_delete_elements(newpools, condition_single_element_pool, 0);
		TRACE("#of groups of files with the same content: %ld", queue_length(newpools));
		queue_destroy(pools);
		pools = newpools;
	}
	// print pool
	int grpIndex = 0;
	queue_enum(pools, pfn_printpool, &grpIndex);
	queue_destroy(pools);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////

