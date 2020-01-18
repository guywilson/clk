#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern "C" {
	#include <gcrypt.h>
}

#include "clk_error.h"
#include "encrypt.h"
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
        throw clk_error("Failed to open hash handle", __FILE__, __LINE__);
	}

	err = gcry_md_enable(blake_hd, GCRY_MD_BLAKE2S_128);

	if (err) {
        throw clk_error("Failed to enable hash function", __FILE__, __LINE__);
	}

	gcry_md_write(blake_hd, key, keyLength);

	err = gcry_md_final(blake_hd);

	if (err) {
        throw clk_error("Failed to finalise hash function", __FILE__, __LINE__);
	}

	iv = gcry_md_read(blake_hd, GCRY_MD_BLAKE2S_128);

	gcry_md_close(blake_hd);

    return iv;
}

DataFile * EncryptionHelper::encryptAES256(DataFile * src, uint8_t * key, uint32_t keyLength)
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
        throw clk_error("Failed to open cipher function", __FILE__, __LINE__);
    }

    err = gcry_cipher_setkey(
    					aes_hd,
    					(const void *)key,
    					keyLength);

    if (err) {
        throw clk_error("Failed to set key", __FILE__, __LINE__);
    }

    err = gcry_cipher_setiv(
    					aes_hd,
    					(const void *)generateIV(key, keyLength),
    					BLOCK_SIZE);

    if (err) {
        throw clk_error("Failed to set iv", __FILE__, __LINE__);
    }

    src->getData(&inputData, &inputDataLength);

	if (inputDataLength % BLOCK_SIZE == 0) {
		outputDataLength = inputDataLength + BLOCK_SIZE;
	}
	else {
		outputDataLength = inputDataLength + (BLOCK_SIZE - (inputDataLength % BLOCK_SIZE) + BLOCK_SIZE);
	}

    printf("Input len %u, Output len %u\n", inputDataLength, outputDataLength);

    outputData = (uint8_t *)malloc(outputDataLength);

    if (outputData == NULL) {
        throw clk_error(clk_error::buildMsg("Failed to allocate %ld bytes", outputDataLength), __FILE__, __LINE__);
    }

	err = gcry_cipher_encrypt(
							aes_hd,
							outputData,
							outputDataLength,
							inputData,
							inputDataLength);

	if (err) {
        throw clk_error(clk_error::buildMsg("Failed to encrypt data - %s", gcry_strerror(err)), __FILE__, __LINE__);
	}

    gcry_cipher_close(aes_hd);

    outputDataFile = new DataFile(outputData, outputDataLength);

    return outputDataFile;
}

DataFile * EncryptionHelper::decryptAES256(DataFile * src, uint8_t * key, uint32_t keyLength)
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
        throw clk_error("Failed to open cipher function", __FILE__, __LINE__);
    }

    err = gcry_cipher_setkey(
    					aes_hd,
    					(const void *)key,
    					keyLength);

    if (err) {
        throw clk_error("Failed to set key", __FILE__, __LINE__);
    }

    err = gcry_cipher_setiv(
    					aes_hd,
    					(const void *)generateIV(key, keyLength),
    					BLOCK_SIZE);

    if (err) {
        throw clk_error("Failed to set iv", __FILE__, __LINE__);
    }

    src->getData(&inputData, &inputDataLength);

	if (inputDataLength % BLOCK_SIZE == 0) {
		outputDataLength = inputDataLength;
	}
	else {
		outputDataLength = inputDataLength + (BLOCK_SIZE - (inputDataLength % BLOCK_SIZE));
	}

    outputData = (uint8_t *)malloc(outputDataLength);

    if (outputData == NULL) {
        throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
    }

	err = gcry_cipher_decrypt(
							aes_hd,
							outputData,
							outputDataLength,
							inputData,
							inputDataLength);

	if (err) {
        throw clk_error("Failed to decrypt data", __FILE__, __LINE__);
	}

    gcry_cipher_close(aes_hd);

    outputDataFile = new DataFile(outputData, outputDataLength);

    return outputDataFile;
}

DataFile * EncryptionHelper::encryptXOR(DataFile * src, uint8_t * key, uint32_t keyLength)
{
    DataFile *          outputDataFile;
    uint8_t *           inputData;
    uint8_t *           outputData;
    uint32_t            inputDataLength;
    uint32_t            outputDataLength;

    src->getData(&inputData, &inputDataLength);

    if (keyLength < inputDataLength) {
        throw clk_error("Key length must be longer than plain text", __FILE__, __LINE__);
    }

	outputDataLength = inputDataLength;

    outputData = (uint8_t *)malloc(outputDataLength);

    if (outputData == NULL) {
        throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
    }

    for (int i = 0;i < inputDataLength;i++) {
        outputData[i] = inputData[i] ^ key[i];
    }

    outputDataFile = new DataFile(outputData, outputDataLength);

    return outputDataFile;
}

DataFile * EncryptionHelper::decryptXOR(DataFile * src, uint8_t * key, uint32_t keyLength)
{
    return encryptXOR(src, key, keyLength);
}
