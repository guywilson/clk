#include <iostream>
#include <stdio.h>
#include <string>
#include <stdint.h>

#include "../src/clk_error.h"
#include "../src/crc32.h"
#include "../src/datafile.h"
#include "../src/filestream.h"

using namespace std;

bool test_calculateCRC()
{
    string          secretFileName = "LICENSE";
    uint32_t        crc;
    uint32_t        verifiedCRC = 2540125440;

    cout << "In test_calculateCRC()" << endl;

    try {
        FileInputStream fis(secretFileName);

        fis.open();
        DataFile * secretFile = fis.read();
        fis.close();

        CRC32Helper crcHelper;

        crc = crcHelper.calculateCRC(secretFile);

        printf("Got CRC 0x%08X, verified CRC = 0x%08X\n", crc, verifiedCRC);

        if (crc == verifiedCRC) {
            cout << "CRC calculated correctly..." << endl;
            return true;
        }
        else {
            throw clk_error("Failed to calculate CRC-32 value", __FILE__, __LINE__);
        }
    }
    catch (clk_error & ce) {
        cout << "Caught clk_error: " << ce.what() << endl;
        exit(-1);
    }
}
