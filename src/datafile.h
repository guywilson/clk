#include <stdint.h>
#include <string.h>

#ifndef _INCL_DATAFILE
#define _INCL_DATAFILE

class DataFile
{
    private:
        uint8_t *       _data;
        uint32_t        _dataLength;

        bool            _isCopied;

    public:
        DataFile(DataFile & src);
        DataFile(uint8_t * data, uint32_t dataLength);

        ~DataFile();

        uint32_t        getDataLength() {
            return this->_dataLength;
        }

        uint8_t *       getData() {
            return this->_data;
        }
        
        void            getData(uint8_t ** data, uint32_t * dataLength) {
            *data = this->_data;
            *dataLength = this->_dataLength;
        }

        bool operator== (DataFile & rhs);
        bool operator!= (DataFile & rhs);

        bool            isCopied() {
            return this->_isCopied;
        }
};

#endif
