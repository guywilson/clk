#include <stdint.h>

#ifndef _INCL_IMAGE

class RGB24BitImage
{
    private:
        uint8_t *       _pImageData;
        uint32_t        _dataLength;

        uint32_t        _width;
        uint32_t        _height;

        bool            _isCompressed;

    protected:

    public:
        RGB24BitImage();
        RGB24BitImage(uint32_t width, uint32_t height);
        RGB24BitImage(uint8_t * data, uint32_t dataLength, bool isCompressed, uint32_t width, uint32_t height);

        uint32_t        getWidth() {
            return _width;
        }
        uint32_t        getHeight() {
            return _height;
        }
        bool            isCompressed() {
            return  _isCompressed;
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
        PNG();
        PNG(uint32_t width, uint32_t height);
        PNG(uint8_t * data, uint32_t dataLength, uint32_t width, uint32_t height);
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
        Bitmap();
        Bitmap(uint32_t width, uint32_t height);
        Bitmap(uint8_t * data, uint32_t dataLength, uint32_t width, uint32_t height);
};

#endif
