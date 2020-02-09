
#define NAPI_CPP_EXCEPTIONS

#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <node_api.h>

extern "C" {
#include "version.h"
}

#include "clk_error.h"
#include "memutil.h"
#include "passwordmgr.h"
#include "crc32.h"
#include "filestream.h"
#include "datafile.h"
#include "ImageStream.h"
#include "image.h"
#include "encrypt.h"
#include "cloak.h"
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

    status = napi_create_function(env, "getVersion", 10, cloak_api_version_wrapper, nullptr, &fnVersion);

    if (status != napi_ok) {
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "getVersion", fnVersion);

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

    status = napi_create_int32(env, 0, &rtn);

    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to set return value");
    }

    return rtn;
}


string & cloak_api_version()
{
    static string   version;

    version.assign(getVersion());
    version.append(" [");
    version.append(getBuildDate());
    version.append("]");

    return version;
}

void cloak_api_hide(
        char * pszInputImageName, 
        char * pszInputFileName, 
        char * pszOutputImageName, 
        CloakHelper::MergeQuality quality, 
        ImageFormat outputImageFormat, 
        EncryptionHelper::Algorithm algo,
        char * pszPassword)
{
    uint8_t *           key;
    uint32_t            keyLength;
    EncryptionHelper    encryptionHelper;
    CloakHelper         cloakHelper;
    clk_info_struct     info;
    uint32_t            crc32;

    try {
        keyLength = PasswordManager::getKeyByteLength();

        key = (uint8_t *)malloc(keyLength);

        if (key == NULL) {
            throw clk_error("Failed to allocate memory for key", __FILE__, __LINE__);
        }

        PasswordManager mgr;
        mgr.getKey(key, pszPassword);

        memclr(pszPassword, strlen(pszPassword));

        /*
        ** Open the input image...
        */
        ImageInputStream is(pszInputImageName);

        is.open();
        RGB24BitImage * inputImage = is.read();
        is.close();

        FileInputStream fis(pszInputFileName);

        fis.open();
        DataFile * inputFile = fis.read();
        fis.close();

        /*
        ** Calculate the CRC32 for the data...
        */
        CRC32Helper crcHelper;
        crc32 = crcHelper.calculateCRC(inputFile);

        /*
        ** Step 1: Encrypt the input file using the supplied algorithm...
        */
        DataFile * encrypted =
            encryptionHelper.encrypt(inputFile, algo, key, keyLength);

        /*
        ** Setup the length structure...
        */
        info.originalLength = inputFile->getDataLength();
        info.encryptedLength = encrypted->getDataLength();
        info.crc = crc32;

        cout << "Merge:" << endl;
        cout << "    Original file len: " << info.originalLength << endl;
        cout << "    Encrypted len:     " << info.encryptedLength << endl;
        cout << "    CRC:               " << info.crc << endl << endl;

        /*
        ** Step 2: Merge the encrypted data with the 
        ** source image...
        */
        RGB24BitImage * mergedImage = 
            cloakHelper.merge(inputImage, encrypted, &info, quality);

        ImageOutputStream os(pszOutputImageName);

        if (outputImageFormat == PNGImage && mergedImage->getFormat() == BitmapImage) {
            RGB24BitImage * outImg = new PNG((Bitmap *)mergedImage);

            os.open();
            os.write(outImg);
            os.close();

            delete outImg;
        }
        else if (outputImageFormat == BitmapImage && mergedImage->getFormat() == PNGImage) {
            RGB24BitImage * outImg = new Bitmap((PNG *)mergedImage);

            os.open();
            os.write(outImg);
            os.close();

            delete outImg;
        }
        else {
            os.open();
            os.write(mergedImage);
            os.close();
        }

        delete inputFile;
        delete encrypted;
        delete mergedImage;
        delete inputImage;
    }
    catch (clk_error & e) {
        cout << "Failed to hide file " << pszInputFileName << " in image file " << pszInputImageName << " - " << e.what() << endl;
        throw e;
    }
}

void cloak_api_reveal(
        char * pszInputImageName, 
        char * pszOutputFileName, 
        CloakHelper::MergeQuality quality, 
        EncryptionHelper::Algorithm algo,
        bool ignoreCRC,
        char * pszPassword)
{
    uint8_t *           key;
    uint32_t            keyLength;
    EncryptionHelper    encryptionHelper;
    CloakHelper         cloakHelper;
    clk_info_struct     info;
    uint32_t            crc32;

    try {
        keyLength = PasswordManager::getKeyByteLength();

        key = (uint8_t *)malloc(keyLength);

        if (key == NULL) {
            throw clk_error("Failed to allocate memory for key", __FILE__, __LINE__);
        }
        
        PasswordManager mgr;
        mgr.getKey(key, pszPassword);

        memclr(pszPassword, strlen(pszPassword));

        ImageInputStream is(pszInputImageName);

        /*
        ** Open the input image...
        */
        is.open();
        RGB24BitImage * inputImage = is.read();
        is.close();

        /*
        ** Step 1: Extract the data file from the source image...
        */
        DataFile * extracted = 
            cloakHelper.extract(inputImage, &info, quality);

        cout << "Extract:" << endl;
        cout << "    Original file len: " << info.originalLength << endl;
        cout << "    Encrypted len:     " << info.encryptedLength << endl;

        if (!ignoreCRC) {
            cout << "    CRC:               " << info.crc << endl << endl;
        }
        else {
            cout << endl;
        }

        /*
        ** Step 2: Decrypt the file from step 2 using the supplied algorithm...
        */
        DataFile * outputFile = 
            encryptionHelper.decrypt(extracted, algo, info.originalLength, key, keyLength);

        if (!ignoreCRC) {
            /*
            ** Calculate the CRC32 for the data...
            */
            CRC32Helper crcHelper;
            crc32 = crcHelper.calculateCRC(outputFile);

            if (crc32 != info.crc) {
                throw clk_error(
                    "CRC validation failure. Either the image is corrupted or the password is incorrect...", 
                    __FILE__, 
                    __LINE__);
            }
        }

        FileOutputStream fos(pszOutputFileName);

        fos.open();
        fos.write(outputFile);
        fos.close();

        delete extracted;
        delete outputFile;
        delete inputImage;
    }
    catch (clk_error & e) {
        cout << "Failed to reveal file from image file " << pszInputImageName << " - " << e.what() << endl;
        throw e;
    }
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
