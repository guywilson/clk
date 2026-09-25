#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>

#include "logger.h"
#include "cloakable.h"
#include "algorithm.h"
#include "xdump.h"
#include "clk_error.h"

class DecryptableFile : public CloakableOutputFile {
    protected:
        uint8_t * key;
        size_t keyLength;

        EncryptionAlgorithm * algorithm;

        virtual void decryptBlock(uint8_t * buffer, size_t bufferLength) = 0;
    
    public:
        virtual ~DecryptableFile() {
            if (key) {
                free(key);
                key = nullptr;
            }

            delete algorithm;
            algorithm = nullptr;

            close();
        }

        size_t getBlockSize() override {
            return algorithm->getBlockSize();
        }

        size_t writeBlock(uint8_t * buffer, size_t bytesToWrite) override {
            log.entry("DecryptableFile::writeBlock()");

            size_t blockSize = getBlockSize();

            decryptBlock(buffer, blockSize);

            size_t bytesWritten = CloakableOutputFile::writeBlock(buffer, bytesToWrite);
            
            bytesLeftToWrite -= bytesWritten;

            log.exit("DecryptableFile::writeBlock()");

            return bytesWritten;
        }

        virtual void setKey(uint8_t * key, size_t keyLength) override {
            this->keyLength = keyLength;
            this->key = (uint8_t *)malloc(keyLength);

            if (this->key == NULL) {
                throw clk_error(
                    clk_error::buildMsg(
                        "Failed to allocate %u bytes for key",
                        keyLength), 
                    __FILE__, 
                    __LINE__);
            }

            memcpy(this->key, key, keyLength);
        }
};

class AESDecryptableFile : public DecryptableFile {
    protected:
        void decryptBlock(uint8_t * buffer, size_t bufferLength) override;

        void extractAdditionalInitialisationBlock(uint8_t * initialisationBlockBuffer) override {
            log.entry("AESDecryptableFile::extractAdditionalInitialisationBlock()");

            size_t blockSize = algorithm->getBlockSize();

            uint8_t * iv = (uint8_t *)malloc(blockSize);
            memcpy(iv, &initialisationBlockBuffer[sizeof(LengthBlock)], blockSize);

            algorithm->setIV(iv, blockSize);

            log.exit("AESDecryptableFile::extractAdditionalInitialisationBlock()");
        }

        int getInitBlockEncryptionOffset() override {
            return 64;
        }

    public:
        AESDecryptableFile() {
            algorithm = new AESEncryptionAlgorithm();
        }

        size_t getInitialisationBlockBufferSize() override {
            return CloakableFile::getInitialisationBlockBufferSize() + getBlockSize();
        }
};

class XORDecryptableFile : public DecryptableFile {
    private:
        size_t keyPointer;

    protected:
        void decryptBlock(uint8_t * buffer, size_t bufferLength) override;

    public:
        XORDecryptableFile() {
            algorithm = new XOREncryptionAlgorithm();
            keyPointer = 0;
        }

        virtual void setKey(uint8_t * key, size_t keyLength) override;
};
