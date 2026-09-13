#include <string>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "clk_error.h"
#include "hostfile.h"


void HostFileReader::calculateFileLength() {
    log.entry("HostFileReader::calculateFileLength()");

    if (fptr != NULL) {
        long currentPos = ftell(fptr);
        fseek(fptr, 0, SEEK_END);
        fileLength = (size_t)ftell(fptr);
        fseek(fptr, currentPos, SEEK_SET);
    }

    log.debug("Got file length as %u", fileLength);

    log.exit("HostFileReader::calculateFileLength()");
}

void HostFileReader::open(const std::string & filename) {
    log.entry("HostFileReader::open()");

    fptr = fopen(filename.c_str(), "rb");

    if (fptr == NULL) {
        throw clk_error(
                clk_error::buildMsg(
                    "Failed to open host file '%s' reason '%s'", 
                    filename.c_str(), 
                    strerror(errno)), 
                __FILE__, 
                __LINE__);
    }

    calculateFileLength();
    onOpen();

    log.exit("HostFileReader::open()");
}

void HostFileWriter::open(const std::string & filename) {
    log.entry("HostFileWriter::open()");

    fptr = fopen(filename.c_str(), "wb");

    if (fptr == NULL) {
        throw clk_error(
                clk_error::buildMsg(
                    "Failed to open host file '%s' reason '%s'", 
                    filename.c_str(), 
                    strerror(errno)), 
                __FILE__, 
                __LINE__);
    }

    onOpen();

    log.exit("HostFileWriter::open()");
}
