#include <string>
#include <string.h>
#include <stdlib.h>

#include <gcrypt.h>

#include "logger.h"
#include "cloakable.h"
#include "encryptable.h"
#include "clk_error.h"

void AESEncryptableFile::encryptBlock(uint8_t * buffer, size_t bufferLength) {
    algorithm->encryptBlock(buffer, getBlockSize(), key, keyLength);
}

void XOREncryptableFile::encryptBlock(uint8_t * buffer, size_t bufferLength) {
    if (keyPointer > keyLength) {
        throw clk_error("Key overrun error", __FILE__, __LINE__);
    }

    size_t blockSize = getBlockSize();

    algorithm->encryptBlock(buffer, blockSize, &key[keyPointer], blockSize);

    keyPointer += blockSize;
}

void XOREncryptableFile::setKey(uint8_t * key, size_t keyLength) {
    if (keyLength < size()) {
        throw clk_error(
            clk_error::buildMsg(
                "The key must be as least as long as the file (%u bytes) to be encrypted for XOR encryption",
                size()), 
            __FILE__, 
            __LINE__);
    }

    EncryptableFile::setKey(key, keyLength);
}
