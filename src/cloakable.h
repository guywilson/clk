#include <string>

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "binary.h"
#include "algorithm.h"
#include "logger.h"
#include "xdump.h"
#include "clk_error.h"

#include "random_block.h"

#ifndef __INCL_CLOAKABLE_FILE
#define __INCL_CLOAKABLE_FILE

typedef uint32_t cloaked_len_t;

#pragma pack(push, 1)
struct LengthBlock {
    /*
    ** The original length of the file to be extracted
    */
    cloaked_len_t originalFileLength;

    /*
    ** The number of bytes bigger the encrypted length vs original,
    ** if originalFileLength is an exact multiple of the blocksize, 
    ** this will be 0 (zero)...
    */
    uint8_t encryptedLengthIncrease;
};
#pragma pack(pop)

#define CLOAKED_LENGTH_BLOCK_SIZE               sizeof(LengthBlock)

class CloakableFile : public BinaryFile {
    protected:
        int blockNum = 0;

        Logger & log = Logger::getInstance();

        int getBlockNum() {
            return blockNum;
        }

        bool isFirstBlock() {
            return blockNum == 0 ? true : false;
        }

        virtual int getInitBlockEncryptionOffset() {
            return 256;
        }

    public:
        virtual ~CloakableFile() {
            close();
        }

        virtual size_t getInitialisationBlockBufferSize() {
            return CLOAKED_LENGTH_BLOCK_SIZE;
        }

        virtual uint8_t * getInitialisationBlockBuffer() {
            log.entry("CloakableFile::getInitialisationBlockBuffer()");

            size_t initialisationBlockSize = getInitialisationBlockBufferSize();
            uint8_t * initialisationBlockBuffer = (uint8_t *)malloc(initialisationBlockSize);

            if (initialisationBlockBuffer == NULL) {
                log.error("Failed to allocate %u bytes for initialisation block", initialisationBlockSize);

                throw clk_error(
                        clk_error::buildMsg(
                            "Failed to allocate %zu bytes for the initialisation block", 
                            initialisationBlockSize), 
                        __FILE__, 
                        __LINE__);
            }

            log.exit("CloakableFile::getInitialisationBlockBuffer()");

            return initialisationBlockBuffer;
        }

        virtual void setKey(uint8_t * key, size_t keyLength) {
            return;
        }

        inline void resetBlockCounter() {
            blockNum = 0;
        }

        virtual inline size_t getBlockSize() {
            return DEFAULT_BLOCK_SIZE;
        }

        virtual uint8_t * getAllocatedBlock() {
            size_t blockSize = getBlockSize();

            uint8_t * block = (uint8_t *)malloc(blockSize);

            if (block == NULL) {
                throw clk_error(
                    clk_error::buildMsg(
                        "Failed to allocate %zu bytes for block",
                        blockSize), 
                    __FILE__, 
                    __LINE__);
            }

            return block;
        }
};

class CloakableInputFile : public CloakableFile {
    private:
        void calculateFileLength();

    protected:
        virtual void addAdditionalInitialisationBlock(uint8_t * initialisationBlockBuffer) {}

        virtual size_t getInitBlockFileSizeDifference() {
            return 0;
        }

    public:
        void open(const std::string & filename) override;

        size_t read(uint8_t * buffer, size_t bufferLength) {
            size_t bytesRead = 0;

            while (hasMoreBlocks()) {
                size_t bytesLeftToRead = bufferLength - bytesRead;

                if (bytesLeftToRead < getBlockSize()) {
                    bytesRead += readBlock(&buffer[bytesRead], bytesLeftToRead);
                }
                else {
                    bytesRead += readBlock(&buffer[bytesRead], getBlockSize());
                }
            }

            return bytesRead;
        }

        virtual size_t readBlock(uint8_t * buffer) {
            return readBlock(buffer, getBlockSize());
        }

        size_t readBlock(uint8_t * buffer, size_t bytesToRead);

        virtual inline bool hasMoreBlocks() {
            return ((getBlockSize() * getBlockNum()) < size() ? true : false);            
        }

        virtual void fillInitialisationBlockBuffer(uint8_t * initialisationBlockBuffer) {
            log.entry("CloakableInputFile::fillInitialisationBlockBuffer()");

            LengthBlock block = {(cloaked_len_t)size(), (uint8_t)getInitBlockFileSizeDifference()};

            log.debug(
                "Length block: originalSize = %zu, encryptedDiff = %zu", 
                (size_t)block.originalFileLength, 
                (size_t)block.encryptedLengthIncrease);

            memcpy(initialisationBlockBuffer, &block, CLOAKED_LENGTH_BLOCK_SIZE);
            addAdditionalInitialisationBlock(initialisationBlockBuffer);

            size_t bufferLength = getInitialisationBlockBufferSize();

            XOREncryptionAlgorithm algo;
            algo.encryptBlock(
                    initialisationBlockBuffer, 
                    bufferLength, 
                    &random_block[getInitBlockEncryptionOffset()], 
                    bufferLength);

            log.exit("CloakableInputFile::fillInitialisationBlockBuffer()");
        }
};

class CloakableOutputFile : public CloakableFile {
    protected:
        size_t bytesLeftToWrite;

        virtual void extractAdditionalInitialisationBlock(uint8_t * initialisationBlockBuffer) {}

    public:
        void open(const std::string & filename) override;

        virtual size_t writeBlock(uint8_t * buffer, size_t bytesToWrite);

        virtual size_t writeBlock(uint8_t * buffer) {
            return writeBlock(buffer, getBytesToWrite());
        }

        inline size_t getBytesLeftToWrite() {
            return bytesLeftToWrite;
        }

        inline size_t getBytesToWrite() {
            return (getBytesLeftToWrite() >= getBlockSize() ? getBlockSize() : getBytesLeftToWrite());
        }

        virtual LengthBlock extractInitialisationBlockFromBuffer(uint8_t * initialisationBlockBuffer) {
            log.entry("CloakableOutputFile::extractInitialisationBlockFromBuffer()");

            size_t bufferLength = getInitialisationBlockBufferSize();

            XOREncryptionAlgorithm algo;
            algo.decryptBlock(
                    initialisationBlockBuffer, 
                    bufferLength, 
                    &random_block[getInitBlockEncryptionOffset()], 
                    bufferLength);

            LengthBlock block;
            memcpy(&block, initialisationBlockBuffer, CLOAKED_LENGTH_BLOCK_SIZE);

            log.debug(
                "Extracted length block: fileLength = %zu, encryptedIncrease = %zu", 
                (size_t)block.originalFileLength, 
                (size_t)block.encryptedLengthIncrease);

            /*
            ** Set the file length so size() works, important so
            ** that checking key length for XOR encrypted files
            ** works.
            */
            fileLength = block.originalFileLength;
            bytesLeftToWrite = block.originalFileLength;

            extractAdditionalInitialisationBlock(initialisationBlockBuffer);

            log.exit("CloakableOutputFile::extractInitialisationBlockFromBuffer()");

            return block;
        }
};

#endif
