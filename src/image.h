#include <stdint.h>

#ifndef _INCL_IMAGE
#define _INCL_IMAGE

/*
** Header sizes
*/
#define BMP_HEADER_SIZE		14
#define WINV3_HEADER_SIZE   40
#define OS2V1_HEADER_SIZE   12

/*
** Bitmap compression methods
*/
#define COMPRESSION_BI_RGB          0x00    // None - default
#define COMPRESSION_BI_RLE8         0X01    // RLE 8-bit/pixel
#define COMPRESSION_BI_RLE4         0x02    // RLE 4-bit/pixel
#define COMPRESSION_BI_BITFIELDS    0x03    // Bit field
#define COMPRESSION_BI_JPEG         0x04    // JPEG
#define COMPRESSION_BI_PNG          0x05    // PNG

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

    protected:
        uint32_t        getDataLength() {
            return this->_dataLength;
        }
        
    public:
        RGB24BitImage(uint8_t * data, uint32_t dataLength, uint32_t width, uint32_t height) {
            this->_pImageData = data;
            this->_dataLength = dataLength;
            this->_width = width;
            this->_height = height;
        }

        uint32_t        getWidth() {
            return _width;
        }
        uint32_t        getHeight() {
            return _height;
        }

        virtual ImageFormat getFormat() {
            return UnsupportedFormat;
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
        PNG(uint8_t * data, uint32_t dataLength, uint32_t width, uint32_t height) : RGB24BitImage(data, dataLength, width, height) {}

        virtual ImageFormat getFormat() {
            return PNGImage;
        }

        virtual void getHeader(uint8_t ** header, uint32_t * headerLen);
};

enum BitmapType {
    WindowsV3, 
    OS2V1, 
    UnknownType
};

class Bitmap : public RGB24BitImage
{
    private:
        BitmapType      type;

    public:
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
