#include <iostream>
#include <stdio.h>
#include <string>
#include <stdint.h>

#include "../src/clk_error.h"
#include "../src/cloak.h"
#include "../src/image.h"
#include "../src/ImageStream.h"
#include "../src/datafile.h"
#include "../src/filestream.h"

using namespace std;

bool test_merge()
{
    string          imageInputName = "flowers.png";
    string          imageOutputName = "flowers_copy.png";
    string          secretFileName = "LICENSE";
    string          outputFileName = "LICENSE.copy";

    cout << "In test_merge()" << endl;

    try {
        ImageInputStream is(imageInputName);

        is.open();
        RGB24BitImage * img =  is.read();
        is.close();

        if (img->getFormat() != PNGImage) {
            throw clk_error("Invalid image format read");
        }

        FileInputStream fis(secretFileName);

        fis.open();
        DataFile * secretFile = fis.read();
        fis.close();

        CloakHelper ch;

        RGB24BitImage * outputImage = ch.merge(img, secretFile, ch.High);

        ImageOutputStream os(imageOutputName);

        os.open();
        os.write(outputImage);
        os.close();

        DataFile * outputDataFile = ch.extract(outputImage, ch.High);

        FileOutputStream fos(outputFileName);

        fos.open();
        fos.write(outputDataFile);
        fos.close();

        cout << "Check data files..." << endl;
        
        return true;
    }
    catch (clk_error & ce) {
        cout << "Caught clk_error: " << ce.what() << endl;
        exit(-1);
    }
}
