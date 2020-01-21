#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "clk_error.h"
#include "datafile.h"
#include "memutil.h"

DataFile::DataFile(DataFile & src)
{
    uint8_t *       srcData;
    uint32_t        srcDataLength;

    srcData = src.getData();
    srcDataLength = src.getDataLength();

    if (this->_data != NULL) {
        memclr(this->_data, this->_dataLength);
        free(this->_data);
        this->_dataLength = 0;
    }

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
    if (isCopied() && this->_data != NULL) {
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

LengthEncodedDataFile::LengthEncodedDataFile(DataFile & src) : DataFile()
{
    uint8_t *       data;
    uint8_t *       srcData;
    uint32_t        dataLength;
    uint32_t        srcDataLength;

    data = _getData();
    dataLength = _getDataLength();

    srcData = src.getData();
    srcDataLength = src.getDataLength();

    if (data != NULL) {
        memclr(data, dataLength);
        free(data);
        _setDataLength(0);
    }

    dataLength = srcDataLength + sizeof(uint32_t);

    data = (uint8_t *)malloc(dataLength);

    if (data == NULL) {
        throw clk_error("Failed to allocate memory for DataFile", __FILE__, __LINE__);
    }

    memcpy(data, &srcDataLength, sizeof(uint32_t));
    memcpy(&data[sizeof(uint32_t)], srcData, srcDataLength);

    this->_setData(data);
    this->_setDataLength(dataLength);

    this->_setIsCopied(true);
}

LengthEncodedDataFile::LengthEncodedDataFile(uint8_t * data, uint32_t dataLength) : DataFile()
{
    uint8_t *       thisData;
    uint32_t        thisDataLength;

    thisDataLength = dataLength + sizeof(uint32_t);

    thisData = (uint8_t *)malloc(thisDataLength);

    if (thisData == NULL) {
        throw clk_error("Failed to allocate memory for DataFile", __FILE__, __LINE__);
    }

    memcpy(thisData, &dataLength, sizeof(uint32_t));
    memcpy(&thisData[sizeof(uint32_t)], data, dataLength);

    this->_setData(thisData);
    this->_setDataLength(thisDataLength);

    this->_setIsCopied(false);
}
