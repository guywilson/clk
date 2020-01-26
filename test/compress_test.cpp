#include <iostream>
#include <stdio.h>
#include <string>
#include <stdint.h>

#include "../src/clk_error.h"
#include "../src/compress.h"
#include "../src/datafile.h"
#include "../src/filestream.h"

using namespace std;

bool test_compress()
{
    uint8_t *       srcData;
    uint32_t        srcDataLength;
    uint8_t *       inflatedData;
    uint32_t        inflatedDataLength;
    string          fname = "LICENSE";

    cout << "In test_compress()" << endl;
    
    FileInputStream is(fname);

    try {
        is.open();

        DataFile * src = is.read();

        is.close();

        src->getData(&srcData, &srcDataLength);

        CompressionHelper c;

        DataFile * compressed = c.compress(src, 7);

        DataFile * inflated = c.inflate(compressed, srcDataLength);

        inflated->getData(&inflatedData, &inflatedDataLength);

        if (memcmp(srcData, inflatedData, srcDataLength) == 0) {
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
