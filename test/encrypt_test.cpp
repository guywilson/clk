#include <iostream>
#include <stdio.h>
#include <string>
#include <stdint.h>

#include "../src/clk_error.h"
#include "../src/encrypt.h"
#include "../src/datafile.h"
#include "../src/filestream.h"

using namespace std;

bool test_encrypt()
{
    char            key[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
    uint32_t        len = 32;
    string          fname;
    uint8_t *       srcData;
    uint32_t        srcDataLength;
    uint8_t *       decryptedData;
    uint32_t        decryptedDataLength;

    LengthEncodedDataFile * encryptedDf;
    LengthEncodedDataFile * decryptedDf;

    cout << "In test_encrypt()" << endl;

    fname.assign("README.md");

    try {
        FileInputStream is(fname);

        is.open();

        DataFile * src = is.read();

        is.close();

        EncryptionHelper * eh = new EncryptionHelper();

        encryptedDf = eh->encrypt(src, eh->AES_256, (uint8_t *)key, len);

        DataFile * df = new DataFile(encryptedDf->getRawData(), encryptedDf->getRawDataLength());

        decryptedDf = eh->decrypt(df, eh->AES_256, (uint8_t *)key, len);

        src->getData(&srcData, &srcDataLength);
        
        decryptedDf->getData(&decryptedData, &decryptedDataLength);

        // for (int i = 0;i < srcDataLength;i++) {
        //     printf("src '%c' [0x%02X]\tdec '%c' [0x%02X]\n", (char)srcData[i], srcData[i], (char)decryptedData[i], decryptedData[i]);
        // }

        if (memcmp(srcData, decryptedData, srcDataLength) == 0) {
            cout << "DataFiles are equal..." << endl;
            return true;
        }
        else {
            cout << "Datafiles are not equal!" << endl;
            return false;
        }
    }
    catch (clk_error & ce) {
        cout << "Caught clk_error: " << ce.what() << endl;
        exit(-1);
    }
}
