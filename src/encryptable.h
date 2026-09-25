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

#include "random_block.h"

class EncryptableFile : public CloakableInputFile {
    protected:
        uint8_t * key;
        size_t keyLength;

        EncryptionAlgorithm * algorithm = nullptr;

        virtual void encryptBlock(uint8_t * buffer, size_t bufferLength) = 0;
    
    public:
        virtual ~EncryptableFile() {
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

        size_t readBlock(uint8_t * buffer) override {
            size_t blockSize = getBlockSize();

            memset(buffer, 0, blockSize);

            size_t bytesRead = CloakableInputFile::readBlock(buffer);

            encryptBlock(buffer, blockSize);

            return bytesRead;
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

class AESEncryptableFile : public EncryptableFile {
    protected:
        void encryptBlock(uint8_t * buffer, size_t bufferLength) override;

        void addAdditionalInitialisationBlock(uint8_t * initialisationBlockBuffer) override {
            uint8_t * iv = algorithm->getIV();
            memcpy(&initialisationBlockBuffer[CLOAKED_LENGTH_BLOCK_SIZE], iv, getBlockSize());
        }

        int getInitBlockEncryptionOffset() override {
            return 64;
        }

        size_t getInitBlockFileSizeDifference() override {
            size_t blockSize = algorithm->getBlockSize();
            size_t difference = 0;

            size_t remainder = size() % blockSize;

            if (remainder) {
                difference = blockSize - remainder;
            }

            return difference;
        }

    public:
        AESEncryptableFile() {
            algorithm = new AESEncryptionAlgorithm();
        }

        size_t getInitialisationBlockBufferSize() override {
            return CloakableFile::getInitialisationBlockBufferSize() + getBlockSize();
        }
};

class XOREncryptableFile : public EncryptableFile {
    private:
        size_t keyPointer;

    protected:
        void encryptBlock(uint8_t * buffer, size_t bufferLength) override;

    public:
        XOREncryptableFile() {
            algorithm = new XOREncryptionAlgorithm();
            keyPointer = 0;
        }

        virtual void setKey(uint8_t * key, size_t keyLength) override;
};
