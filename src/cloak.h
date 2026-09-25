#pragma once

#include <string>
#include <stdint.h>

enum class CloakSecurity {
    security_high = 1,
    security_medium = 2,
    security_low = 4,
    security_none = 8
};

inline uint8_t getBitMask(const CloakSecurity & security) {
	static uint8_t mask[9] = {0x00, 0x01, 0x03, 0x00, 0x0F, 0x00, 0x00, 0x00, 0xFF};
	return mask[(int)security];

    /*
    ** The above lines replaces the code below
    */
    // uint8_t mask = 0x00;

	// switch (security) {
	// 	case CloakSecurity::security_high:
	// 		mask = 0x01;
	// 		break;

	// 	case CloakSecurity::security_medium:
	// 		mask = 0x03;
	// 		break;

	// 	case CloakSecurity::security_low:
	// 		mask = 0x0F;
	// 		break;

	// 	case CloakSecurity::security_none:
	// 		mask = 0xFF;
	// 		break;
	// }

    // return mask;
}

class CloakAlgorithm {
    private:
        // static void mergeByte(uint8_t * imageData, size_t imageDataLen, uint8_t dataByte, CloakSecurity & security);
        // static uint8_t extractByte(uint8_t * imageData, size_t imageDataLen, CloakSecurity & security);

    public:
        static size_t calculateBlockHostBytesRequired(size_t blockSize, const CloakSecurity & security) {
            return ((blockSize * (size_t)8) / (size_t)security);
        }

        static void mergeByte(uint8_t * imageData, size_t imageDataLen, uint8_t dataByte, const CloakSecurity & security);
        static uint8_t extractByte(uint8_t * imageData, size_t imageDataLen, const CloakSecurity & security);

        static void mergeBlock(
                        uint8_t * imageData, 
                        uint8_t * dataBlock, 
                        size_t blockLength, 
                        const CloakSecurity & security);

        static void extractBlock(
                        uint8_t * imageData, 
                        uint8_t * dataBlock, 
                        size_t blockLength, 
                        const CloakSecurity & security);
};
