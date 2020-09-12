#include <string>
#include <string.h>
#include <stdio.h>

extern "C" {
#include <png.h>
}

#include "clk_error.h"
#include "image.h"
#include "ImageStream.h"
#include "memutil.h"

using namespace std;

typedef struct _png_struct {
    int				sample_depth;
	int				compression_level;
    unsigned long	width;
    unsigned long	height;
    FILE *			outfile;
    jmp_buf			jmpbuf;
}
PNG_INFO;

static void pngreadwrite_error_handler(png_structp png_ptr, png_const_charp msg)
{
	PNG_INFO *		pngInfo;

    /* This function, aside from the extra step of retrieving the "error
     * pointer" (below) and the fact that it exists within the application
     * rather than within libpng, is essentially identical to libpng's
     * default error handler.  The second point is critical:  since both
     * setjmp() and longjmp() are called from the same code, they are
     * guaranteed to have compatible notions of how big a jmp_buf is,
     * regardless of whether _BSD_SOURCE or anything else has (or has not)
     * been defined. */

    fprintf(stderr, "writepng libpng error: %s\n", msg);
    fflush(stderr);

    pngInfo = (PNG_INFO *)png_get_error_ptr(png_ptr);

    if (pngInfo == NULL) {         /* we are completely hosed now */
        fprintf(stderr,
          "writepng severe error:  jmpbuf not recoverable; terminating.\n");
        fflush(stderr);
        exit(99);
    }

    longjmp(pngInfo->jmpbuf, 1);
}

bool ImageStream::isPNG(uint8_t * header, int size)
{
    int         is_png;
    
    is_png = png_sig_cmp(header, 0, size);

    return is_png == 0 ? true : false;
}

bool ImageStream::isBMP(uint8_t * header, int size)
{
    if (header[0] == 'B' && header[1] == 'M') {
        return true;
    }
    else {
        return false;
    }
}

ImageInputStream::ImageInputStream(string & filename) : ImageStream(filename)
{

}

ImageInputStream::ImageInputStream(char * filename) : ImageStream(filename)
{

}

