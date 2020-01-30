#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern "C" {
	#include <gcrypt.h>
}

#include "clk_error.h"
#include "memutil.h"
#include "encrypt.h"
#include "datafile.h"

using namespace std;

uint8_t * EncryptionHelper::generateIV()
{
    uint8_t *           iv;

    iv = (uint8_t *)malloc(BLOCK_SIZE);

    if (iv == NULL) {
        throw clk_error("Failed to allocate memory for IV", __FILE__, __LINE__);
    }

    gcry_randomize(iv, BLOCK_SIZE, GCRY_STRONG_RANDOM);

    return iv;
}

uint8_t * EncryptionHelper::encryptXOR(uint8_t * srcData, uint32_t srcDataLength, uint8_t * key, uint32_t keyLength)
{
    uint8_t *               outputData;

    if (keyLength < srcDataLength) {
        throw clk_error("Key length must be longer than plain text", __FILE__, __LINE__);
    }

    outputData = (uint8_t *)malloc(srcDataLength);

    if (outputData == NULL) {
        throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
    }

    for (int i = 0;i < srcDataLength;i++) {
        outputData[i] = srcData[i] ^ key[i];
    }

    return outputData;
}

DataFile * EncryptionHelper::encryptAES256(DataFile * src, uint8_t * key, uint32_t keyLength)
{
    DataFile *              outputDataFile;
    gcry_cipher_hd_t	    aes_hd;
    uint8_t *               inputData;
    uint8_t *               outputData;
    uint8_t *               iv;
    uint32_t                inputDataLength;
    uint32_t                outputDataLength;
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

    err = gcry_cipher_setkey(
    					aes_hd,
    					(const void *)key,
    					keyLength);

    if (err) {
        throw clk_error("Failed to set key", __FILE__, __LINE__);
    }

    iv = generateIV();

    err = gcry_cipher_setiv(
    					aes_hd,
    					iv,
    					BLOCK_SIZE);

    if (err) {
        throw clk_error("Failed to set iv", __FILE__, __LINE__);
    }

    blockLength = gcry_cipher_get_algo_blklen(GCRY_CIPHER_AES256);

    if (!blockLength) {
        throw clk_error("Faied to get algorithm block length", __FILE__, __LINE__);
    }

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

    outputData = (uint8_t *)malloc(outputDataLength + BLOCK_SIZE);

    if (outputData == NULL) {
        throw clk_error(clk_error::buildMsg("Failed to allocate %ld bytes", outputDataLength), __FILE__, __LINE__);
    }

    /*
    ** Prepend the output data wih the iv...
    */
    memcpy(outputData, iv, BLOCK_SIZE);
    memcpy(&outputData[BLOCK_SIZE], inputData, inputDataLength);

    memclr(iv, BLOCK_SIZE);

	err = gcry_cipher_encrypt(
							aes_hd,
							&outputData[BLOCK_SIZE],
							outputDataLength,
							NULL,//inputData,
							0);//inputDataLength);

	if (err) {
        throw clk_error(clk_error::buildMsg("Failed to encrypt data - %s [%d]", gcry_strerror(err), err), __FILE__, __LINE__);
	}

    gcry_cipher_close(aes_hd);

    outputDataFile = new DataFile(outputData, outputDataLength + BLOCK_SIZE);

    return outputDataFile;
}

DataFile * EncryptionHelper::decryptAES256(DataFile * src, uint32_t decryptedDataLength, uint8_t * key, uint32_t keyLength)
{
    DataFile *              outputDataFile;
    gcry_cipher_hd_t	    aes_hd;
    uint8_t *               inputData;
    uint8_t *               outputData;
    uint8_t *               iv;
    uint32_t                inputDataLength;
    uint32_t                outputDataLength;
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
    
    src->getData(&inputData, &inputDataLength);
    
    iv = (uint8_t *)malloc(BLOCK_SIZE);

    if (iv == NULL) {
        throw clk_error("Failed to allocate memory for IV", __FILE__, __LINE__);
    }

    /*
    ** IV is prepended to the input data...
    */
    memcpy(iv, inputData, BLOCK_SIZE);

    err = gcry_cipher_setiv(
    					aes_hd,
    					iv,
    					BLOCK_SIZE);
    if (err) {
        throw clk_error("Failed to set iv", __FILE__, __LINE__);
    }

	outputDataLength = inputDataLength - BLOCK_SIZE;

    outputData = (uint8_t *)malloc(outputDataLength);

    if (outputData == NULL) {
        throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
    }

	err = gcry_cipher_decrypt(
							aes_hd,
							outputData,
							outputDataLength,
							&inputData[BLOCK_SIZE],
							inputDataLength - BLOCK_SIZE);

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

    outputData = encryptXOR(inputData, inputDataLength, key, keyLength);

    outputDataLength = inputDataLength;

    outputDataFile = new DataFile(outputData, outputDataLength);

    return outputDataFile;
}

DataFile * EncryptionHelper::decryptXOR(DataFile * src, uint8_t * key, uint32_t keyLength)
{
    return encryptXOR(src, key, keyLength);
}

DataFile * EncryptionHelper::seededXOR(DataFile * src, uint32_t seed)
{
    DataFile *      outputDataFile;
    uint8_t *       srcData;
    uint32_t        srcDataLength;
    uint8_t *       outputData;
    uint8_t *       data = NULL;
    uint8_t         key[64];
    uint32_t        xorLength;
    int             i;

    srcData = src->getData();
    srcDataLength = src->getDataLength();

    outputData = (uint8_t *)malloc(srcDataLength);

    if (outputData == NULL) {
        throw clk_error("Failed to allocate memory for output buffer", __FILE__, __LINE__);
    }

    xorLength = 64;

    for (i = 0;i < srcDataLength;i += xorLength) {
        gcry_md_hash_buffer(GCRY_MD_SHA3_512, key, &seed, sizeof(uint32_t));

        if (i > (srcDataLength - xorLength)) {
            xorLength = srcDataLength - i;
        }

        data = encryptXOR(&srcData[i], xorLength, key, 64);
        memcpy(&outputData[i], data, xorLength);
        free(data);

        seed++;
    }

    outputDataFile = new DataFile(outputData, srcDataLength);

    return outputDataFile;
}
