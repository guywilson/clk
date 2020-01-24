#include <iostream>
#include <stdio.h>
#include <string>
#include <stdint.h>

#include "../src/clk_error.h"
#include "../src/datafile.h"
#include "../src/filestream.h"

using namespace std;

bool test_read_write()
{
    string          inputFileName = "LICENSE";
    string          outputFileName = "LICENSE.copy";

    cout << "In test_read_write" << endl;

    try {
        FileInputStream is(inputFileName);

        is.open();

        DataFile * src = is.read();

        is.close();

        DataFile * tgt = new DataFile(src);

        FileOutputStream os(outputFileName);

        os.open();

        os.write(tgt);

        os.close();

        cout << "Check data files..." << endl;

        return true;
    }
    catch (clk_error & ce) {
        cout << "Caught clk_error: " << ce.what() << endl;
        exit(-1);
    }
}
