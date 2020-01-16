#include <stdint.h>

#include "datafile.h"

#ifndef _INCL_ENCRYPT
#define _INCL_ENCRYPT

#define BLOCK_SIZE			16

class EncryptionHelper
{
    private:
        uint8_t *       generateIV(uint8_t * key, uint32_t keyLength);

    public:
        EncryptionHelper() {}

        DataFile *      encrypt(DataFile & src, uint8_t * key, uint32_t keyLength);
        DataFile *      decrypt(DataFile & src, uint8_t * key, uint32_t keyLength);
};

#endif
