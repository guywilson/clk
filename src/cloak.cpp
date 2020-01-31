#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "clk_error.h"
#include "passwordmgr.h"
#include "crc32.h"
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
RGB24BitImage * CloakHelper::merge(RGB24BitImage * srcImage, DataFile * srcDataFile, clk_info_struct * infoStruct, MergeQuality bitsPerByte)
{
    RGB24BitImage *     targetImage;
    uint8_t *           srcImageData;
    uint32_t            srcImageDataLength;
    uint8_t *           targetImageData;
    uint32_t            targetImageDataLength;
    uint8_t *           secretData;
    uint32_t            secretDataLength;
    uint8_t             infoStructBuffer[sizeof(clk_info_struct)];
    uint8_t             xorBuffer[sizeof(uint32_t) * 3];
    uint8_t             xorKey[128 / 8];
    uint32_t            imageCapacity;
    uint32_t            width;
    uint32_t            height;
    uint8_t             mask;
    uint8_t             dataBits;
    int                 i, pos, bitCounter, numLengthBytes, numDataBytes;

    srcImageData = srcImage->getImageData();
    srcImageDataLength = srcImage->getDataLength();

    secretData = srcDataFile->getData();
    secretDataLength = srcDataFile->getDataLength();

    numLengthBytes = sizeof(clk_info_struct) * (8 / bitsPerByte);
    numDataBytes = secretDataLength * (8 / bitsPerByte);

    width = srcImage->getWidth();
    height = srcImage->getHeight();

    /*
    ** Copy the image data len + width + height (12 bytes total)
    ** to xorBuffer...
    */
    memcpy(&xorBuffer[0], &srcImageDataLength, sizeof(uint32_t));
    memcpy(&xorBuffer[4], &width, sizeof(uint32_t));
    memcpy(&xorBuffer[8], &height, sizeof(uint32_t));

    /*
    ** Get the 128-bit (16 byte) hash of the xor buffer...
    */
    PasswordManager mgr(PasswordManager::Low);
    mgr.getKey(xorKey, xorBuffer, sizeof(xorBuffer));

    /*
    ** Pop the length struct into a buffer...
    */
    memcpy(infoStructBuffer,infoStruct, sizeof(clk_info_struct));

    /*
    ** XOR the length struct with the first n bytes of the key,
    ** n = sizeof the length struct...
    */
    for (i = 0;i < sizeof(clk_info_struct);i++) {
        infoStructBuffer[i] = infoStructBuffer[i] ^ xorKey[i];
    }

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

    bitCounter = 0;
    pos = 0;

    for (i = 0;i < numLengthBytes;i++) {
        dataBits = (infoStructBuffer[pos] >> bitCounter) & mask;
        targetImageData[i] = (srcImageData[i] & ~mask) | dataBits;

        bitCounter += bitsPerByte;

        if (bitCounter == 8) {
            bitCounter = 0;
            pos++;
        }
    }

    bitCounter = 0;
    pos = 0;

    for (i = numLengthBytes;i < (numDataBytes + numLengthBytes);i++) {
        dataBits = (secretData[pos] >> bitCounter) & mask;
        targetImageData[i] = (srcImageData[i] & ~mask) | dataBits;

        bitCounter += bitsPerByte;

        if (bitCounter == 8) {
            bitCounter = 0;
            pos++;
        }
    }

    if (srcImage->getFormat() == PNGImage) {
        targetImage = new PNG(targetImageData, targetImageDataLength, width, height);
    }
    else {
        targetImage = new Bitmap(targetImageData, targetImageDataLength, width, height);
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
DataFile * CloakHelper::extract(RGB24BitImage * srcImage, clk_info_struct * infoStruct, MergeQuality bitsPerByte)
{
    DataFile *              targetDataFile;
    uint8_t *               srcImageData;
    uint32_t                srcImageDataLength;
    uint8_t *               targetData;
    uint32_t                targetDataLength;
    uint32_t                width;
    uint32_t                height;
    uint8_t                 mask;
    uint8_t                 infoStructBuffer[sizeof(clk_info_struct)];
    uint8_t                 xorBuffer[sizeof(uint32_t) * 3];
    uint8_t                 xorKey[128 / 8];
    uint8_t                 targetBits;
    uint8_t                 targetByte;
    int                     i, bitCounter, pos, numLengthBytes, numDataBytes;

    srcImageData = srcImage->getImageData();
    srcImageDataLength = srcImage->getDataLength();

    mask = getBitMask(bitsPerByte);

    bitCounter = 0;
    pos = 0;
    targetBits = 0x00;
    targetByte = 0x00;

    numLengthBytes = sizeof(clk_info_struct) * (8 / bitsPerByte);

    width = srcImage->getWidth();
    height = srcImage->getHeight();

    memset(infoStructBuffer, 0, sizeof(clk_info_struct));

    for (i = 0;i < numLengthBytes;i++) {
        targetBits = srcImageData[i] & mask;
        targetByte += targetBits << bitCounter;

        bitCounter += bitsPerByte;

        if (bitCounter == 8) {
            bitCounter = 0;

            infoStructBuffer[pos++] = targetByte;
            targetByte = 0x00;
        }
    }

    /*
    ** Copy the image data len + width + height (12 bytes total)
    ** to xorBuffer...
    */
    memcpy(&xorBuffer[0], &srcImageDataLength, sizeof(uint32_t));
    memcpy(&xorBuffer[4], &width, sizeof(uint32_t));
    memcpy(&xorBuffer[8], &height, sizeof(uint32_t));

    /*
    ** Get the 128-bit (16 byte) hash of the xor buffer...
    */
    PasswordManager mgr(PasswordManager::Low);
    mgr.getKey(xorKey, xorBuffer, sizeof(xorBuffer));

    /*
    ** XOR the length struct with the first 12 bytes of the key...
    */
    for (i = 0;i < sizeof(clk_info_struct);i++) {
        infoStructBuffer[i] = infoStructBuffer[i] ^ xorKey[i];
    }

    /*
    ** Get the length struct out of the buffer...
    */
    memcpy(infoStruct, infoStructBuffer, sizeof(clk_info_struct));

    targetDataLength = infoStruct->encryptedLength;

    targetData = (uint8_t *)malloc(targetDataLength);

    if (targetData == NULL) {
        throw clk_error("Failed to allocate memory for target data file", __FILE__, __LINE__);
    }

    bitCounter = 0;
    pos = 0;
    targetBits = 0x00;
    targetByte = 0x00;

    numDataBytes = targetDataLength * (8 / bitsPerByte);

    for (i = numLengthBytes;i < (numDataBytes + numLengthBytes);i++) {
        targetBits = srcImageData[i] & mask;
        targetByte += targetBits << bitCounter;

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
