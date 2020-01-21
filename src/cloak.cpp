#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "clk_error.h"
#include "cloak.h"
#include "image.h"
#include "datafile.h"

/******************************************************************************
**
** Hide a file in the source image...
**
** 1.   Store the encoded data length in the first n bytes of image data, 
**      encoded in 1, 2 or 4 bites per byte.
** 2.   Then store the file data as 1, 2, or 4 bits per byte depending on the
**      value supplied.
**
******************************************************************************/
RGB24BitImage * CloakHelper::merge(RGB24BitImage * srcImage, DataFile * srcDataFile, MergeQuality bitsPerByte)
{
    RGB24BitImage *     targetImage;
    uint8_t *           srcImageData;
    uint32_t            srcImageDataLength;
    uint8_t *           targetImageData;
    uint32_t            targetImageDataLength;
    uint8_t *           secretData;
    uint32_t            secretDataLength;
    uint8_t             secretDataLengthBuffer[sizeof(uint32_t)];
    uint32_t            imageCapacity;
    uint8_t             mask;
    uint8_t             dataBits;
    int                 i, bitCounter, numLengthBytes;

    srcImageData = srcImage->getImageData();
    srcImageDataLength = srcImage->getDataLength();

    secretData = srcDataFile->getData();
    secretDataLength = srcDataFile->getDataLength();

    numLengthBytes = sizeof(uint32_t) * (8 / bitsPerByte);

    imageCapacity = (srcImageDataLength / (8 / bitsPerByte) - numLengthBytes);

    if (srcImageDataLength < ((secretDataLength + numLengthBytes) * (8 / bitsPerByte))) {
        throw clk_error(
            clk_error::buildMsg(
                "Insufficient space in image, maximum capacity is %u bytes, secret file is %u bytes long.", 
                imageCapacity, 
                secretDataLength), 
            __FILE__, 
            __LINE__);
    }

    targetImageDataLength = srcImageDataLength;

    targetImageData = (uint8_t *)malloc(targetImageDataLength);

    if (targetImageData == NULL) {
        throw clk_error("Failed to allocate memory for target image", __FILE__, __LINE__);
    }

    mask = getBitMask(bitsPerByte);

    memcpy(secretDataLengthBuffer, &secretDataLength, sizeof(uint32_t));

    for (i = 0;i < numLengthBytes;i++) {
        for (bitCounter = 0;bitCounter < 8;bitCounter += bitsPerByte) {
            dataBits = (secretDataLengthBuffer[i] >> bitCounter) & mask;
            targetImageData[i] = (srcImageData[i] & ~mask) | dataBits;
        }
    }

    for (i = 0;i < secretDataLength;i++) {
        for (bitCounter = 0;bitCounter < 8;bitCounter += bitsPerByte) {
            dataBits = (secretData[i] >> bitCounter) & mask;
            targetImageData[i] = (srcImageData[i] & ~mask) | dataBits;
        }
    }

    if (srcImage->getFormat() == PNGImage) {
        targetImage = new PNG(targetImageData, targetImageDataLength, srcImage->getWidth(), srcImage->getHeight());
    }
    else {
        targetImage = new Bitmap(targetImageData, targetImageDataLength, srcImage->getWidth(), srcImage->getHeight());
    }

    return targetImage;
}

/******************************************************************************
**
** Extract a file hidden in the source image...
**
** 1.   Extract the encoded data length, this is stored in the first n bytes
**      of image data, encoded in 1, 2 or 4 bites per byte.
** 2.   Use this data length to allocate enough space to fit the encoded
**      data in.
**
******************************************************************************/
DataFile * CloakHelper::extract(RGB24BitImage * srcImage, MergeQuality bitsPerByte)
{
    DataFile *          targetDataFile;
    uint8_t *           srcImageData;
    uint32_t            srcImageDataLength;
    uint8_t *           targetData;
    uint32_t            targetDataLength;
    uint8_t             mask;
    uint8_t             dataBits;
    int                 i, bitCounter;

    srcImageData = srcImage->getImageData();
    srcImageDataLength = srcImage->getDataLength();

    targetDataLength = srcImageDataLength;

    targetData = (uint8_t *)malloc(targetDataLength);

    if (targetData == NULL) {
        throw clk_error("Failed to allocate memory for target data file", __FILE__, __LINE__);
    }

    mask = getBitMask(bitsPerByte);

    // for (i = 0;i < secretDataLength;i++) {
    //     for (bitCounter = 0;bitCounter < 8;bitCounter += bitsPerByte) {
    //         dataBits = (secretData[i] >> bitCounter) & mask;
    //         targetImageData[i] = (srcImageData[i] & ~mask) | dataBits;
    //     }
    // }

    targetDataFile = new DataFile(targetData, targetDataLength);

    return targetDataFile;
}
