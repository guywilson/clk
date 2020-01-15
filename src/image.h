#include <stdint.h>

#ifndef _INCL_IMAGE
#define _INCL_IMAGE

/*
** Header sizes
*/
#define BMP_HEADER_SIZE		14
#define WINV3_HEADER_SIZE   40

/*
** Bitmap compression methods
*/
#define COMPRESSION_BI_RGB          0x00    // None - default
#define COMPRESSION_BI_RLE8         0X01    // RLE 8-bit/pixel
#define COMPRESSION_BI_RLE4         0x02    // RLE 4-bit/pixel
#define COMPRESSION_BI_BITFIELDS    0x03    // Bit field
#define COMPRESSION_BI_JPEG         0x04    // JPEG
#define COMPRESSION_BI_PNG          0x05    // PNG

typedef struct {
    char        magicString[2] = {'B', 'M'};
    uint32_t    fileLength;
    uint16_t    reserved[2] = {0, 0};
    uint32_t    startOffset;
}
BitmapHeader;

typedef struct {
    uint32_t    headerSize;
    uint32_t    width;
    uint32_t    height;
    uint16_t    colourPlanes;
    uint16_t    bitsPerPixel = 24;
    uint32_t    compressionMethod;
    uint32_t    dataLength;
    uint32_t    horizontalResolution;
    uint32_t    verticalResolution;
    uint32_t    numColours;
    uint32_t    numImportantColours;
}
WinV3Header;

class RGB24BitImage;
class PNG;
class Bitmap;

enum ImageFormat {
    PNGImage,
    BitmapImage,
    UnsupportedFormat
};

class RGB24BitImage
{
    private:
        uint8_t *       _pImageData;
        uint32_t        _dataLength;

        uint32_t        _width;
        uint32_t        _height;

        bool            _isCopied;

    protected:
        void            copyImageData(uint8_t * srcData, uint32_t srcDataLength);
        void            transformImageData(uint8_t * srcData, uint32_t srcDataLength);

        bool            isCopied() {
            return this->_isCopied;
        }

    public:
        RGB24BitImage(PNG & png);
        RGB24BitImage(Bitmap & bmp);
        RGB24BitImage(uint8_t * data, uint32_t dataLength, uint32_t width, uint32_t height);

        ~RGB24BitImage();

        uint32_t        getWidth() {
            return _width;
        }
        uint32_t        getHeight() {
            return _height;
        }

        virtual ImageFormat getFormat() {
            return UnsupportedFormat;
        }

        uint32_t        getDataLength() {
            return this->_dataLength;
        }

        virtual void getHeader(uint8_t ** header, uint32_t * headerLen) = 0;

        void getImageData(uint8_t ** data, uint32_t * dataLength) {
            *data = this->_pImageData;
            *dataLength = this->_dataLength;
        }
};

enum PNGFormat {
    RGB = 0x02,
    RGB_alpha = 0x06,
    Grayscale = 0x00,
    Grayscale_alpha = 0x04,
    Palette = 0x03,
    Unsupported = 0xFF
};

class PNG : public RGB24BitImage
{
    private:
        int             compressionLevel;
        PNGFormat       format;

    public:
        PNG(PNG & src) : RGB24BitImage(src) {}
        PNG(Bitmap & src) : RGB24BitImage(src) {}
        PNG(uint8_t * data, uint32_t dataLength, uint32_t width, uint32_t height) : RGB24BitImage(data, dataLength, width, height) {}

        virtual ImageFormat getFormat() {
            return PNGImage;
        }

        int getCompressionLevel() {
            return this->compressionLevel;
        }

        PNGFormat getPNGFormat() {
            return this->format;
        }

        virtual void getHeader(uint8_t ** header, uint32_t * headerLen);
};

enum BitmapType {
    WindowsV3, 
    UnknownType
};

class Bitmap : public RGB24BitImage
{
    private:
        BitmapType      type;

    public:
        Bitmap(Bitmap & src) : RGB24BitImage(src) {}
        Bitmap(PNG & src) : RGB24BitImage(src) {}
        Bitmap(uint8_t * data, uint32_t dataLength, uint32_t width, uint32_t height) : RGB24BitImage(data, dataLength, width, height) {}

        void setType(BitmapType t) {
            this->type = t;
        }

        virtual ImageFormat getFormat() {
            return BitmapImage;
        }

        virtual void getHeader(uint8_t ** header, uint32_t * headerLen);
};

#endif
