#ifndef __CONFIG__
#include <stdio.h>

#define DEBUG 1
#define MAXREQUESTLEN 50000
#define MAXLISTENNUM 5
#define MaxResponseLen 5000

#if (DEBUG > 0)
#define debug_print(...) printf(__VA_ARGS__)
#else
#define debug_print(...) ;
#endif

#endif
