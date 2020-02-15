#define NAPI_CPP_EXCEPTIONS

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <node_api.h>

#include "clk_error.h"
#include "cloak_api.h"
#include "node_export.h"

using namespace std;

char * getStringArgument(napi_env env, napi_value arg)
{
    char *          pszArg;
    size_t          argLength;
    napi_status     status;

    status = napi_get_value_string_utf8(env, arg, NULL, 0, &argLength);

    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get length of argument");
    }

    pszArg = (char *)malloc(argLength + 1);

    if (pszArg == NULL) {
        napi_throw_error(env, nullptr, "Failed to allocate memory for argument");
    }

    status = napi_get_value_string_utf8(env, arg, pszArg, argLength + 1, &argLength);

    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get string value");
    }

    return pszArg;
}

int32_t getInt32Argument(napi_env env, napi_value arg)
{
    int32_t         argument;
    napi_status     status;

    status = napi_get_value_int32(env, arg, &argument);

    return argument;
}

bool getBoolArgument(napi_env env, napi_value arg)
{
    bool            argument;
    napi_status     status;

    status = napi_get_value_bool(env, arg, &argument);

    return argument;
}

napi_value init(napi_env env, napi_value exports)
{
    napi_status     status;
    napi_value      fnVersion;
    napi_value      fnHide;
    napi_value      fnReveal;
    napi_value      fnFilelength;

    status = napi_create_function(env, "getVersion", 10, cloak_api_version_wrapper, nullptr, &fnVersion);

    if (status != napi_ok) {
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "getVersion", fnVersion);

    if (status != napi_ok) {
        return nullptr;
    }

    status = napi_create_function(env, "getFilelength", 13, cloak_api_filelength_wrapper, nullptr, &fnFilelength);

    if (status != napi_ok) {
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "getFilelength", fnFilelength);

    if (status != napi_ok) {
        return nullptr;
    }

    status = napi_create_function(env, "hide", 4, cloak_api_hide_wrapper, nullptr, &fnHide);

    if (status != napi_ok) {
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "hide", fnHide);

    if (status != napi_ok) {
        return nullptr;
    }

    status = napi_create_function(env, "reveal", 6, cloak_api_reveal_wrapper, nullptr, &fnReveal);

    if (status != napi_ok) {
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "reveal", fnReveal);

    if (status != napi_ok) {
        return nullptr;
    }

    return exports;
}

napi_value cloak_api_version_wrapper(napi_env env, napi_callback_info info)
{
    string          version;
    napi_status     status;
    napi_value      value;

    version = cloak_api_version();

    status = napi_create_string_utf8(env, version.c_str(), version.length(), &value);

    return value;
}

napi_value cloak_api_filelength_wrapper(napi_env env, napi_callback_info info)
{
    uint32_t        fileLength;
    char *          pszFilename;
    size_t          argc = 1;
    napi_value      argv[1];
    napi_status     status;
    napi_value      value;

    status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to parse arguments");
    }

    pszFilename = getStringArgument(env, argv[0]);

    fileLength = cloak_api_filelength(pszFilename);

    status = napi_create_uint32(env, fileLength, &value);

    return value;
}

