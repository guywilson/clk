#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "memutil.h"

void memclr(void * src, size_t len)
{
    int i;

    for (i = 0;i < NUM_CLR_CYCLES; i++) {
        memset(src, 0x00, len);
        memset(src, 0xFF, len);
    }

    memset(src, 0x00, len);
}

void * malloc_s(size_t len)
{
    void *      mem;

    mem = malloc(len);

    if (mem != NULL) {
        memclr(mem, len);
    }

    return mem;
}

void free_s(void * mem, size_t len)
{
    if (mem != NULL) {
        memclr(mem, len);

        free(mem);
    }

    mem = NULL;
}
