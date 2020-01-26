#include <stdint.h>

#include "datafile.h"

#ifndef _INCL_ENCRYPT
#define _INCL_ENCRYPT

#define BLOCK_SIZE			16

class EncryptionHelper
{
    private:
        uint8_t *               generateIV(uint8_t * key, uint32_t keyLength);

        LengthEncodedDataFile * encryptAES256(DataFile * src, uint8_t * key, uint32_t keyLength);
        LengthEncodedDataFile * decryptAES256(DataFile * src, uint8_t * key, uint32_t keyLength);

        LengthEncodedDataFile * encryptXOR(DataFile * src, uint8_t * key, uint32_t keyLength);
        LengthEncodedDataFile * decryptXOR(DataFile * src, uint8_t * key, uint32_t keyLength);

    public:
        EncryptionHelper() {}

        enum Algorithm {
            AES_256,
            XOR
        };

        LengthEncodedDataFile * encrypt(DataFile * src, Algorithm alg, uint8_t * key, uint32_t keyLength) {
            switch (alg) {
                case AES_256:
                    return encryptAES256(src, key, keyLength);

                case XOR:
                    return encryptXOR(src, key, keyLength);

                default:
                    return NULL;
            }
        }

        LengthEncodedDataFile * decrypt(DataFile * src, Algorithm alg, uint8_t * key, uint32_t keyLength) {
            switch (alg) {
                case AES_256:
                    return decryptAES256(src, key, keyLength);

                case XOR:
                    return decryptXOR(src, key, keyLength);

                default:
                    return NULL;
            }
        }
};

#endif
