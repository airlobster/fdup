// utils.c

#include "utils.h"

static int verbose = 0;


void set_verbose(int v) {
	verbose = v;
}

int get_verbose() {
	return verbose;
}

