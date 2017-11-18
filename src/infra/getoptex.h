// getoptex.h

#ifndef __GETOPTEX_H_
#define	__GETOPTEX_H_

#include <getopt.h>


#define	GETOPT_BEGIN(varname)		static const struct option varname[] = {
#define	GETOPT_END()			{0,0,0,0}};
#define	GETOPT_OPT(v,name,has_arg)	{(name),(has_arg),0,(v)},


void getopt_ex(int argc, char* const argv[], const struct option* options, void(*f)(int c, char* const arg, void* ctx), void* ctx);


#endif


