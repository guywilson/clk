#include "filestream.h"
#include "datafile.h"
#include "ImageStream.h"
#include "image.h"

#ifndef _INCL_CLOAK
#define _INCL_CLOAK

typedef struct __attribute__((__packed__))
{
    uint32_t        originalLength;
    uint32_t        encryptedLength;
}
clk_length_struct;

class CloakHelper
{
    public:
        enum MergeQuality {
            High = 1,
            Medium = 2,
            Low = 4
        };

    private:
        uint8_t getBitMask(MergeQuality bitsPerByte) {
            uint8_t mask = 0x00;

            for (int i = 0;i < bitsPerByte;i++) {
                mask += (0x01 << i) & 0xFF;
            }

            return mask;
        }

    public:
        CloakHelper() {}

        RGB24BitImage * merge(
                            RGB24BitImage * srcImage, 
                            DataFile * srcDataFile, 
                            clk_length_struct * lengthStruct, 
                            MergeQuality bitsPerByte);

        DataFile * extract(
                            RGB24BitImage * srcImage, 
                            clk_length_struct * clk_length_struct,
                            MergeQuality bitsPerByte);
};

#endif
