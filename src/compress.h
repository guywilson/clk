#include <stdint.h>

#include "datafile.h"

#ifndef _INCL_COMPRESS
#define _INCL_COMPRESS

class CompressionHelper
{
    public:
        CompressionHelper();

        DataFile *      compress(DataFile & src);
        DataFile *      inflate(DataFile & src);
};

#endif
