#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../src/passwordmgr.h"
#include "testsuite.h"

bool test_getKey()
{
    char            szPassword[32];
    uint8_t *       key;
    uint32_t        keyLength;
    PasswordManager mgr;

    printf("In test_getKey()\n");
    
    strcpy(szPassword, "Thisisacrappassword");

    keyLength = mgr.getKeyByteLength();

    key = (uint8_t *)malloc(keyLength);

    mgr.getKey(key, szPassword);

    printf("Key for password [%s] is: ", szPassword);
    for (int i = 0;i < keyLength;i++) {
        printf("%2X", key[i]);
    }
    printf("\n\n");

    return true;
}
