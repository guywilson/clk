#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "clk_error.h"
#include "datafile.h"
#include "memutil.h"

DataFile::DataFile(DataFile & src) : DataFile(&src)
{
}


DataFile::DataFile(DataFile * src)
{
    uint8_t *       srcData;
    uint32_t        srcDataLength;

    srcData = src->getData();
    srcDataLength = src->getDataLength();

    this->_data = (uint8_t *)malloc(srcDataLength);

    if (this->_data == NULL) {
        throw clk_error("Failed to allocate memory for DataFile", __FILE__, __LINE__);
    }

    memcpy(this->_data, srcData, srcDataLength);
    this->_dataLength = srcDataLength;

    this->_isCopied = true;
}

DataFile::DataFile(uint8_t * data, uint32_t dataLength)
{
    this->_data = data;
    this->_dataLength = dataLength;

    this->_isCopied = false;
}

DataFile::~DataFile()
{
    if (this->_data != NULL) {
        memclr(this->_data, this->_dataLength);
        free(this->_data);
        this->_dataLength = 0;
    }
}

bool DataFile::operator== (DataFile& rhs) {
    uint8_t *   lhsData;
    uint8_t *   rhsData;
    uint32_t    lhsLength, rhsLength, compareLength;

    this->getData(&lhsData, &lhsLength);
    rhs.getData(&rhsData, &rhsLength);

    // if (lhsLength != rhsLength) {
    //     return false;
    // }

    if (lhsLength < rhsLength) {
        compareLength = lhsLength;
    }
    else {
        compareLength = rhsLength;
    }

    if (memcmp(lhsData, rhsData, compareLength) != 0) {
        return false;
    }

    return true;
}

bool DataFile::operator!= (DataFile& rhs)
{
    return !(*this == rhs);
}

LengthEncodedDataFile::LengthEncodedDataFile(DataFile & src, uint32_t encodedLength) : LengthEncodedDataFile(&src, encodedLength)
{
}

LengthEncodedDataFile::LengthEncodedDataFile(DataFile * src, uint32_t encodedLength) : DataFile()
{
    uint8_t *       srcData;
    uint8_t *       newData;
    uint32_t        srcDataLength;
    uint32_t        newDataLength;

    srcData = src->getData();
    srcDataLength = src->getDataLength();

    newDataLength = srcDataLength + sizeof(encodedLength);

    newData = (uint8_t *)malloc(newDataLength);

    if (newData == NULL) {
        throw clk_error("Failed to allocate memory for DataFile", __FILE__, __LINE__);
    }

    memcpy(newData, &encodedLength, sizeof(encodedLength));
    memcpy(&newData[sizeof(encodedLength)], srcData, srcDataLength);

    this->_setData(newData);
    this->_setDataLength(newDataLength);

    this->_setIsCopied(true);
}

LengthEncodedDataFile::LengthEncodedDataFile(uint8_t * data, uint32_t dataLength, uint32_t encodedLength) : DataFile()
{
    uint8_t *       newData;
    uint32_t        newDataLength;

    newDataLength = dataLength + sizeof(encodedLength);

    newData = (uint8_t *)malloc(newDataLength);

    if (newData == NULL) {
        throw clk_error("Failed to allocate memory for DataFile", __FILE__, __LINE__);
    }

    memcpy(newData, &encodedLength, sizeof(encodedLength));
    memcpy(&newData[sizeof(encodedLength)], data, dataLength);

    this->_setData(newData);
    this->_setDataLength(newDataLength);

    this->_setIsCopied(false);
}

LengthEncodedDataFile::LengthEncodedDataFile(uint8_t * data, uint32_t dataLength) : DataFile()
{
    uint8_t *       newData;

    newData = (uint8_t *)malloc(dataLength);

    if (newData == NULL) {
        throw clk_error("Failed to allocate memory for DataFile", __FILE__, __LINE__);
    }

    memcpy(newData, data, dataLength);

    this->_setData(newData);
    this->_setDataLength(dataLength);

    this->_setIsCopied(false);
}

uint32_t LengthEncodedDataFile::extractLength(uint8_t * data)
{
    uint32_t    encodedLength;

    memcpy(&encodedLength, data, sizeof(uint32_t));

    return encodedLength;
}

uint32_t LengthEncodedDataFile::getEncodedLength()
{
    return LengthEncodedDataFile::extractLength(this->_getData());
}

uint8_t * LengthEncodedDataFile::getRawData()
{
    uint8_t *       rawData;

    rawData = this->_getData();

    return &rawData[sizeof(uint32_t)];
}
