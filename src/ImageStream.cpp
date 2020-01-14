#include <string>
#include <system_error>
#include <stdio.h>
#include <cerrno>

extern "C" {
#include <png.h>
}

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

    return is_png != 0 ? true : false;
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

PNG * ImageInputStream::_readPNG()
{
    PNG *           png;
	png_structp		png_ptr;
	png_infop		info_ptr;
	PNG_INFO		pngInfo;
    uint32_t        width;
    uint32_t        height;
    uint32_t        dataLength;
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
	bitsPerPixel = png_get_bit_depth(png_ptr, info_ptr) * png_get_channels(png_ptr, info_ptr);
	colourType = png_get_color_type(png_ptr, info_ptr);
	
    if (bitsPerPixel != 24) {
        throw new system_error(make_error_code(errc::not_supported));
    }
    if (colourType != PNG_COLOR_TYPE_RGB) {
        throw new system_error(make_error_code(errc::not_supported));
    }

    dataLength = png_get_image_width(png_ptr, info_ptr) * png_get_channels(png_ptr, info_ptr) * png_get_image_height(png_ptr, info_ptr);

	data = (uint8_t *)malloc(dataLength);
	
	if (data == NULL) {
	  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	  return NULL;
	}
	
	dataStart = data;
	
	row = (uint8_t *)malloc(png_get_image_width(png_ptr, info_ptr) * png_get_channels(png_ptr, info_ptr));
	
	if (row == NULL) {
	  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	  return NULL;
	}
	
	rowStart = row;
	
	for (y = 0;y < png_get_image_height(png_ptr, info_ptr);++y) {
		row = rowStart;
		
		png_read_row(png_ptr, row, NULL);
		
		for (x = 0;x < png_get_image_width(png_ptr, info_ptr);++x) {
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
	uint8_t 	    headerBuffer[BMP_HEADER_SIZE];
	uint8_t		    DIBHeaderBuffer[WINV3_HEADER_SIZE];
    uint32_t        DIBHeaderSize;
    uint32_t        fileSize;
    uint32_t        startOffset;
    uint32_t        width;
    uint32_t        height;
    uint32_t        compressionMethod;
    uint32_t        dataLength;
    uint8_t *       data;
    uint16_t        bitsPerPixel;
    BitmapType      type;
    Bitmap *        bmp;

	/*
	** Read bitmap header...
	*/
	fread(headerBuffer, 1, BMP_HEADER_SIZE, getFilePtr());

	memcpy(&fileSize, &headerBuffer[2], 4);
	memcpy(&startOffset, &headerBuffer[10], 4);

	fread(&DIBHeaderSize, 1, DIB_HEADER_SIZE_LEN, getFilePtr());
	fread(DIBHeaderBuffer, 1, DIBHeaderSize - DIB_HEADER_SIZE_LEN, getFilePtr());

	if (DIBHeaderSize == WINV3_HEADER_SIZE) {
		type = WindowsV3;

        memcpy(&width, &DIBHeaderBuffer[0], 4);
        memcpy(&height, &DIBHeaderBuffer[4], 4);
        memcpy(&bitsPerPixel, &DIBHeaderBuffer[10], 2);
        memcpy(&compressionMethod, &DIBHeaderBuffer[12], 4);
        memcpy(&dataLength, &DIBHeaderBuffer[16], 4);
	}
	else if (DIBHeaderSize == OS2V1_HEADER_SIZE) {
		type = OS2V1;

        memcpy(&width, &DIBHeaderBuffer[0], 2);
        memcpy(&height, &DIBHeaderBuffer[2], 2);
        memcpy(&bitsPerPixel, &DIBHeaderBuffer[6], 2);

		dataLength = fileSize - startOffset;
	}
	else {
		type = UnknownType;

        throw new system_error(make_error_code(errc::not_supported));
	}

    if (bitsPerPixel != 24) {
        throw new system_error(make_error_code(errc::not_supported));
    }
    if (compressionMethod != COMPRESSION_BI_RGB) {
        throw new system_error(make_error_code(errc::not_supported));
    }

    memclr(headerBuffer, BMP_HEADER_SIZE);
    memclr(DIBHeaderBuffer, DIBHeaderSize);
    memclr(&DIBHeaderSize, DIB_HEADER_SIZE_LEN);

	/*
	** Seek to beginning of bitmap data...
	*/
    if (fseek(getFilePtr(), startOffset, SEEK_SET)) {
		throw new system_error(make_error_code(errc::invalid_seek));
	}

	data = (uint8_t *)malloc((size_t)dataLength);

	if (data == NULL) {
		return NULL;
	}

	fread(data, 1, dataLength, getFilePtr());

    bmp = new Bitmap(data, dataLength, width, height);
    bmp->setType(type);

    return bmp;
}

void ImageInputStream::open()
{
    FILE *      fptr;

    fptr = fopen(getFilename().c_str(), "rb");

    if (fptr == NULL) {
        throw new system_error(make_error_code(errc::no_such_file_or_directory));
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
        throw new system_error(make_error_code(errc::not_supported));
    }

    memclr(headerBuffer, 8);

    return image;
}


ImageOutputStream::ImageOutputStream(string & filename) : ImageStream(filename)
{

}

void ImageOutputStream::_writePNG(PNG * png)
{
	long			y;
	long			x;
	png_structp		png_ptr = NULL;
    png_infop		info_ptr = NULL;
	uint8_t *		row = NULL;
	uint8_t *		rowStart = NULL;
    uint8_t *       data;
    uint32_t        dataLength;
	PNG_INFO		pngInfo;

    /* could also replace libpng warning-handler (final NULL), but no need: */
    png_ptr = png_create_write_struct(
					PNG_LIBPNG_VER_STRING, 
					&pngInfo,
					pngreadwrite_error_handler, 
					NULL);
    
	if (!png_ptr) {
        throw new system_error(make_error_code(errc::not_enough_memory));
    }

    info_ptr = png_create_info_struct(png_ptr);
    
	if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        throw new system_error(make_error_code(errc::not_enough_memory));
    }

    /* make sure outfile is (re)opened in BINARY mode */

    png_init_io(png_ptr, getFilePtr());

    /* set the compression level*/

    png_set_compression_level(png_ptr, 5);

    /* set the image parameters appropriately */

    png_set_IHDR(
			png_ptr, 
			info_ptr, 
			png->getWidth(), 
			png->getWidth(),
			8, 
			PNG_COLOR_TYPE_RGB, 
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, 
			PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);
	
 	row = (uint8_t *)malloc(png->getWidth() * NUM_CHANNELS);

	if (row == NULL) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
        throw new system_error(make_error_code(errc::not_enough_memory));
	}

    if (setjmp(pngInfo.jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
		free(row);
        throw new system_error(make_error_code(errc::operation_not_permitted));
    }
	
    png->getImageData(&data, &dataLength);

	rowStart = row;

	for (y = 0L;y < png->getHeight();++y) {
		row = rowStart;
	
		for (x = 0L;x < png->getWidth();++x) {
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

    bmp->getHeader(&header, &headerLength);
    bmp->getImageData(&data, &dataLength);

    headerBytesWritten = fwrite(header, 1, headerLength, getFilePtr());

    if (headerBytesWritten < headerLength) {
        throw new system_error(make_error_code(errc::io_error));
    }

    dataBytesWritten = fwrite(data, 1, dataLength, getFilePtr());

    if (dataBytesWritten < dataLength) {
        throw new system_error(make_error_code(errc::io_error));
    }
}

void ImageOutputStream::open()
{
    FILE *      fptr;

    fptr = fopen(getFilename().c_str(), "wb");

    if (fptr == NULL) {
        throw new system_error(make_error_code(errc::no_such_file_or_directory));
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
        throw new system_error(make_error_code(errc::not_supported));
    }
}
