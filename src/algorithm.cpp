#include <vector>
#include <cstddef>

#include <gcrypt.h>

#include "clk_error.h"
#include "algorithm.h"

void AESEncryptionAlgorithm::open() {
    gcry_error_t error = 
            gcry_cipher_open(
                        &handle,
                        GCRY_CIPHER_RIJNDAEL256,
                        GCRY_CIPHER_MODE_CBC,
                        0);

    if (error) {
        throw clk_error(
                clk_error::buildMsg(
                    "Failed to open cipher with gcrypt: %s/%s", 
                    gcry_strerror(error),
                    gcry_strsource(error)),
                __FILE__,
                __LINE__);
    }
}

void AESEncryptionAlgorithm::generateIV() {
    size_t blockSize = getBlockSize();
    iv = (uint8_t *)malloc(blockSize);

    if (iv == NULL) {
        throw clk_error(
                clk_error::buildMsg(
                    "Failed to allocate %u bytes for cipher IV", 
                    blockSize), 
                __FILE__, 
                __LINE__);
    }

    gcry_randomize(iv, blockSize, GCRY_STRONG_RANDOM);

    setIVFromBuffer(iv);
}

void AESEncryptionAlgorithm::setIVFromBuffer(uint8_t * iv) {
    size_t blockSize = getBlockSize();

    gcry_error_t error = gcry_cipher_setiv(handle, iv, blockSize);

    if (error) {
        throw clk_error(
                clk_error::buildMsg(
                    "Failed to set cipher IV with gcrypt: %s/%s", 
                    gcry_strerror(error),
                    gcry_strsource(error)),
                __FILE__,
                __LINE__);
    }
}

void AESEncryptionAlgorithm::setKey(uint8_t * key, size_t keyLength) {
    gcry_error_t error = gcry_cipher_setkey(handle, (void *)key, keyLength);

    if (error) {
        throw clk_error(
                clk_error::buildMsg(
                    "Failed to set cipher key with gcrypt: %s/%s", 
                    gcry_strerror(error),
                    gcry_strsource(error)),
                __FILE__,
                __LINE__);
    }
}

void AESEncryptionAlgorithm::encryptBlock(uint8_t * buffer) {
    size_t blockSize = getBlockSize();

    gcry_error_t error = gcry_cipher_encrypt(handle, buffer, blockSize, NULL, 0);

    if (error) {
        throw clk_error(
                clk_error::buildMsg(
                    "Failed to encrypt block with gcrypt: %s/%s", 
                    gcry_strerror(error),
                    gcry_strsource(error)),
                __FILE__,
                __LINE__);
    }
}

void AESEncryptionAlgorithm::decryptBlock(uint8_t * buffer) {
    size_t blockSize = getBlockSize();
    
    gcry_error_t error = gcry_cipher_decrypt(handle, buffer, blockSize, NULL, 0);

    if (error) {
        throw clk_error(
                clk_error::buildMsg(
                    "Failed to decrypt block with gcrypt: %s/%s", 
                    gcry_strerror(error),
                    gcry_strsource(error)),
                __FILE__,
                __LINE__);
    }
}

AESEncryptionAlgorithm::AESEncryptionAlgorithm() {
    blockNum = 0;

    open();
    generateIV();
}

AESEncryptionAlgorithm::~AESEncryptionAlgorithm() {
    gcry_cipher_close(handle);

    if (iv != NULL) {
        free(iv);
    }
}

size_t AESEncryptionAlgorithm::getBlockSize() {
    return gcry_cipher_get_algo_blklen(GCRY_CIPHER_RIJNDAEL256);
}

uint8_t * AESEncryptionAlgorithm::getIV() {
    return iv;
}

void AESEncryptionAlgorithm::setIV(uint8_t * buffer, size_t bufferLength) {
    size_t blockSize = getBlockSize();

    if (bufferLength < blockSize) {
        throw clk_error("IV buffer length must be at least as big as the cipher block size", __FILE__, __LINE__);
    }

    memcpy(iv, buffer, blockSize);
}

void AESEncryptionAlgorithm::encryptBlock(uint8_t * buffer, size_t bufferLength, uint8_t * key, size_t keyLength) {
    if (isFirstBlock()) {
        setKey(key, keyLength);
    }

    encryptBlock(buffer);

    blockNum++;
}

void AESEncryptionAlgorithm::decryptBlock(uint8_t * buffer, size_t bufferLength, uint8_t * key, size_t keyLength) {
    if (isFirstBlock()) {
        setIVFromBuffer(iv);
        setKey(key, keyLength);
    }

    decryptBlock(buffer);

    blockNum++;
}

void XOREncryptionAlgorithm::encryptBlock(uint8_t * buffer, size_t bufferLength, uint8_t * key, size_t keyLength) {
    for (int i = 0;i < bufferLength;i++) {
	    buffer[i] = buffer[i] ^ (uint8_t)key[i];
    }
}

void XOREncryptionAlgorithm::decryptBlock(uint8_t * buffer, size_t bufferLength, uint8_t * key, size_t keyLength) {
    encryptBlock(buffer, bufferLength, key, keyLength);
}
