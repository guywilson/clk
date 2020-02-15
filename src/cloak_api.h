#include <string>

#include "cloak.h"
#include "image.h"
#include "encrypt.h"

using namespace std;

#ifndef _INCL_CLOAK_API
#define _INCL_CLOAK_API

string & cloak_api_version();

uint32_t cloak_api_filelength(char * pszFileName);

void cloak_api_hide(
        char * pszInputImageName, 
        char * pszInputFileName, 
        char * pszOutputImageName, 
        CloakHelper::MergeQuality quality, 
        ImageFormat outputImageFormat, 
        EncryptionHelper::Algorithm algo,
        char * pszPassword);

void cloak_api_reveal(
        char * pszInputImageName, 
        char * pszOutputFileName, 
        CloakHelper::MergeQuality quality, 
        EncryptionHelper::Algorithm algo,
        bool ignoreCRC,
        char * pszPassword);

#endif
