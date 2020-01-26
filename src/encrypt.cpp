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

	iv = gcry_md_read(blake_hd, GCRY_MD_BLAKE2S_128);

	gcry_md_close(blake_hd);

    return iv;
}

DataFile * EncryptionHelper::encryptAES256(DataFile * src, uint8_t * key, uint32_t keyLength)
{
    DataFile *              outputDataFile;
    gcry_cipher_hd_t	    aes_hd;
    uint8_t *               inputData;
    uint8_t *               outputData;
    uint32_t                inputDataLength;
    uint32_t                outputDataLength;
    uint32_t                ivLength;
    uint32_t                blockLength;
    int                     err;

    err = gcry_cipher_open(
    					&aes_hd,
    					GCRY_CIPHER_AES256,
                        GCRY_CIPHER_MODE_CBC,
                        0);

    if (err) {
        throw clk_error("Failed to open cipher function", __FILE__, __LINE__);
    }

    printf("Key length = %u\n", keyLength * 8);

    err = gcry_cipher_setkey(
    					aes_hd,
    					(const void *)key,
    					keyLength);

    if (err) {
        throw clk_error("Failed to set key", __FILE__, __LINE__);
    }

    ivLength = gcry_md_get_algo_dlen(GCRY_MD_BLAKE2S_128);

    err = gcry_cipher_setiv(
    					aes_hd,
    					(const void *)generateIV(key, keyLength),
    					ivLength);

    if (err) {
        throw clk_error("Failed to set iv", __FILE__, __LINE__);
    }

    blockLength = gcry_cipher_get_algo_blklen(GCRY_CIPHER_AES256);

    if (!blockLength) {
        throw clk_error("Faied to get algorithm block length", __FILE__, __LINE__);
    }

    printf("Block length = %u\n", blockLength);

    src->getData(&inputData, &inputDataLength);

    if (inputData == NULL) {
        throw clk_error("Failed to get input data", __FILE__, __LINE__);
    }

	if (inputDataLength % blockLength == 0) {
		outputDataLength = inputDataLength;
	}
	else {
		outputDataLength = inputDataLength + (blockLength - (inputDataLength % blockLength));
	}

    printf("Input len %u, Output len %u\n", inputDataLength, outputDataLength);

    outputData = (uint8_t *)malloc(outputDataLength);

    if (outputData == NULL) {
        throw clk_error(clk_error::buildMsg("Failed to allocate %ld bytes", outputDataLength), __FILE__, __LINE__);
    }

    memcpy(outputData, inputData, inputDataLength);

	err = gcry_cipher_encrypt(
							aes_hd,
							outputData,
							outputDataLength,
							NULL,//inputData,
							0);//inputDataLength);

	if (err) {
        throw clk_error(clk_error::buildMsg("Failed to encrypt data - %s [%d]", gcry_strerror(err), err), __FILE__, __LINE__);
	}

    gcry_cipher_close(aes_hd);

    outputDataFile = new DataFile(outputData, outputDataLength);

    return outputDataFile;
}

DataFile * EncryptionHelper::decryptAES256(DataFile * src, uint32_t decryptedDataLength, uint8_t * key, uint32_t keyLength)
{
    DataFile *              outputDataFile;
    gcry_cipher_hd_t	    aes_hd;
    uint8_t *               inputData;
    uint8_t *               outputData;
    uint32_t                inputDataLength;
    uint32_t                outputDataLength;
    uint32_t                ivLength;
    int                     err;

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

    ivLength = gcry_md_get_algo_dlen(GCRY_MD_BLAKE2S_128);

    err = gcry_cipher_setiv(
    					aes_hd,
    					(const void *)generateIV(key, keyLength),
    					ivLength);
    if (err) {
        throw clk_error("Failed to set iv", __FILE__, __LINE__);
    }

    src->getData(&inputData, &inputDataLength);
    
	outputDataLength = inputDataLength;

    printf("Input data length = %u, output data length = %u\n", inputDataLength, outputDataLength);

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
        throw clk_error(clk_error::buildMsg("Failed to decrypt data - %s [%d]", gcry_strerror(err), err), __FILE__, __LINE__);
	}

    gcry_cipher_close(aes_hd);

    outputDataFile = new DataFile(outputData, decryptedDataLength);

    return outputDataFile;
}

DataFile * EncryptionHelper::encryptXOR(DataFile * src, uint8_t * key, uint32_t keyLength)
{
    DataFile *              outputDataFile;
    uint8_t *               inputData;
    uint8_t *               outputData;
    uint32_t                inputDataLength;
    uint32_t                outputDataLength;

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
