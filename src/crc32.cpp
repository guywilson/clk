#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "crc32.h"
#include "datafile.h"

void CRC32Helper::initialise()
{
    int         codes;
    int         pos;

    //0x04C11DB7 is the official polynomial used by PKZip, WinZip and Ethernet.
    uint32_t polynomial = 0x04C11DB7;

    memset(&this->table, 0, sizeof(this->table));

    // 256 values representing ASCII character codes.
    for (codes = 0; codes <= 0xFF; codes++) {
        this->table[codes] = this->reflect(codes, 8) << 24;

        for (pos = 0; pos < 8; pos++) {
            this->table[codes] = (this->table[codes] << 1) ^ ((this->table[codes] & (1 << 31)) ? polynomial : 0);
        }

        this->table[codes] = this->reflect(this->table[codes], 32);
    }
}

uint32_t CRC32Helper::reflect(uint32_t reflect, uint8_t c)
{
    int             pos;
    uint32_t        value = 0;

    // Swap bit 0 for bit 7, bit 1 For bit 6, etc....
    for (pos = 1; pos < (c + 1); pos++)
    {
        if (reflect & 1)
        {
            value |= (1 << (c - pos));
        }
        reflect >>= 1;
    }

    return value;
}

uint32_t CRC32Helper::calculateCRC(DataFile * src)
{
    uint8_t *       data;
    uint32_t        dataLength;
    uint32_t        crc = 0xffffffff; //Initilaize the CRC...
    int             i;

    data = src->getData();
    dataLength = src->getDataLength();

    for (i = 0;i < dataLength;i++) {
        crc = (crc >> 8) ^ this->table[(crc & 0xFF) ^ data[i]];
    }

    return crc ^ 0xffffffff; //Finalize the CRC and return.
}
