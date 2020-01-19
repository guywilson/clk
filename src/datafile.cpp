#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "datafile.h"
#include "memutil.h"

DataFile::DataFile(DataFile & src)
{
    uint8_t *       srcData;
    uint32_t        srcDataLength;

    src.getData(&srcData, &srcDataLength);

    if (this->_data != NULL) {
        memclr(this->_data, this->_dataLength);
        free(this->_data);
        this->_dataLength = 0;
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
