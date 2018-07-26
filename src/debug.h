#include <stdio.h>

#define DEBUG 3

#if defined(DEBUG) && DEBUG > 0
#define debug(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): \t" fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define debug(fmt, args...) /* Don't do anything in release builds */
#endif