PNG * ImageInputStream::_readPNG()
{
    PNG *           png;
	png_structp		png_ptr;
	png_infop		info_ptr;
	PNG_INFO		pngInfo;
    uint32_t        width;
    uint32_t        height;
    uint32_t        dataLength;
    uint8_t         channels;
    uint8_t *       data;
    uint8_t *       dataStart;
    uint8_t *       row;
    uint8_t *       rowStart;
    uint16_t        bitsPerPixel;
    int             colourType;
    unsigned int    x;
    unsigned int    y;

	png_ptr = png_create_read_struct(
					PNG_LIBPNG_VER_STRING,
					&pngInfo, 
					pngreadwrite_error_handler, 
					NULL);

	if (png_ptr == NULL) {
	  return NULL;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	
    if (info_ptr == NULL) {
	  png_destroy_read_struct(&png_ptr, NULL, NULL);
	  return NULL;
	}

	/* Set error handling if you are using the setjmp/longjmp method (this is
	* the normal method of doing things with libpng).  REQUIRED unless you
	* set up your own error handlers in the png_create_read_struct() earlier.
	*/

	if (setjmp(pngInfo.jmpbuf)) {
	  /* Free all of the memory associated with the png_ptr and info_ptr */
	  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	  /* If we get here, we had a problem reading the file */
	  return NULL;
	}

	/* Set up the input control if you are using standard C streams */
	png_init_io(png_ptr, getFilePtr());
	
	png_read_info(png_ptr, info_ptr);
	
	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
    channels = png_get_channels(png_ptr, info_ptr);
	bitsPerPixel = png_get_bit_depth(png_ptr, info_ptr) * png_get_channels(png_ptr, info_ptr);
	colourType = png_get_color_type(png_ptr, info_ptr);
	
    if (bitsPerPixel != 24) {
        throw clk_error("Image must be 24-bit", __FILE__, __LINE__);
    }
    if (colourType != PNG_COLOR_TYPE_RGB) {
        throw clk_error("Image must be RGB", __FILE__, __LINE__);
    }

    dataLength = width * height * channels;

	data = (uint8_t *)malloc(dataLength);
	
	if (data == NULL) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
	}
	
	dataStart = data;
	
	row = (uint8_t *)malloc(width * channels);
	
	if (row == NULL) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
	}
	
	rowStart = row;
	
	for (y = 0;y < height;++y) {
		row = rowStart;
		
		png_read_row(png_ptr, row, NULL);
		
		for (x = 0;x < width;++x) {
			*data++ = *row++; // Red
			*data++ = *row++; // Green
			*data++ = *row++; // Blue
		}
	}

	png_read_end(png_ptr, NULL);
	
	  /* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    png = new PNG(dataStart, dataLength, width, height);

    return png;
}

Bitmap * ImageInputStream::_readBMP()
{
    BitmapHeader    bmpHeader;
    WinV3Header     dibHeader;
	uint8_t 	    headerBuffer[BMP_HEADER_SIZE];
	uint8_t		    DIBHeaderBuffer[WINV3_HEADER_SIZE];
    uint32_t        DIBHeaderSize;
    uint8_t *       data;
    BitmapType      type;
    Bitmap *        bmp = nullptr;
    size_t          bytesRead;

	/*
	** Read bitmap header...
	*/
	bytesRead = fread(headerBuffer, 1, BMP_HEADER_SIZE, getFilePtr());

    if (bytesRead == BMP_HEADER_SIZE) {
        memcpy(&bmpHeader, headerBuffer, BMP_HEADER_SIZE);
    }
    else {
        throw clk_error(clk_error::buildMsg("Failed to read bitmap header, read %u bytes", bytesRead), __FILE__, __LINE__);
    }
    
	bytesRead = fread(DIBHeaderBuffer, 1, 4, getFilePtr());

    if (bytesRead == 4) {
        DIBHeaderSize = (uint32_t)DIBHeaderBuffer[0];
    }
    else {
        throw clk_error(clk_error::buildMsg("Failed to read DIB header length, read %u bytes", bytesRead), __FILE__, __LINE__);
    }

	bytesRead = fread(&DIBHeaderBuffer[4], 1, DIBHeaderSize - 4, getFilePtr());

    if (bytesRead == (DIBHeaderSize - 4)) {
        if (DIBHeaderSize == WINV3_HEADER_SIZE) {
            type = WindowsV3;

            memcpy(&dibHeader, DIBHeaderBuffer, DIBHeaderSize);
        }
        else {
            throw clk_error("Invalid bitmap type", __FILE__, __LINE__);
        }
    }
    else {
        throw clk_error(clk_error::buildMsg("Failed to read DIB header, read %u bytes", bytesRead), __FILE__, __LINE__);
    }

    if (dibHeader.bitsPerPixel != 24) {
        throw clk_error("Image must be 24-bit", __FILE__, __LINE__);
    }
    if (dibHeader.compressionMethod != COMPRESSION_BI_RGB) {
        throw clk_error("Compressed bitmaps are not supported", __FILE__, __LINE__);
    }

    memclr(headerBuffer, BMP_HEADER_SIZE);
    memclr(DIBHeaderBuffer, DIBHeaderSize);
    memclr(&DIBHeaderSize, 4);

	/*
	** Seek to beginning of bitmap data...
	*/
    if (fseek(getFilePtr(), bmpHeader.startOffset, SEEK_SET)) {
        throw clk_error("Failed to seek to bitmap data", __FILE__, __LINE__);
	}

	data = (uint8_t *)malloc((size_t)dibHeader.dataLength);

	if (data == NULL) {
        throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
	}

	bytesRead = fread(data, 1, dibHeader.dataLength, getFilePtr());

    if (bytesRead == dibHeader.dataLength) {
        bmp = new Bitmap(data, dibHeader.dataLength, dibHeader.width, dibHeader.height);
        bmp->setType(type);
    }
    else {
        throw clk_error(clk_error::buildMsg("Failed to read bitmap data, read %u bytes and expected %u bytes", bytesRead, dibHeader.dataLength), __FILE__, __LINE__);
    }

    return bmp;
}

void ImageInputStream::open()
{
    FILE *      fptr;

    fptr = fopen(getFilename().c_str(), "rb");

    if (fptr == NULL) {
        throw clk_error("Failed to open image file", __FILE__, __LINE__);
    }

    setFilePtr(fptr);
}

