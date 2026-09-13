#include <string>
#include <memory>

#include "logger.h"
#include "binary.h"
#include "cloak.h"

#ifndef __INCL_HOSTFILE
#define __INCL_HOSTFILE

class HostFileReader : public BinaryFile {
    private:
        void calculateFileLength();

    protected:
        Logger & log = Logger::getInstance();

        virtual void onOpen() {}
        virtual void onClose() {}

    public:
        virtual ~HostFileReader() {
            close();
        }

        virtual size_t getCapacity(size_t headerSize, const CloakSecurity & securityLevel) = 0;

        virtual void open(const std::string & filename) override;
        virtual void close() override {
            onClose();
            BinaryFile::close();
        }

        virtual void readBlock(uint8_t * buffer, size_t bufferLength) = 0;
        virtual void * getData() = 0;
};

class HostFileWriter : public BinaryFile {
    protected:
        Logger & log = Logger::getInstance();

        virtual void onOpen() {}
        virtual void onClose() {}

    public:
        virtual ~HostFileWriter() {
            close();
        }

        virtual void open(const std::string & filename) override;
        virtual void close() override {
            onClose();
            BinaryFile::close();
        }

        virtual void writeBlock(uint8_t * buffer, size_t bufferLength) = 0;
};

class HostFile {
    protected:
        Logger & log = Logger::getInstance();

        CloakSecurity security;

    public:
        void setCloakSecurityLevel(const CloakSecurity & securityLevel) {
            security = securityLevel;
        }

        virtual void addBlock(
                        HostFileReader * reader,
                        uint8_t * sourceBlock, 
                        size_t sourceBlockSize) = 0;

        virtual void extractBlock(
                        HostFileReader * reader,
                        uint8_t * targetBlock, 
                        size_t targetBlockSize) = 0;
};

#endif