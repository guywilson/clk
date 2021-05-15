#include <string>
#include <stdio.h>

#include <zlib.h>

#include "clk_error.h"
#include "datafile.h"
#include "filestream.h"

using namespace std;

void FileInputStream::open()
{
    FILE *      fptr;

    fptr = fopen(this->getFileName().c_str(), "rb");

    if (fptr == NULL) {
        throw clk_error("Failed to open file for reading", __FILE__, __LINE__);
    }

    this->setFilePtr(fptr);
}

uint32_t FileInputStream::getFilelength()
{
    uint32_t        fileLength;

    /*
    ** Find out the length of the file...
    */
    fseek(this->getFilePtr(), 0L, SEEK_END);
    fileLength = ftell(this->getFilePtr());
    fseek(this->getFilePtr(), 0L, SEEK_SET);

    return fileLength;
}

DataFile * FileInputStream::read()
{
    DataFile *      df;
    uint32_t        fileLength;
    uint8_t *       fileData;
    uint32_t        bytesRead;

    fileLength = getFilelength();
    
    fileData = (uint8_t *)malloc(fileLength);

    if (fileData == NULL) {
        throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
    }

    bytesRead = fread(fileData, 1, fileLength, this->getFilePtr());

    if (bytesRead < fileLength) {
        throw clk_error("Failed to read enough data bytes", __FILE__, __LINE__);
    }

    df = new DataFile(fileData, fileLength);

    return df;
}

void FileZippedInputStream::open()
{
    FileInputStream::open();
}

DataFile * FileZippedInputStream::read()
{
    DataFile *      df;
    uint32_t        fileLength;
    uint8_t *       fileData;
    uint8_t *       block;
    uint32_t        bytesDeflated;
    uint32_t        totalBytes = 0;
    z_stream        zipStream;
    int             level = 6;
    int             flush;
    int             zrtn;

    fileLength = getFilelength();
    
    fileData = (uint8_t *)malloc(fileLength);

    if (fileData == NULL) {
        throw clk_error("Failed to allocate memory", __FILE__, __LINE__);
    }

    block = (uint8_t *)malloc(blockSize);

    if (block == NULL) {
        throw clk_error("Failed to allocate memory for block", __FILE__, __LINE__);
    }

    zipStream.zalloc = Z_NULL;
    zipStream.zfree = Z_NULL;
    zipStream.opaque = Z_NULL;

    zrtn = deflateInit(&zipStream, level);

    if (zrtn != Z_OK) {
        throw clk_error("Failed to initialise zip stream", __FILE__, __LINE__);
    }

    do {
        zipStream.avail_in = fread(block, 1, blockSize, this->getFilePtr());

        if (ferror(this->getFilePtr())) {
            deflateEnd(&zipStream);
            throw clk_error("Error reading file", __FILE__, __LINE__);
        }

        flush = feof(this->getFilePtr()) ? Z_FINISH : Z_NO_FLUSH;

        zipStream.next_in = block;

        do {
            zipStream.avail_out = blockSize;
            zipStream.next_out = &fileData[totalBytes];

            zrtn = deflate(&zipStream, flush);

            if (zrtn == Z_STREAM_ERROR) {
                throw clk_error("Error deflating data", __FILE__, __LINE__);
            }

            bytesDeflated = blockSize - zipStream.avail_out;

            totalBytes += bytesDeflated;
        }
        while (zipStream.avail_out == 0);

        if (zipStream.avail_in > 0) {
            throw clk_error("Error deflating data, input still available", __FILE__, __LINE__);
        }
    }
    while (flush != Z_FINISH);

    deflateEnd(&zipStream);

    df = new DataFile(&fileData[0], totalBytes);

    free(block);

    return df;
}

void FileOutputStream::open()
{
    FILE *      fptr;

    fptr = fopen(this->getFileName().c_str(), "wb");

    if (fptr == NULL) {
        throw clk_error("Failed to open file for writing", __FILE__, __LINE__);
    }

    this->setFilePtr(fptr);
}

void FileOutputStream::write(DataFile * df)
{
    uint8_t *       fileData;
    uint32_t        fileLength;
    uint32_t        bytesWritten;

    df->getData(&fileData, &fileLength);

    bytesWritten = fwrite(fileData, 1, fileLength, this->getFilePtr());

    if (bytesWritten < fileLength) {
        throw clk_error(clk_error::buildMsg("Failed to write enough data, expected %u, actual %u", fileLength, bytesWritten), __FILE__, __LINE__);
    }
}

void FileZippedOutputStream::open()
{
    FileOutputStream::open();
}

void FileZippedOutputStream::write(DataFile * df)
{
    uint8_t *       fileData;
    uint32_t        fileLength;
    uint32_t        bytesWritten;
    uint8_t *       block;
    uint32_t        bytesInflated;
    uint32_t        totalBytes = 0;
    z_stream        zipStream;
    int             zrtn;

    df->getData(&fileData, &fileLength);

    block = (uint8_t *)malloc(blockSize);

    if (block == NULL) {
        throw clk_error("Failed to allocate memory for block", __FILE__, __LINE__);
    }

    zipStream.zalloc = Z_NULL;
    zipStream.zfree = Z_NULL;
    zipStream.opaque = Z_NULL;
    zipStream.avail_in = 0;
    zipStream.next_in = Z_NULL;

    zrtn = inflateInit(&zipStream);

    if (zrtn != Z_OK) {
        throw clk_error("Failed to initialise zip stream", __FILE__, __LINE__);
    }

    do {
        zipStream.avail_in = blockSize;
        zipStream.next_in = &fileData[totalBytes];

        do {
            zipStream.avail_out = blockSize;
            zipStream.next_out = block;

            zrtn = inflate(&zipStream, Z_NO_FLUSH);

            if (zrtn == Z_STREAM_ERROR || zrtn == Z_DATA_ERROR || zrtn == Z_MEM_ERROR) {
                inflateEnd(&zipStream);
                throw clk_error("Failed to inflate data", __FILE__, __LINE__);
            }

            bytesInflated = blockSize - zipStream.avail_out;

            totalBytes += bytesInflated;

            bytesWritten = fwrite(block, 1, bytesInflated, this->getFilePtr());

            if (bytesWritten < bytesInflated) {
                inflateEnd(&zipStream);
                throw clk_error(clk_error::buildMsg("Failed to write enough data, expected %u, actual %u", bytesInflated, bytesWritten), __FILE__, __LINE__);
            }
        }
        while (zipStream.avail_out == 0);
    }
    while (zrtn != Z_STREAM_END);

    inflateEnd(&zipStream);

    free(block);
}
