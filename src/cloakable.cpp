#include <string>

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "binary.h"
#include "cloakable.h"
#include "logger.h"
#include "clk_error.h"

void CloakableInputFile::calculateFileLength() {
    log.entry("CloakableInputFile::calculateFileLength()");

    if (fptr != NULL) {
        long currentPos = ftell(fptr);
        fseek(fptr, 0, SEEK_END);
        fileLength = (size_t)ftell(fptr);
        fseek(fptr, currentPos, SEEK_SET);
    }

    log.debug("Got file length as %u", fileLength);

    log.exit("CloakableInputFile::calculateFileLength()");
}

void CloakableInputFile::open(const std::string & filename) {
    log.entry("CloakableInputFile::open()");

    fptr = fopen(filename.c_str(), "rb");

    if (fptr == NULL) {
        throw clk_error(
                clk_error::buildMsg(
                    "Failed to open input file '%s' : '%s'", 
                    filename.c_str(), 
                    strerror(errno)),
                __FILE__,
                __LINE__);
    }

    calculateFileLength();

    this->fileName = filename;

    log.exit("CloakableInputFile::open()");
}

void CloakableInputFile::open() {
    log.entry("CloakableInputFile::open()");

    fptr = stdin;

    calculateFileLength();

    this->fileName = "standard_input";

    log.exit("CloakableInputFile::open()");
}

size_t CloakableInputFile::readBlock(uint8_t * buffer, size_t bytesToRead) {
    log.entry("CloakableInputFile::readBlock()");

    size_t bytesRead = fread(buffer, sizeof(uint8_t), bytesToRead, fptr);

    log.debug("Read %zu bytes from '%s'", bytesRead, fileName.c_str());

    if (bytesRead < bytesToRead) {
        if (!feof(fptr)) {
            int error = ferror(fptr);

            if (error) {
                throw clk_error(
                        clk_error::buildMsg(
                            "Failed to read %zu bytes from '%s'", 
                            bytesToRead, 
                            fileName.c_str()),
                        __FILE__,
                        __LINE__);
            }
        }
    }

    blockNum++;

    log.exit("CloakableInputFile::readBlock()");

    return bytesRead;
}


void CloakableOutputFile::open(const std::string & filename) {
    log.entry("CloakableOutputFile::open()");

    fptr = fopen(filename.c_str(), "wb");

    if (fptr == NULL) {
        throw clk_error(
                clk_error::buildMsg(
                    "Failed to open output file '%s' : '%s'", 
                    filename.c_str(), 
                    strerror(errno)),
                __FILE__,
                __LINE__);
    }

    this->fileName = filename;

    log.exit("CloakableOutputFile::open()");
}

void CloakableOutputFile::open() {
    log.entry("CloakableOutputFile::open()");

    fptr = stdout;

    this->fileName = "standard_output";

    log.exit("CloakableOutputFile::open()");
}

size_t CloakableOutputFile::writeBlock(uint8_t * buffer, size_t bytesToWrite) {
    log.entry("CloakableOutputFile::writeBlock()");

    size_t bytesWritten = fwrite(buffer, sizeof(uint8_t), bytesToWrite, fptr);

    log.debug("Wrote %zu bytes to file '%s'", bytesWritten, fileName.c_str());

    if (bytesWritten < bytesToWrite) {
        int error = ferror(fptr);

        if (error) {
            throw clk_error(
                    clk_error::buildMsg(
                        "Failed to write %zu bytes to '%s'", 
                        bytesToWrite, 
                        fileName.c_str()),
                    __FILE__,
                    __LINE__);
        }
    }

    log.exit("CloakableOutputFile::writeBlock()");

    return bytesWritten;
}
