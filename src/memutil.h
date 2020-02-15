#include <stddef.h>

#ifndef _INCL_MEMUTIL
#define _INCL_MEMUTIL

#define NUM_CLR_CYCLES              7

#ifdef __cplusplus
extern "C" {
void memclr(void * src, size_t len);
}
#else
void memclr(void * src, size_t len);
#endif

#endif
