#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <system_error>
#include <cerrno>

#include "image.h"

using namespace std;

void PNG::getHeader(uint8_t ** data, uint32_t * dataLength)
{
    throw new system_error(make_error_code(errc::not_supported));
}

void Bitmap::getHeader(uint8_t ** header, uint32_t * headerLen)
{
    uint8_t *       buffer;

    if (type == WindowsV3) {
        *headerLen = BMP_HEADER_SIZE + WINV3_HEADER_SIZE;
        *header = (uint8_t *)malloc(*headerLen);

        if (*header == NULL) {
            throw new system_error(make_error_code(errc::not_enough_memory));
        }

        buffer = *header;

        uint32_t fileLen = this->getDataLength() + BMP_HEADER_SIZE + WINV3_HEADER_SIZE;
        uint32_t startOffset = BMP_HEADER_SIZE + WINV3_HEADER_SIZE;

        buffer[0] = 'B';
        buffer[1] = 'M';
        memcpy(&buffer[2], &fileLen, sizeof(uint32_t));
        memset(&buffer[6], 0, 4);
        memcpy(&buffer[10], &startOffset, sizeof(uint32_t));

        
    }
    else if (type == OS2V1) {
        *headerLen = BMP_HEADER_SIZE + OS2V1_HEADER_SIZE;
        *header = (uint8_t *)malloc(*headerLen);

    }
    else {
        throw new system_error(make_error_code(errc::not_supported));
    }
}