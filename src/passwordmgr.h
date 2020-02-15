#include <stdint.h>

#ifndef _INCL_PASSWORDMGR
#define _INCL_PASSWORDMGR

class PasswordManager
{
    public:
        enum Keylength {
            VeryHigh = 512,
            High = 384,
            Medium = 256,
            Low = 128
        };

    private:
        Keylength       keyLength;

    public:
        PasswordManager() : PasswordManager(Medium) {}
        
        PasswordManager(Keylength keylen) {
            this->keyLength = keylen;
        }

        static uint32_t getKeyByteLength() {
            return (256 / 8);
        }

        void            getKey(uint8_t * key, char * szPassPhrase);
        void            getKey(uint8_t * key, void * input, uint32_t inputLength);
};

#endif
