#include <stdint.h>

#ifndef _INCL_PASSWORDMGR
#define _INCL_PASSWORDMGR

class PasswordManager
{
    public:
        PasswordManager() {}

        static uint32_t getKeyLength() {
            return (256 / 8);
        }

        void            getKey(uint8_t * key, char * szPassword);
};

#endif
