#include <iostream>
#include <stdio.h>
#include <string>
#include <stdint.h>

#include "../src/clk_error.h"
#include "../src/encrypt.h"
#include "../src/datafile.h"
#include "../src/filestream.h"

using namespace std;

void test_encrypt()
{
    char        key[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
    uint32_t    len = 32;
    string      fname;

    printf("In test_encrypt()\n");

    fname.assign("README.md");

    try {
        FileInputStream is(fname);

        is.open();

        DataFile * src = is.read();

        is.close();

        EncryptionHelper * eh = new EncryptionHelper();

        eh->encrypt(src, eh->AES_256, (uint8_t *)key, len);
    }
    catch (clk_error & ce) {
        cout << "Caught clk_error: " << ce.what() << endl;
        exit(-1);
    }
}