RGB24BitImage * ImageInputStream::read()
{
    RGB24BitImage * image;
    uint8_t         headerBuffer[8];
    int             headerBytesRead;

    headerBytesRead = fread(headerBuffer, 1, 8, getFilePtr());
    fseek(getFilePtr(), 0, SEEK_SET);

    if (isPNG(headerBuffer, headerBytesRead)) {
        PNG * png = _readPNG();
        image = png;
    }
    else if (isBMP(headerBuffer, headerBytesRead)) {
        Bitmap * bmp = _readBMP();
        image = bmp;
    }
    else {
        memclr(headerBuffer, 8);
        throw clk_error("Only PNG and BMP files are supported", __FILE__, __LINE__);
    }

    memclr(headerBuffer, 8);

    return image;
}


ImageOutputStream::ImageOutputStream(string & filename) : ImageStream(filename)
{

}

ImageOutputStream::ImageOutputStream(char * filename) : ImageStream(filename)
{

}

void ImageOutputStream::_writePNG(PNG * png)
{
	uint32_t		y;
	uint32_t		x;
	png_structp		png_ptr = NULL;
    png_infop		info_ptr = NULL;
	uint8_t *		row = NULL;
	uint8_t *		rowStart = NULL;
    uint8_t *       data;
    uint32_t        dataLength;
    uint32_t        width;
    uint32_t        height;
	PNG_INFO		pngInfo;

    /* could also replace libpng warning-handler (final NULL), but no need: */
    png_ptr = png_create_write_struct(
					PNG_LIBPNG_VER_STRING, 
					&pngInfo,
					pngreadwrite_error_handler, 
					NULL);
    
	if (!png_ptr) {
        throw clk_error("Failed to create PNG write structure", __FILE__, __LINE__);
    }

    info_ptr = png_create_info_struct(png_ptr);
    
	if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        throw clk_error("Failed to create PNG info structure", __FILE__, __LINE__);
    }

    /* make sure outfile is (re)opened in BINARY mode */

    png_init_io(png_ptr, getFilePtr());

    /* set the compression level*/

    png_set_compression_level(png_ptr, 5);

    png->getImageData(&data, &dataLength);
    width = png->getWidth();
    height = png->getHeight();

    /* set the image parameters appropriately */

    png_set_IHDR(
			png_ptr, 
			info_ptr, 
			width, 
			height,
			8, 
			PNG_COLOR_TYPE_RGB, 
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, 
			PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);
	
 	row = (uint8_t *)malloc(width * NUM_CHANNELS);

	if (row == NULL) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
        throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
	}

    if (setjmp(pngInfo.jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
		free(row);
        throw clk_error("Failed to set error jump buffer", __FILE__, __LINE__);
    }
	
	rowStart = row;

	for (y = 0L;y < height;++y) {
		row = rowStart;
	
		for (x = 0L;x < width;++x) {
			*row++ = *data++; // Red
			*row++ = *data++; // Green
			*row++ = *data++; // Blue
		}

		png_write_row(png_ptr, rowStart);
	}
	
    png_write_end(png_ptr, NULL);
	
	if (png_ptr != NULL && info_ptr != NULL) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
	}
}

void ImageOutputStream::_writeBMP(Bitmap * bmp)
{
    uint8_t *       header;
    uint32_t        headerLength;
    uint8_t *       data;
    uint32_t        dataLength;
    uint32_t        headerBytesWritten;
    uint32_t        dataBytesWritten;

    header = bmp->getHeader();
    headerLength = bmp->getHeaderLength();
    
    bmp->getImageData(&data, &dataLength);

    headerBytesWritten = fwrite(header, 1, headerLength, getFilePtr());

    if (headerBytesWritten < headerLength) {
        throw clk_error("Failed to read enough Bitmap header bytes", __FILE__, __LINE__);
    }

    dataBytesWritten = fwrite(data, 1, dataLength, getFilePtr());

    if (dataBytesWritten < dataLength) {
        throw clk_error("Failed to write enough Bitmap data", __FILE__, __LINE__);
    }
}

void ImageOutputStream::open()
{
    FILE *      fptr;

    fptr = fopen(getFilename().c_str(), "wb");

    if (fptr == NULL) {
        throw clk_error("Failed to open ImageStream for writing", __FILE__, __LINE__);
    }

    setFilePtr(fptr);
}

void ImageOutputStream::write(RGB24BitImage * image)
{
    if (image->getFormat() == PNGImage) {
        _writePNG((PNG *)image);
    }
    else if (image->getFormat() == BitmapImage) {
        _writeBMP((Bitmap *)image);
    }
    else {
        throw clk_error("Trying to write invalid image data", __FILE__, __LINE__);
    }
}
