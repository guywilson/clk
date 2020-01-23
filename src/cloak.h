#include "filestream.h"
#include "datafile.h"
#include "ImageStream.h"
#include "image.h"

#ifndef _INCL_CLOAK
#define _INCL_CLOAK

class CloakHelper
{
    public:
        enum MergeQuality {
            High = 1,
            Medium = 2,
            Low = 4
        };

    private:
        uint8_t             getBitMask(MergeQuality bitsPerByte) {
            uint8_t mask = 0x00;

            for (int i = 0;i < bitsPerByte;i++) {
                mask += (0x01 << i) & 0xFF;
            }

            return mask;
        }

    public:
        CloakHelper() {}

        RGB24BitImage *     merge(RGB24BitImage * srcImage, DataFile * srcDataFile, MergeQuality bitsPerByte);
        DataFile *          extract(RGB24BitImage * srcImage, MergeQuality bitsPerByte);
};

#endif
