#include <system_error>
#include <cerrno>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern "C" {
	#include <gcrypt.h>
}

#include "encypt.h"
#include "datafile.h"

using namespace std;

uint8_t * EncryptionHelper::generateIV(uint8_t * key, uint32_t keyLength)
{
	int					err;
    uint8_t *           iv;
	gcry_md_hd_t		blake_hd;

    /*
	** Get the Blake-128 hash of the key as the iv...
	*/
	err = gcry_md_open(&blake_hd, 0, 0);

	if (err) {
        throw new system_error(make_error_code(errc::protocol_error));
	}

	err = gcry_md_enable(blake_hd, GCRY_MD_BLAKE2S_128);

	if (err) {
        throw new system_error(make_error_code(errc::protocol_error));
	}

	gcry_md_write(blake_hd, key, keyLength);

	err = gcry_md_final(blake_hd);

	if (err) {
        throw new system_error(make_error_code(errc::protocol_error));
	}

	iv = gcry_md_read(blake_hd, GCRY_MD_BLAKE2S_128);

	gcry_md_close(blake_hd);

    return iv;
}

DataFile * EncryptionHelper::encryptAES256(DataFile & src, uint8_t * key, uint32_t keyLength)
{
    DataFile *          outputDataFile;
    gcry_cipher_hd_t	aes_hd;
    uint8_t *           inputData;
    uint8_t *           outputData;
    uint32_t            inputDataLength;
    uint32_t            outputDataLength;
    int                 err;

    err = gcry_cipher_open(
    					&aes_hd,
    					GCRY_CIPHER_AES256,
                        GCRY_CIPHER_MODE_CBC,
                        0);

    if (err) {
        throw new system_error(make_error_code(errc::protocol_error));
    }

    err = gcry_cipher_setkey(
    					aes_hd,
    					(const void *)key,
    					keyLength);

    if (err) {
        throw new system_error(make_error_code(errc::protocol_error));
    }

    err = gcry_cipher_setiv(
    					aes_hd,
    					(const void *)generateIV(key, keyLength),
    					BLOCK_SIZE);

    if (err) {
        throw new system_error(make_error_code(errc::protocol_error));
    }

    src.getData(&inputData, &inputDataLength);

	if (inputDataLength % BLOCK_SIZE == 0) {
		outputDataLength = inputDataLength;
	}
	else {
		outputDataLength = inputDataLength + (BLOCK_SIZE - (inputDataLength % BLOCK_SIZE));
	}

    outputData = (uint8_t *)malloc(outputDataLength);

    if (outputData == NULL) {
        throw new system_error(make_error_code(errc::not_enough_memory));
    }

	err = gcry_cipher_encrypt(
							aes_hd,
							outputData,
							outputDataLength,
							inputData,
							inputDataLength);

	if (err) {
        throw new system_error(make_error_code(errc::protocol_error));
	}

    gcry_cipher_close(aes_hd);

    outputDataFile = new DataFile(outputData, outputDataLength);

    return outputDataFile;
}

DataFile * EncryptionHelper::decryptAES256(DataFile & src, uint8_t * key, uint32_t keyLength)
{
    DataFile *          outputDataFile;
    gcry_cipher_hd_t	aes_hd;
    uint8_t *           inputData;
    uint8_t *           outputData;
    uint32_t            inputDataLength;
    uint32_t            outputDataLength;
    int                 err;

    err = gcry_cipher_open(
    					&aes_hd,
    					GCRY_CIPHER_AES256,
                        GCRY_CIPHER_MODE_CBC,
                        0);

    if (err) {
        throw new system_error(make_error_code(errc::protocol_error));
    }

    err = gcry_cipher_setkey(
    					aes_hd,
    					(const void *)key,
    					keyLength);

    if (err) {
        throw new system_error(make_error_code(errc::protocol_error));
    }

    err = gcry_cipher_setiv(
    					aes_hd,
    					(const void *)generateIV(key, keyLength),
    					BLOCK_SIZE);

    if (err) {
        throw new system_error(make_error_code(errc::protocol_error));
    }

    src.getData(&inputData, &inputDataLength);

	if (inputDataLength % BLOCK_SIZE == 0) {
		outputDataLength = inputDataLength;
	}
	else {
		outputDataLength = inputDataLength + (BLOCK_SIZE - (inputDataLength % BLOCK_SIZE));
	}

    outputData = (uint8_t *)malloc(outputDataLength);

    if (outputData == NULL) {
        throw new system_error(make_error_code(errc::not_enough_memory));
    }

	err = gcry_cipher_decrypt(
							aes_hd,
							outputData,
							outputDataLength,
							inputData,
							inputDataLength);

	if (err) {
        throw new system_error(make_error_code(errc::protocol_error));
	}

    gcry_cipher_close(aes_hd);

    outputDataFile = new DataFile(outputData, outputDataLength);

    return outputDataFile;
}

DataFile * EncryptionHelper::encryptXOR(DataFile & src, uint8_t * key, uint32_t keyLength)
{
    DataFile *          outputDataFile;
    uint8_t *           inputData;
    uint8_t *           outputData;
    uint32_t            inputDataLength;
    uint32_t            outputDataLength;

    src.getData(&inputData, &inputDataLength);

    if (keyLength < inputDataLength) {
        throw new system_error(make_error_code(errc::protocol_error));
    }

	outputDataLength = inputDataLength;

    outputData = (uint8_t *)malloc(outputDataLength);

    if (outputData == NULL) {
        throw new system_error(make_error_code(errc::not_enough_memory));
    }

    for (int i = 0;i < inputDataLength;i++) {
        outputData[i] = inputData[i] ^ key[i];
    }

    outputDataFile = new DataFile(outputData, outputDataLength);

    return outputDataFile;
}

DataFile * EncryptionHelper::decryptXOR(DataFile & src, uint8_t * key, uint32_t keyLength)
{
    return encryptXOR(src, key, keyLength);
}
