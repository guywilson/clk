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

    protected:
        uint8_t *       _getData() {
            return this->_data;
        }
        uint32_t        _getDataLength() {
            return this->_dataLength;
        }
        void _setData(uint8_t * data) {
            this->_data = data;
        }
        void _setDataLength(uint32_t dataLength) {
            this->_dataLength = dataLength;
        }

        void _setIsCopied(bool isCopied) {
            this->_isCopied = isCopied;
        }

    public:
        DataFile() {}
        DataFile(DataFile & src);
        DataFile(DataFile * src);
        DataFile(uint8_t * data, uint32_t dataLength);

        virtual ~DataFile();

        virtual uint32_t getDataLength() {
            return this->_dataLength;
        }

        virtual uint8_t * getData() {
            return this->_data;
        }
        
        virtual void getData(uint8_t ** data, uint32_t * dataLength) {
            *data = this->_data;
            *dataLength = this->_dataLength;
        }

        bool operator== (DataFile & rhs);
        bool operator!= (DataFile & rhs);

        bool isCopied() {
            return this->_isCopied;
        }
};

class LengthEncodedDataFile : public DataFile
{
    public:
        LengthEncodedDataFile(DataFile & src, uint32_t encodedLength);
        LengthEncodedDataFile(DataFile * src, uint32_t encodedLength);
        LengthEncodedDataFile(uint8_t * data, uint32_t dataLength, uint32_t encodedLength);
        LengthEncodedDataFile(uint8_t * data, uint32_t dataLength);

        static uint32_t extractLength(uint8_t * data);

        uint32_t        getEncodedLength();

        uint8_t *       getRawData();

        uint32_t        getRawDataLength() {
            return getDataLength() - sizeof(uint32_t);
        }
};

#endif