napi_value cloak_api_hide_wrapper(napi_env env, napi_callback_info info)
{
    napi_status     status;
    napi_value      rtn;
    size_t          argc = 7;
    napi_value      argv[7];
    char *          pszInputImageName;
    char *          pszInputFileName;
    char *          pszOutputImageName;
    char *          pszPassword;
    char *          pszOutputImageFormat;
    char *          pszAlgorithm;
    int             quality;

    status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to parse arguments");
    }

    /*
    ** Fetch the arguments...
    */
    pszInputImageName = getStringArgument(env, argv[0]);
    pszInputFileName = getStringArgument(env, argv[1]);
    pszOutputImageName = getStringArgument(env, argv[2]);
    quality = getInt32Argument(env, argv[3]);
    pszOutputImageFormat = getStringArgument(env, argv[4]);
    pszAlgorithm = getStringArgument(env, argv[5]);
    pszPassword = getStringArgument(env, argv[6]);

    CloakHelper::MergeQuality q = CloakHelper::High;

    switch (quality) {
        case 1:
            q = CloakHelper::High;
            break;

        case 2:
            q = CloakHelper::Medium;
            break;

        case 4:
            q = CloakHelper::Low;
            break;

        default:
            napi_throw_error(env, nullptr, "Invalid merge quality, must be 1, 2 or 4");
    }

    ImageFormat f = PNGImage;

    if (strcmp(pszOutputImageFormat, "BMP") == 0) {
        f = BitmapImage;
    }
    else if (strcmp(pszOutputImageFormat, "PNG") == 0) {
        f = PNGImage;
    }
    else {
        napi_throw_error(env, nullptr, "Invalid output image format, only 'BMP' and 'PNG' are supported");
    }

    EncryptionHelper::Algorithm a = EncryptionHelper::AES_256;

    if (strcmp(pszAlgorithm, "AES-256") == 0) {
        a = EncryptionHelper::AES_256;
    }
    else if (strcmp(pszAlgorithm, "XOR") == 0) {
        a = EncryptionHelper::XOR;
    }
    else {
        napi_throw_error(env, nullptr, "Invalid encryption algorithm, only 'AES-256' and 'XOR' are supported");
    }

    cloak_api_hide(
        pszInputImageName, 
        pszInputFileName, 
        pszOutputImageName, 
        q, 
        f, 
        a, 
        pszPassword);

    free(pszInputImageName);
    free(pszInputFileName);
    free(pszOutputImageName);
    free(pszOutputImageFormat);
    free(pszAlgorithm);
    free(pszPassword);

    status = napi_create_int32(env, 0, &rtn);

    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to set return value");
    }

    return rtn;
}

napi_value cloak_api_reveal_wrapper(napi_env env, napi_callback_info info)
{
    napi_status     status;
    napi_value      rtn;
    size_t          argc = 6;
    napi_value      argv[6];
    char *          pszInputImageName;
    char *          pszOutputFileName;
    char *          pszPassword;
    char *          pszAlgorithm;
    bool            ignoreCRC;
    int             quality;

    status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to parse arguments");
    }

    /*
    ** Fetch the arguments...
    */
    pszInputImageName = getStringArgument(env, argv[0]);
    pszOutputFileName = getStringArgument(env, argv[1]);
    quality = getInt32Argument(env, argv[2]);
    pszAlgorithm = getStringArgument(env, argv[3]);
    ignoreCRC = getBoolArgument(env, argv[4]);
    pszPassword = getStringArgument(env, argv[5]);

    CloakHelper::MergeQuality q = CloakHelper::High;

    switch (quality) {
        case 1:
            q = CloakHelper::High;
            break;

        case 2:
            q = CloakHelper::Medium;
            break;

        case 4:
            q = CloakHelper::Low;
            break;

        default:
            napi_throw_error(env, nullptr, "Invalid merge quality, must be 1, 2 or 4");
    }

    EncryptionHelper::Algorithm a = EncryptionHelper::AES_256;

    if (strcmp(pszAlgorithm, "AES-256") == 0) {
        a = EncryptionHelper::AES_256;
    }
    else if (strcmp(pszAlgorithm, "XOR") == 0) {
        a = EncryptionHelper::XOR;
    }
    else {
        napi_throw_error(env, nullptr, "Invalid encryption algorithm, only 'AES-256' and 'XOR' are supported");
    }

    cloak_api_reveal(
        pszInputImageName, 
        pszOutputFileName, 
        q, 
        a,
        ignoreCRC,
        pszPassword);

    free(pszInputImageName);
    free(pszOutputFileName);
    free(pszAlgorithm);
    free(pszPassword);

    status = napi_create_int32(env, 0, &rtn);

    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to set return value");
    }

    return rtn;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
