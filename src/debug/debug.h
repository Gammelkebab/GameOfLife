#include <stdio.h>

#define DEBUG 0

#if defined(DEBUG) && DEBUG >= 4
#define debug4(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): \t" fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define debug4(fmt, args...) /* Don't do anything in release builds */
#endif

#if defined(DEBUG) && DEBUG >= 3
#define debug3(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): \t" fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define debug3(fmt, args...) /* Don't do anything in release builds */
#endif

#if defined(DEBUG) && DEBUG >= 2
#define debug2(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): \t" fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define debug2(fmt, args...) /* Don't do anything in release builds */
#endif

#if defined(DEBUG) && DEBUG >= 1
#define debug1(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): \t" fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define debug1(fmt, args...) /* Don't do anything in release builds */
#endif

#if defined(DEBUG) && DEBUG >= 0
#define debug0(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): \t" fmt, __FILE__, __LINE__, __func__, ##args)
#define debug(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): \t" fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define debug0(fmt, args...) /* Don't do anything in release builds */
#define debug(fmt, args...)  /* Don't do anything in release builds */
#endif
