#include <string>
#include <system_error>
#include <stdio.h>
#include <cerrno>

extern "C" {
#include <png.h>
}

#include "ImageStream.h"

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

ImageInputStream::ImageInputStream() : ImageStream()
{

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

void ImageInputStream::open(string & filename)
{
    FILE *      fptr;

    fptr = fopen(filename.c_str(), "rb");

    if (fptr == NULL) {
        throw new system_error(make_error_code(errc::no_such_file_or_directory));
    }

    setFilePtr(fptr);
}

RGB24BitImage * ImageInputStream::read()
{
    RGB24BitImage * image;
    uint8_t *       headerBuffer;
    int             headerBytesRead;
    int             bytesRead;

    headerBytesRead = fread(headerBuffer, 1, 8, getFilePtr());

    if (isPNG(headerBuffer, headerBytesRead)) {
        PNG * png = _readPNG();
        image = png;
    }
    else if (isBMP(headerBuffer, headerBytesRead)) {
        Bitmap * bmp = _readBMP();
        image = bmp;
    }
    else {
        throw new system_error(make_error_code(errc::not_supported));
    }

    return image;
}