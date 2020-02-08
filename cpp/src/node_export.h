#include <iostream>
#include <string>

#include "cloak.h"
#include "image.h"
#include "encrypt.h"

using namespace std;

#ifndef _INCL_NODE_EXPORT
#define _INCL_NODE_EXPORT

void cloak_api_hide(
        string & inputImageName, 
        string & inputFileName, 
        string & outputImageName, 
        CloakHelper::MergeQuality quality, 
        ImageFormat outputImageFormat, 
        EncryptionHelper::Algorithm algo,
        string & password);

void cloak_api_reveal(
        string & inputImageName, 
        string & outputFileName, 
        CloakHelper::MergeQuality quality, 
        EncryptionHelper::Algorithm algo,
        bool ignoreCRC,
        string & password);
        
#endif
