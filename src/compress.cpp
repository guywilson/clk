#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "clk_error.h"
#include "datafile.h"
#include "compress.h"

using namespace std;

DataFile * CompressionHelper::compress(DataFile * src)
{
    return compress(src, 5);
}

DataFile * CompressionHelper::compress(DataFile * src, int compressionLevel)
{  
    DataFile *              compressedDataFile;
    uint32_t                compressedDataLength;
    uint8_t *               compressedData;
    uint32_t                srcDataLength;
    uint8_t *               srcData;
    int                     rtn;

    src->getData(&srcData, &srcDataLength);

    if (compressionLevel > 0) {
        compressedDataLength = compressBound(srcDataLength);

        compressedData = (uint8_t *)malloc(compressedDataLength);

        if (compressedData == NULL) {
            throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
        }

        rtn = compress2(
                    compressedData,
                    (unsigned long *)&compressedDataLength,
                    srcData,
                    srcDataLength,
                    compressionLevel);

        if (rtn != Z_OK) {
            throw clk_error(clk_error::buildMsg("Failed to compress data - %d", rtn), __FILE__, __LINE__);
        }
    }
    else {
        compressedDataLength = srcDataLength;

        compressedData = (uint8_t *)malloc(compressedDataLength);

        if (compressedData == NULL) {
            throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
        }
        
        memcpy(compressedData, srcData, srcDataLength);
    }

    compressedDataFile = new DataFile(compressedData, compressedDataLength);

    return compressedDataFile;
}

DataFile * CompressionHelper::inflate(DataFile * src, uint32_t outputDataLength)
{
    DataFile *              inflatedDataFile;
    uint8_t *               inflatedData;
    uint32_t                srcDataLength;
    uint8_t *               srcData;
    int                     rtn;

    src->getData(&srcData, &srcDataLength);

    inflatedData = (uint8_t *)malloc(outputDataLength);

    if (inflatedData == NULL) {
        throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
    }

    rtn = uncompress(
                inflatedData,
                (unsigned long *)&outputDataLength,
                srcData,
                srcDataLength);

    if (rtn != Z_OK) {
        throw clk_error(clk_error::buildMsg("Failed to inflate data - %d", rtn), __FILE__, __LINE__);
    }

    inflatedDataFile = new DataFile(inflatedData, outputDataLength);

    return inflatedDataFile;
}
