#include <stdint.h>

#ifndef _INCL_PASSWORDMGR
#define _INCL_PASSWORDMGR

class PasswordManager
{
    public:
        PasswordManager();

        uint8_t *       getKey(char * szPassword);
};

#endif
