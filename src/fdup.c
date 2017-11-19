// fdup.c

#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <fnmatch.h>
#include "utils.h"
#include "getoptex.h"
#include "queue.h"
#include "type.h"
#include "set.h"

////////////////////////////////////////////////////////////////////////////////////////////

typedef enum _fdup_flags {
	FF_DUPSONLY=0x1,
	FF_NOEMPTY=0x2
} fdup_flags;

static unsigned long flags = 0;

#define	SET_OPT(opt)	flags |= (opt)
#define	IS_OPT(opt)	((flags & (opt))==(opt))

////////////////////////////////////////////////////////////////////////////////////////////

static unsigned long files_scanned = 0;
static set* visited = 0;
static queue* ignore = 0;
static int num_dirs = 0;

////////////////////////////////////////////////////////////////////////////////////////////

static int scan(const char* dir, int(*f)(const char* fullpath, void* ctx), void* ctx);
static int pfn_printpoolentry(void* e, void* ctx);
static int pfn_printpool(void* e, void* ctx);
static unsigned long get_file_inode(const char* path);
static unsigned long get_file_size(const char* path);
static int pfn_findpool(void* e, void* ctx);
static int condition_sameinode(void* e, void* ctx);
static int condition_wildcard(void* e, void* ctx);
static int checkfilebysize(const char* fullpath, void* ctx);
static long checksum(const char* filename);
static int condition_samechecksum(void* e, void* ctx);
static int pfn_scanpool(void* e, void* ctx);

////////////////////////////////////////////////////////////////////////////////////////////
// INODE TYPE

static long inode_hash(void* e) {
	return (long)e;
}
static const type tInode = { 0, inode_hash, 0 };

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

void usage(FILE* os) {
	static const char* txt =
		"Usage:\n"
		"\tfdup {options} -d <dir>...\n"
		"Options:\n"
		"\t-d|--directory <dir>\n"
		"\t            Add a directory to the scanning list.\n"
		"\t-i|--ignore <pattern>\n"
		"\t            Add a wildcard pattern to the ignore list.\n"
		"\t-e|--noempty\n"
		"\t            Ignore empty files.\n"
		"\t-l|--list\n"
		"\t            Print only duplicated files as a flat list, while hiding the first file of each group.\n"
		"\t            Using this mode as an input to a 'rm' command will remove only the duplicates, while\n"
		"\t            leaving every first copy untouched.\n"
		"\t-v|--verbose\n"
		"\t            Turn verbose mode on.\n"
		"\t-h|--help\n"
		"\t            Print this help text.\n"
		;
	fputs(txt, os);
}

////////////////////////////////////////////////////////////////////////////////////////////

static int scan(const char* dir, int(*f)(const char* fullpath, void* ctx), void* ctx) {
	DIR* dp;
	struct dirent* e;
	struct stat st;
	char fullpath[1024];
	int r = 0;

	dp = opendir(dir);
	if( ! dp )
		return -1;

	while( (e=readdir(dp)) ) {
		snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, e->d_name);
		// skip if in ignore list
		if( queue_find(ignore, condition_wildcard, (void*)fullpath) )
			continue;
		lstat(fullpath, &st);
		if( S_ISDIR(st.st_mode) ) {
			if( strcmp(e->d_name, ".")==0 || strcmp(e->d_name, "..")==0 )
				continue;
			if( (r=scan(fullpath, f, ctx)) )
				break;
		} else {
			if( (r=f(fullpath, ctx)) )
				break;
		}
	}

	closedir(dp);

	return r;
}

////////////////////////////////////////////////////////////////////////////////////////////

static int pfn_printpoolentry(void* e, void* ctx) {
	const char* path = (const char*)e;
	int* index = (int*)ctx;
	if( IS_OPT(FF_DUPSONLY) ) {
		if( *index > 0 )
			fprintf(stdout, "%s\n", path);
	} else {
		fprintf(stdout, "\t%s\n", path);
	}
	++*index;
	return 1;
}

