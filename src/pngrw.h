#pragma once

#include <string>

#include <png.h>

#include "hostfile.h"

struct png_geometry_t {
    png_uint_32 width;
    png_uint_32 height;
};

struct png_details_t {
    bool isWriter;
    
    png_structp image_handle;
    png_infop image_info;

    png_byte bitDepth;
    png_byte colourType;

    png_byte channels;
    png_uint_16 bitsPerPixel;

    png_geometry_t geometry;

    uint8_t * imgBuffer = NULL;
    size_t imgBufferLength = 0;
};

class PNGReader : public HostFileReader {
    private:
        size_t rowCounter = 0;
        size_t readPointer = 0;
        png_details_t imageDetails;

        void readImage();
        bool hasMoreRows() {
            return (rowCounter < imageDetails.geometry.height ? true : false);
        }

    protected:
        void onOpen() override;
        void onClose() override;

    public:
        void * getData() override {
            return imageDetails.imgBuffer;
        }

        void reset() {
            readPointer = 0;
            rowCounter = 0;
        }
        
        size_t getCapacity(size_t headerSize, const CloakSecurity & securityLevel) override {
            return (imageDetails.imgBufferLength - headerSize) / ((size_t)8 / (size_t)securityLevel);
        }

        uint8_t * getBlockPointer(size_t blockLength);
        void readBlock(uint8_t * buffer, size_t bufferLength) override;

        png_details_t * getPNGDetails() {
            return &imageDetails;
        }
};

class PNGWriter : public HostFileWriter {
    private:
        size_t rowCounter = 0;
        png_details_t imageDetails;

        void writeImage();
        bool hasMoreRows() {
            return (rowCounter < imageDetails.geometry.height ? true : false);
        }

    protected:
        void onOpen() override;
        void onClose() override;
        
    public:
        void assignImageDetails(png_details_t * source);
        void writeBlock(uint8_t * buffer, size_t bufferLength) override {}
};
