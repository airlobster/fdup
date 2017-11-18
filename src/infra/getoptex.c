// getoptex.c

#include <stdlib.h>
#include <string.h>
#include "getoptex.h"


void getopt_ex(int argc, char* const argv[], const struct option* options, void(*f)(int c, char* const arg, void* ctx), void* ctx) {
	const struct option* o=0;
	int n=0;
	char* optstr=0;
	int c;

	// count options
	for(o=options, n=0; o->val; ++n, ++o)
		;

	// build opt-string
	optstr = (char*)malloc(n*2+1);
	*optstr = 0;

	for(o=options; o->val; ++o) {
		strncat(optstr, (const char*)&o->val, 1);
		if( o->has_arg )
			strcat(optstr, ":");
	}

	// iterate and notify
	while( (c=getopt_long(argc, argv, optstr, options, 0)) != -1 ) {
		f(c, optarg, ctx);
	}

	free(optstr);
}