static int pfn_printpool(void* e, void* ctx) {
	int* grpIndex = (int*)ctx;
	int index = 0;
	sizepool* sp = (sizepool*)e;
	if( ! IS_OPT(FF_DUPSONLY) )
		fprintf(stdout, "#%d\n", *grpIndex);
	queue_enum(sp->q, pfn_printpoolentry, &index);
	++*grpIndex;
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////

static unsigned long get_file_inode(const char* path) {
	struct stat st;
	if( stat(path, &st) == 0 )
		return st.st_ino;
	return 0;
}

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

static int condition_wildcard(void* e, void* ctx) {
	const char* pattern = (const char*)e;
	const char* path = (const char*)ctx;
	return fnmatch(pattern, path, 0) == 0;
}

static int condition_sameinode(void* e, void* ctx) {
	unsigned long id = (unsigned long)e;
	unsigned long cmp = (unsigned long)ctx;
	return id == cmp;
}

static int checkfilebysize(const char* fullpath, void* ctx) {
	++files_scanned;
	// check if this file's inode is already known (visited)
	unsigned long inode = get_file_inode(fullpath);
	if( set_exists(visited, (void*)inode) ) {
		TRACE("File \"%s\" already visited", fullpath);
		return 0;
	}
	queue* pools = (queue*)ctx;
	unsigned long size = get_file_size(fullpath);
	// ignore empty files if so requested
	if( size == 0 && IS_OPT(FF_NOEMPTY) )
		return 0;
	sizepool* sp = queue_find(pools, pfn_findpool, (void*)size);
	if( ! sp ) {
		sp = create_pool(size);
		queue_pushtail(pools, sp);
	}
	queue_pushtail(sp->q, strdup(fullpath));
	set_add(visited, (void*)inode); // register this inode
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////

static long checksum(const char* filename) {
	static const size_t bufsize = 1024 * 128;
	FILE* f = 0;
	long cs = 0;
	int i;
	if( ! (f=fopen(filename,"rb")) )
		return 0;
	char* buf = (char*)malloc(bufsize);
	while( 1 ) {
		int cb = fread(buf, 1, bufsize, f);
		if( cb <= 0 )
			break;
		const char* p = buf;
		for(i=1; i <= cb; ++i)
			cs ^= *p++ * i;
	}
	free(buf);
	fclose(f);
	return cs;
}

static int condition_samechecksum(void* e, void* ctx) {
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
	queue_enum(sp->q, condition_samechecksum, ctx);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////

GETOPT_BEGIN(_options)
	GETOPT_OPT('d',"directory",required_argument)
	GETOPT_OPT('i',"ignore",required_argument)
	GETOPT_OPT('e',"noempty",no_argument)
	GETOPT_OPT('v',"verbose",no_argument)
	GETOPT_OPT('l',"list",no_argument)
	GETOPT_OPT('h',"help",no_argument)
GETOPT_END();

static void cmdopt(int c, char* const arg, void* ctx) {
	switch( c ) {
		case 'd': {
			++num_dirs;
			if( scan(arg, checkfilebysize, ctx) < 0 ) {
				fprintf(stderr, "Failed to open \"%s\"!\n", arg);
				exit(1);
			}
			break;
		}
		case 'i': {
			queue_pushtail(ignore, (void*)strdup(arg));
			break;
		}
		case 'e': {
			SET_OPT(FF_NOEMPTY);
			break;
		}
		case 'l': {
			SET_OPT(FF_DUPSONLY);
			break;
		}
		case 'v': {
			set_verbose(1);
			break;
		}
		case 'h': {
			fprintf(stdout, "fdup - Finding duplicate files.\n");
			usage(stdout);
			exit(0);
			break;
		}
		default: {
			fprintf(stderr, "Invalid option!\n");
			usage(stderr);
			exit(1);
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
	visited = set_create(&tInode);
	ignore = queue_create((dtor)free, 0);
	pools = queue_create((dtor)destroy_pool, 0);
	getopt_ex(argc, argv, _options, cmdopt, pools);
	if( num_dirs == 0 )
		scan(".", checkfilebysize, pools);
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
	int grpIndex = 1;
	if( ! queue_enum(pools, pfn_printpool, &grpIndex) )
		fprintf(stderr, "No duplicates found.\n");
	queue_destroy(pools);
	set_destroy(visited);
	queue_destroy(ignore);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////

