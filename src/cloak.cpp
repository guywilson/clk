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
    uint32_t            imageCapacity;
    uint8_t             mask;
    uint8_t             dataBits;
    int                 i, pos, bitCounter, numLengthBytes, numDataBytes;

    srcImageData = srcImage->getImageData();
    srcImageDataLength = srcImage->getDataLength();

    secretData = srcDataFile->getData();
    secretDataLength = srcDataFile->getDataLength();

    numLengthBytes = sizeof(uint32_t) * (8 / bitsPerByte);
    numDataBytes = secretDataLength * (8 / bitsPerByte);

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

    memcpy(targetImageData, srcImageData, srcImageDataLength);

    mask = getBitMask(bitsPerByte);

//    printf("Secret data length = %u [0x%04X], mask = 0x%02X\n", secretDataLength, secretDataLength, mask);

    bitCounter = 0;

    for (i = 0;i < numLengthBytes;i++) {
        dataBits = (uint8_t)((secretDataLength >> bitCounter) & mask);
        targetImageData[i] = (srcImageData[i] & ~mask) | dataBits;
//        printf("data bits = 0x%02X, target image byte = 0x%02X, source image byte = 0x%02X\n", dataBits, targetImageData[i], srcImageData[i]);

        bitCounter += bitsPerByte;
    }

    bitCounter = 0;
    pos = 0;

    for (i = numLengthBytes;i < (numDataBytes + numLengthBytes);i++) {
        dataBits = (secretData[pos] >> bitCounter) & mask;
        targetImageData[i] = (srcImageData[i] & ~mask) | dataBits;

//        printf("data bits = %X, target image byte = 0x%02X, source image byte = 0x%02X\n", dataBits, targetImageData[i], srcImageData[i]);

        bitCounter += bitsPerByte;

        if (bitCounter == 8) {
            bitCounter = 0;
            pos++;
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
    uint8_t             dataLengthBuffer[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t             targetBits;
    uint8_t             targetByte;
    int                 i, bitCounter, pos, numDataLengthBytes, numDataBytes;

    srcImageData = srcImage->getImageData();
    srcImageDataLength = srcImage->getDataLength();

    mask = getBitMask(bitsPerByte);

    bitCounter = 0;
    pos = 0;
    targetBits = 0x00;
    targetByte = 0x00;

    numDataLengthBytes = 4 * 8 / bitsPerByte;

    for (i = 0;i < numDataLengthBytes;i++) {
        targetBits = srcImageData[i] & mask;
        targetByte += targetBits << bitCounter;

//        printf("Image byte = 0x%02X, buffer[%d] = 0x%02X\n", srcImageData[i], pos, targetByte);

        bitCounter += bitsPerByte;

        if (bitCounter == 8) {
            bitCounter = 0;

            dataLengthBuffer[pos++] = targetByte;
            targetByte = 0x00;
        }
    }

    memcpy(&targetDataLength, dataLengthBuffer, 4);

//    printf("Target data length = %u\n", targetDataLength);

    targetData = (uint8_t *)malloc(targetDataLength);

    if (targetData == NULL) {
        throw clk_error("Failed to allocate memory for target data file", __FILE__, __LINE__);
    }

    bitCounter = 0;
    pos = 0;
    targetBits = 0x00;
    targetByte = 0x00;

    numDataBytes = targetDataLength * (8 / bitsPerByte);

    for (i = numDataLengthBytes;i < (numDataBytes + numDataLengthBytes);i++) {
        targetBits = srcImageData[i] & mask;
        targetByte += targetBits << bitCounter;

//        printf("src image byte = 0x%02X, target data[%d] = 0x%02X\n", srcImageData[i], pos, targetByte);

        bitCounter += bitsPerByte;

        if (bitCounter == 8) {
            bitCounter = 0;

            targetData[pos++] = targetByte;
            targetByte = 0x00;
        }
    }

    targetDataFile = new DataFile(targetData, targetDataLength);

    return targetDataFile;
}
