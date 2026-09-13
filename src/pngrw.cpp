#include <string>

#include <png.h>

#include "logger.h"
#include "clk_error.h"
#include "hostfile.h"
#include "pngrw.h"

static void _readwrite_error_handler(png_structp png_ptr, png_const_charp msg) {
    png_details_t * imageDetails = (png_details_t *)png_ptr;
    
    if (imageDetails->isWriter) {
        png_destroy_write_struct(&imageDetails->image_handle, &imageDetails->image_info);
    }
    else {
        png_destroy_read_struct(&imageDetails->image_handle, &imageDetails->image_info, NULL);
    }

    throw clk_error(
            clk_error::buildMsg(
                "libpng error: '%s'", 
                msg), 
            __FILE__, 
            __LINE__);
}

void PNGReader::onOpen() {
    imageDetails.isWriter = false;

	imageDetails.image_handle = 
                    png_create_read_struct(
                            PNG_LIBPNG_VER_STRING,
                            &imageDetails, 
                            _readwrite_error_handler, 
                            NULL);

    imageDetails.image_info = png_create_info_struct(imageDetails.image_handle);
	
    if (imageDetails.image_info == NULL) {
        png_destroy_read_struct(&imageDetails.image_handle, NULL, NULL);
        throw clk_error("Failed to create png info struct", __FILE__, __LINE__);
	}

	/* Set up the input control if you are using standard C streams */
	png_init_io(imageDetails.image_handle, fptr);
	
	png_read_info(imageDetails.image_handle, imageDetails.image_info);

	imageDetails.geometry.width = png_get_image_width(imageDetails.image_handle, imageDetails.image_info);
	imageDetails.geometry.height = png_get_image_height(imageDetails.image_handle, imageDetails.image_info);
    imageDetails.channels = png_get_channels(imageDetails.image_handle, imageDetails.image_info);
    imageDetails.bitDepth = png_get_bit_depth(imageDetails.image_handle, imageDetails.image_info);
	imageDetails.colourType = png_get_color_type(imageDetails.image_handle, imageDetails.image_info);

    imageDetails.bitsPerPixel = imageDetails.bitDepth * imageDetails.channels;

    png_byte interlaceType = png_get_interlace_type(imageDetails.image_handle, imageDetails.image_info);

    if (interlaceType != PNG_INTERLACE_NONE) {
        throw clk_error("Cloak does not support interlaced PNG images", __FILE__, __LINE__);
    }

    if (imageDetails.bitDepth == 16) {
        png_set_strip_16(imageDetails.image_handle);
    }

    if (imageDetails.colourType == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(imageDetails.image_handle);
    }

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if (imageDetails.colourType == PNG_COLOR_TYPE_GRAY && imageDetails.bitDepth < 8) {
        png_set_expand_gray_1_2_4_to_8(imageDetails.image_handle);
    }

    if ( imageDetails.colourType == PNG_COLOR_TYPE_GRAY ||
        imageDetails.colourType == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        png_set_gray_to_rgb(imageDetails.image_handle);
    }

    png_read_update_info(imageDetails.image_handle, imageDetails.image_info);

    if (imageDetails.bitsPerPixel != 24) {
        throw clk_error("CLK supports 24-bit RGB PNG images only");
    }

    readImage();
}

void PNGReader::onClose() {
	png_read_end(imageDetails.image_handle, NULL);
	png_destroy_read_struct(&imageDetails.image_handle, &imageDetails.image_info, NULL);

    if (imageDetails.imgBuffer) {
        free(imageDetails.imgBuffer);
        imageDetails.imgBuffer = NULL;
    }
}

void PNGReader::readImage() {
    imageDetails.imgBufferLength = 
                (size_t)imageDetails.geometry.height * 
                png_get_rowbytes(
                    imageDetails.image_handle, 
                    imageDetails.image_info);

    imageDetails.imgBuffer = (uint8_t *)malloc(imageDetails.imgBufferLength);

    if (imageDetails.imgBuffer == NULL) {
        throw clk_error(
            clk_error::buildMsg(
                "Failed to allocate image data buffer (%zu bytes)", 
                imageDetails.imgBufferLength), 
            __FILE__, 
            __LINE__);
    }

    rowCounter = 0;
    size_t index = 0;
    size_t rowBufferLength = png_get_rowbytes(imageDetails.image_handle, imageDetails.image_info);

    while (hasMoreRows()) {
        png_read_row(imageDetails.image_handle, &imageDetails.imgBuffer[index], NULL);
        rowCounter++;

        index += rowBufferLength;
    }
}

