#include <string>
#include <string.h>
#include <stdlib.h>

#include <gcrypt.h>

#include "logger.h"
#include "cloakable.h"
#include "decryptable.h"
#include "clk_error.h"

void AESDecryptableFile::decryptBlock(uint8_t * buffer, size_t bufferLength) {
    algorithm->decryptBlock(buffer, getBlockSize(), key, keyLength);
}

void XORDecryptableFile::decryptBlock(uint8_t * buffer, size_t bufferLength) {
    log.entry("XORDecryptableFile::decryptBlock()");

    if (keyPointer > keyLength) {
        throw clk_error("Key overrun error", __FILE__, __LINE__);
    }

    size_t blockSize = getBlockSize();

    algorithm->decryptBlock(buffer, blockSize, &key[keyPointer], blockSize);

    keyPointer += blockSize;

    log.exit("XORDecryptableFile::decryptBlock()");
}

void XORDecryptableFile::setKey(uint8_t * key, size_t keyLength) {
    if (keyLength < size()) {
        throw clk_error(
            clk_error::buildMsg(
                "The key must be as least as long as the file (%u bytes) to be encrypted for XOR encryption",
                size()), 
            __FILE__, 
            __LINE__);
    }

    DecryptableFile::setKey(key, keyLength);
}
