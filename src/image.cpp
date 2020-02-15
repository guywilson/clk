#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "clk_error.h"
#include "image.h"
#include "memutil.h"

using namespace std;

RGB24BitImage::RGB24BitImage(RGB24BitImage & src) : RGB24BitImage(&src)
{
}

RGB24BitImage::RGB24BitImage(RGB24BitImage * src)
{
    uint8_t * srcData;
    uint32_t srcDataLength;
    
    this->_width = src->getWidth();
    this->_height = src->getHeight();

    srcData = src->getImageData();
    srcDataLength = src->getDataLength();

    if (this->getFormat() != src->getFormat()) {
        transformImageData(srcData, srcDataLength);
    }
    else {
        copyImageData(srcData, srcDataLength);
    }
}
 
RGB24BitImage::RGB24BitImage(PNG & src) : RGB24BitImage(&src)
{
}

RGB24BitImage::RGB24BitImage(PNG * src)
{
    uint8_t * srcData;
    uint32_t srcDataLength;
    
    this->_width = src->getWidth();
    this->_height = src->getHeight();

    srcData = src->getImageData();
    srcDataLength = src->getDataLength();

    if (this->getFormat() != src->getFormat()) {
        transformImageData(srcData, srcDataLength);
    }
    else {
        copyImageData(srcData, srcDataLength);
    }
}

RGB24BitImage::RGB24BitImage(Bitmap & src) : RGB24BitImage(&src)
{
}

RGB24BitImage::RGB24BitImage(Bitmap * src)
{
    uint8_t * srcData;
    uint32_t srcDataLength;
    
    this->_width = src->getWidth();
    this->_height = src->getHeight();

    srcData = src->getImageData();
    srcDataLength = src->getDataLength();

    if (this->getFormat() != src->getFormat()) {
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

    this->_isCopied = false;
}

RGB24BitImage::~RGB24BitImage()
{
    if (this->_pImageData != NULL) {
        memclr(this->_pImageData, this->_dataLength);
        free(this->_pImageData);
        this->_dataLength = 0;
    }
}

void RGB24BitImage::copyImageData(uint8_t * srcData, uint32_t srcDataLength)
{
    if (this->_pImageData != NULL) {
        memclr(this->_pImageData, this->_dataLength);
        free(_pImageData);
        this->_dataLength = 0;
    }

    memcpy(this->_pImageData, srcData, srcDataLength);

    this->_dataLength = srcDataLength;

    this->_isCopied = true;
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

    rows = (uint8_t **)malloc(getHeight() * sizeof(uint8_t *));

    if (rows == NULL) {
        throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
    }

    for (y = getHeight() - 1;y >= 0L;--y) {
        sourceRow = (uint8_t *)malloc(rowBytes);

        if (sourceRow == NULL) {
            throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
        }

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

    this->_isCopied = true;
}

uint8_t * PNG::getHeader()
{
    throw clk_error("Called unimplemented method", __FILE__, __LINE__);
}

uint32_t PNG::getHeaderLength()
{
    throw clk_error("Called unimplemented method", __FILE__, __LINE__);
}

uint8_t * Bitmap::getHeader()
{
    BitmapHeader    bitmapHeader;
    WinV3Header     infoHeader;
    uint8_t *       buffer;

    if (type == WindowsV3) {
        buffer = (uint8_t *)malloc(getHeaderLength());

        if (buffer == NULL) {
            throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
        }

        memclr(&bitmapHeader, sizeof(BitmapHeader));
        memclr(&infoHeader, sizeof(WinV3Header));

        infoHeader.dataLength = this->getDataLength();
        infoHeader.headerSize = WINV3_HEADER_SIZE;
        infoHeader.width = this->getWidth();
        infoHeader.height = this->getHeight();
        infoHeader.bitsPerPixel = 24;
        infoHeader.horizontalResolution = 300;
        infoHeader.verticalResolution = 300;
        infoHeader.colourPlanes = 1;

        bitmapHeader.magicString[0] = 'B';
        bitmapHeader.magicString[1] = 'M';
        bitmapHeader.fileLength = infoHeader.dataLength + BMP_HEADER_SIZE + infoHeader.headerSize;
        bitmapHeader.startOffset = BMP_HEADER_SIZE + infoHeader.headerSize;

        memcpy(buffer, &bitmapHeader, sizeof(BitmapHeader));
        memcpy(&buffer[BMP_HEADER_SIZE], &infoHeader, sizeof(WinV3Header));
    }
    else {
        throw clk_error("Invalid bitmap format", __FILE__, __LINE__);
    }

    return buffer;
}

uint32_t Bitmap::getHeaderLength()
{
    return sizeof(BitmapHeader) + sizeof(WinV3Header);
}