#include <stdint.h>

#ifndef _INCL_PASSWORDMGR
#define _INCL_PASSWORDMGR

class PasswordManager
{
    public:
        PasswordManager() {}

        uint32_t    getKeyLength();

        void        getKey(uint8_t * key, char * szPassword);
};

#endif
