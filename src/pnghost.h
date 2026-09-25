#pragma once

#include <string>

#include <png.h>

#include "image.h"

class PNGHost : public Image {
    public:
        void addBlock(             
                HostFileReader * reader,           
                uint8_t * sourceBlock, 
                size_t sourceBlockSize) override;

        void extractBlock(
                        HostFileReader * reader,           
                        uint8_t * targetBlock, 
                        size_t targetBlockSize) override;
};
