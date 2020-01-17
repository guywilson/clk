#include <system_error>
#include <cerrno>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
	#include <gcrypt.h>
}

#include "passwordmgr.h"

using namespace std;

uint32_t PasswordManager::getKeyLength()
{
	return (256 / 8);
}

void PasswordManager::getKey(uint8_t * key, char * szPassword)
{
	uint32_t			pwdLength;

	pwdLength = (uint32_t)strlen(szPassword);

	/*
	** Get the SHA-256 hash of the password...
	*/
	gcry_md_hash_buffer(GCRY_MD_SHA3_256, key, szPassword, pwdLength);
}
