#include <stdint.h>

#include "datafile.h"

#ifndef _INCL_ENCRYPT
#define _INCL_ENCRYPT

class EncryptionHelper
{
    public:
        EncryptionHelper();

        DataFile *      encrypt(DataFile & src, uint8_t * key);
        DataFile *      decrypt(DataFile & src, uint8_t * key);
};

#endif
