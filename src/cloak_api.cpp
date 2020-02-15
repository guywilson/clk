#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
#include "cloak_api.h"

using namespace std;

string & cloak_api_version()
{
    static string   version;

    version.assign(getVersion());
    version.append(" [");
    version.append(getBuildDate());
    version.append("]");

    return version;
}

uint32_t cloak_api_filelength(char * pszFileName)
{
    uint32_t        fileLength;

    try {
        FileInputStream is(pszFileName);

        is.open();
        fileLength = is.getFilelength();
        is.close();
    }
    catch (clk_error & e) {
        cout << "Failed to get file length from " << pszFileName << " - " << e.what() << endl;
        throw e;
    }

    return fileLength;
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
