#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <system_error>
#include <cerrno>

#include "image.h"
#include "memutil.h"

using namespace std;

RGB24BitImage::RGB24BitImage(PNG & png)
{
    uint8_t * srcData;
    uint32_t srcDataLength;
    
    this->_width = png.getWidth();
    this->_height = png.getHeight();

    png.getImageData(&srcData, &srcDataLength);
    
    if (this->getFormat() != PNGImage) {
        transformImageData(srcData, srcDataLength);
    }
    else {
        copyImageData(srcData, srcDataLength);
    }
}

RGB24BitImage::RGB24BitImage(Bitmap & bmp)
{
    uint8_t * srcData;
    uint32_t srcDataLength;
    
    this->_width = bmp.getWidth();
    this->_height = bmp.getHeight();

    bmp.getImageData(&srcData, &srcDataLength);
    
    if (this->getFormat() != BitmapImage) {
        transformImageData(srcData, srcDataLength);
    }
    else {
        copyImageData(srcData, srcDataLength);
    }
}

RGB24BitImage::RGB24BitImage(uint8_t * data, uint32_t dataLength, uint32_t width, uint32_t height)
{
    this->_pImageData = data;
    this->_dataLength = dataLength;
    this->_width = width;
    this->_height = height;
}

void RGB24BitImage::copyImageData(uint8_t * srcData, uint32_t srcDataLength)
{
    if (this->_pImageData != NULL) {
        memclr(this->_pImageData, this->_dataLength);
        free(_pImageData);
        this->_dataLength = 0;
    }

    this->_pImageData = srcData;
    this->_dataLength = srcDataLength;
}

void RGB24BitImage::transformImageData(uint8_t * srcData, uint32_t srcDataLength)
{
	long		i;
	long		x;
	long		y;
	long		rowBytes;
	uint8_t *	sourceRow;
	uint8_t *	targetRow;
	uint8_t **	rows;

    if (this->_pImageData != NULL) {
        memclr(this->_pImageData, this->_dataLength);
        free(_pImageData);
        this->_dataLength = 0;
    }
    
    /*
    ** Bitmap data is encoded in rows left->right with the rows
    ** encoded from bottom to top. Each pixel is encoded with
    ** 3 channels (RGB) but in BGR order.
    **
    ** PNG data is encoded in rows left->right with the rows
    ** encoded from top to bottom. Each pixel is encoded with
    ** 3 channels (RGB), in RGB order.
    */
    rowBytes = getWidth() * 3;

    i = 0L;

    rows = (uint8_t **)malloc(getHeight());

    for (y = getHeight() - 1;y >= 0L;--y) {
        sourceRow = (uint8_t *)malloc(rowBytes);

        rows[y] = sourceRow;

        for (x = 0L;x < rowBytes;x += 3) {
            sourceRow[x+2] = *srcData++; //Blue/Red
            sourceRow[x+1] = *srcData++; //Green
            sourceRow[x]   = *srcData++; //Red/Blue

            i += 3;
        }
    }

    this->_pImageData = (uint8_t *)malloc(srcDataLength);
    this->_dataLength = srcDataLength;

    i = 0L;

    for (y = 0L;y < getHeight();y++) {
        targetRow = rows[y];

        for (x = 0L;x < rowBytes;x++) {
            this->_pImageData[i++] = *targetRow++;
        }

        free(rows[y]);
    }

    free(rows);
}

void PNG::getHeader(uint8_t ** data, uint32_t * dataLength)
{
    throw new system_error(make_error_code(errc::not_supported));
}

void Bitmap::getHeader(uint8_t ** header, uint32_t * headerLen)
{
    BitmapHeader    bitmapHeader;
    WinV3Header     infoHeader;
    uint8_t *       buffer;

    if (type == WindowsV3) {
        *headerLen = BMP_HEADER_SIZE + WINV3_HEADER_SIZE;
        *header = (uint8_t *)malloc(*headerLen);

        if (*header == NULL) {
            throw new system_error(make_error_code(errc::not_enough_memory));
        }

        memclr(&bitmapHeader, sizeof(BitmapHeader));
        memclr(&infoHeader, sizeof(WinV3Header));

        buffer = *header;

        bitmapHeader.magicString[0] = 'B';
        bitmapHeader.magicString[1] = 'M';
        infoHeader.dataLength = this->getDataLength();
        infoHeader.headerSize = WINV3_HEADER_SIZE;
        bitmapHeader.fileLength = infoHeader.dataLength + BMP_HEADER_SIZE + infoHeader.headerSize;
        bitmapHeader.startOffset = BMP_HEADER_SIZE + infoHeader.headerSize;
        infoHeader.width = this->getWidth();
        infoHeader.height = this->getHeight();
        infoHeader.bitsPerPixel = 24;

        memcpy(buffer, &bitmapHeader, sizeof(BitmapHeader));
        memcpy(&buffer[BMP_HEADER_SIZE], &infoHeader, sizeof(WinV3Header));
    }
    else {
        throw new system_error(make_error_code(errc::not_supported));
    }
}