uint8_t * PNGReader::getBlockPointer(size_t blockLength) {
    if (readPointer >= (imageDetails.imgBufferLength - blockLength)) {
        throw clk_fatal(
                clk_error::buildMsg(
                    "Reading %zu bytes from this point will overrun the image buffer by %zu bytes", 
                    blockLength,
                    readPointer + blockLength - imageDetails.imgBufferLength), 
                __FILE__, 
                __LINE__);
    }

    if (readPointer >= imageDetails.imgBufferLength) {
        throw clk_error("At the end of the image buffer", __FILE__, __LINE__);
    }

    uint8_t * ptr =  &imageDetails.imgBuffer[readPointer];

    readPointer += blockLength;

    return ptr;
}

void PNGReader::readBlock(uint8_t * buffer, size_t bufferLength) {
    size_t index = 0;

    if (readPointer >= imageDetails.imgBufferLength) {
        throw clk_error("At the end of the image buffer", __FILE__, __LINE__);
    }

    while (readPointer < imageDetails.imgBufferLength && index < bufferLength) {
        buffer[index++] = imageDetails.imgBuffer[readPointer++];
    }
}

void PNGWriter::onOpen() {
    imageDetails.isWriter = true;
    
	imageDetails.image_handle = 
                    png_create_write_struct(
                            PNG_LIBPNG_VER_STRING,
                            &imageDetails, 
                            _readwrite_error_handler, 
                            NULL);

    imageDetails.image_info = png_create_info_struct(imageDetails.image_handle);
	
    if (imageDetails.image_info == NULL) {
        png_destroy_read_struct(&imageDetails.image_handle, NULL, NULL);
        throw clk_error("Failed to create png info struct", __FILE__, __LINE__);
	}

	/* Set up the input control if you are using standard C streams */
	png_init_io(imageDetails.image_handle, fptr);

    png_set_compression_level(imageDetails.image_handle, 7);

    png_set_IHDR(
            imageDetails.image_handle, 
            imageDetails.image_info, 
            imageDetails.geometry.width, 
            imageDetails.geometry.height, 
            8, 
            PNG_COLOR_TYPE_RGB, 
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT, 
            PNG_FILTER_TYPE_DEFAULT);

    png_write_info(imageDetails.image_handle, imageDetails.image_info);
}

void PNGWriter::onClose() {
    writeImage();

	png_write_end(imageDetails.image_handle, NULL);
	png_destroy_write_struct(&imageDetails.image_handle, &imageDetails.image_info);

    if (imageDetails.imgBuffer) {
        free(imageDetails.imgBuffer);
        imageDetails.imgBuffer = NULL;
    }
}

void PNGWriter::assignImageDetails(png_details_t * source) {
    imageDetails.geometry.width = source->geometry.width;
    imageDetails.geometry.height = source->geometry.height;
    imageDetails.channels = source->channels;
    imageDetails.bitDepth = source->bitDepth;
    imageDetails.bitsPerPixel = source->bitsPerPixel;
    imageDetails.colourType = source->colourType;

    imageDetails.imgBufferLength = source->imgBufferLength;

    imageDetails.imgBuffer = (uint8_t *)malloc(imageDetails.imgBufferLength);

    if (imageDetails.imgBuffer == NULL) {
        throw clk_error(
            clk_error::buildMsg(
                "Failed to allocate image data buffer (%zu bytes)", 
                imageDetails.imgBufferLength), 
            __FILE__, 
            __LINE__);
    }

    memcpy(imageDetails.imgBuffer, source->imgBuffer, imageDetails.imgBufferLength);
}

void PNGWriter::writeImage() {
    rowCounter = 0;
    size_t index = 0;
    size_t rowBufferLength = png_get_rowbytes(imageDetails.image_handle, imageDetails.image_info);

    while (hasMoreRows()) {
        png_write_row(imageDetails.image_handle, &imageDetails.imgBuffer[index]);
        rowCounter++;

        index += rowBufferLength;
    }
}
