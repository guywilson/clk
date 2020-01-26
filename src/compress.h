#include <stdint.h>

#include "datafile.h"

#ifndef _INCL_COMPRESS
#define _INCL_COMPRESS

class CompressionHelper
{
    public:
        CompressionHelper() {}

        LengthEncodedDataFile * compress(DataFile * src);
        LengthEncodedDataFile * compress(DataFile * src, int compressionLevel);

        LengthEncodedDataFile * inflate(DataFile * src, uint32_t outputDataLength);
};

#endif
