#include <string>

#include "cloak.h"
#include "encrypt.h"
#include "image.h"

using namespace std;

#ifndef __INCL_REQUEST
#define __INCL_REQUEST

void hide(
        string & inputImageName, 
        string & inputFileName, 
        string & outputImageName, 
        CloakHelper::MergeQuality quality, 
        ImageFormat outputImageFormat, 
        EncryptionHelper::Algorithm algo,
        uint8_t * key,
        uint32_t keyLength,
        int zipLevel);
void reveal(
        string & inputImageName, 
        string & outputFileName, 
        CloakHelper::MergeQuality quality, 
        EncryptionHelper::Algorithm algo,
        bool ignoreCRC,
        uint8_t * key,
        uint32_t keyLength);

#endif
