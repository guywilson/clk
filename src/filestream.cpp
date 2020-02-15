#include <string>
#include <stdio.h>

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
