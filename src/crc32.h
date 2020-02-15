#include <stdint.h>

#include "datafile.h"

#ifndef _INCL_CRC32
#define _INCL_CRC32

class CRC32Helper
{
    private:
        void initialise();

        uint32_t reflect(uint32_t reflect, uint8_t c);
        uint32_t table[256];

    public:
        CRC32Helper() {
            this->initialise();
        }

        ~CRC32Helper() {}

        uint32_t calculateCRC(DataFile * src);
};

#endif
