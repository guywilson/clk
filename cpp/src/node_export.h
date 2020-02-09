#include <iostream>
#include <string>

#include "cloak.h"
#include "image.h"
#include "encrypt.h"

using namespace std;

#ifndef _INCL_NODE_EXPORT
#define _INCL_NODE_EXPORT

napi_value cloak_api_version_wrapper(napi_env env, napi_callback_info info);
napi_value cloak_api_hide_wrapper(napi_env env, napi_callback_info info);
napi_value cloak_api_reveal_wrapper(napi_env env, napi_callback_info info);

string & cloak_api_version();

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
