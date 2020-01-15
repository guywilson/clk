#include <string>
#include <system_error>
#include <cerrno>
#include <stdio.h>

#include "datafile.h"
#include "filestream.h"

using namespace std;

void FileInputStream::open()
{
    FILE *      fptr;

    fptr = fopen(this->getFileName().c_str(), "rb");

    if (fptr == NULL) {
        throw new system_error(make_error_code(errc::no_such_file_or_directory));
    }

    this->setFilePtr(fptr);
}

DataFile * FileInputStream::read()
{
    DataFile *      df;
    uint32_t        fileLength;
    uint8_t *       fileData;
    uint32_t        bytesRead;

    /*
    ** Find out the length of the file...
    */
    fseek(this->getFilePtr(), 0L, SEEK_END);
    fileLength = ftell(this->getFilePtr());
    fseek(this->getFilePtr(), 0L, SEEK_SET);

    fileData = (uint8_t *)malloc(fileLength);

    if (fileData == NULL) {
        throw new system_error(make_error_code(errc::not_enough_memory));
    }

    bytesRead = fread(fileData, 1, fileLength, this->getFilePtr());

    if (bytesRead < fileLength) {
        throw new system_error(make_error_code(errc::io_error));
    }

    df = new DataFile(fileData, fileLength);

    return df;
}

void FileOutputStream::open()
{
    FILE *      fptr;

    fptr = fopen(this->getFileName().c_str(), "wb");

    if (fptr == NULL) {
        throw new system_error(make_error_code(errc::no_such_file_or_directory));
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
        throw new system_error(make_error_code(errc::io_error));
    }
}
