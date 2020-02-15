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
