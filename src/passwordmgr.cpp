#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
	#include <gcrypt.h>
}

#include "passwordmgr.h"

using namespace std;

void PasswordManager::getKey(uint8_t * key, void * input, uint32_t inputLength)
{
	gcry_md_algos		alg;

	switch (this->keyLength) {
		case VeryHigh:
			alg = GCRY_MD_SHA3_512;
			break;

		case High:
			alg = GCRY_MD_SHA3_384;
			break;

		case Medium:
			alg = GCRY_MD_SHA3_256;
			break;

		case Low:
			alg = GCRY_MD_BLAKE2S_128;
			break;

		default:
			alg = GCRY_MD_SHA3_256;
			break;
	}

	/*
	** Get the hash of the password...
	*/
	gcry_md_hash_buffer(alg, key, input, inputLength);
}

void PasswordManager::getKey(uint8_t * key, char * szPassword)
{
	return getKey(key, szPassword, strlen(szPassword));
}
