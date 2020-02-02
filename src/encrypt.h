#include <stdint.h>

extern "C" {
	#include <gcrypt.h>
}

#include "datafile.h"

#ifndef _INCL_ENCRYPT
#define _INCL_ENCRYPT

class EncryptionHelper
{
    private:
        uint8_t *               generateIV();

        uint8_t *               encryptXOR(uint8_t * srcData, uint32_t srcDataLength, uint8_t * key, uint32_t keyLength);

        DataFile *              encryptAES256(DataFile * src, uint8_t * key, uint32_t keyLength);
        DataFile *              decryptAES256(DataFile * src, uint32_t decryptedDataLength, uint8_t * key, uint32_t keyLength);

        DataFile *              encryptXOR(DataFile * src, uint8_t * key, uint32_t keyLength);
        DataFile *              decryptXOR(DataFile * src, uint8_t * key, uint32_t keyLength);

    public:
        EncryptionHelper();

        enum Algorithm {
            AES_256,
            XOR
        };

        DataFile * encrypt(DataFile * src, Algorithm alg, uint8_t * key, uint32_t keyLength) {
            switch (alg) {
                case AES_256:
                    return encryptAES256(src, key, keyLength);

                case XOR:
                    return encryptXOR(src, key, keyLength);

                default:
                    return NULL;
            }
        }

        DataFile * decrypt(DataFile * src, Algorithm alg, uint32_t decryptedDataLength, uint8_t * key, uint32_t keyLength) {
            switch (alg) {
                case AES_256:
                    return decryptAES256(src, decryptedDataLength, key, keyLength);

                case XOR:
                    return decryptXOR(src, key, keyLength);

                default:
                    return NULL;
            }
        }
};

#endif
