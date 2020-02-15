#include <iostream>
#include <stdio.h>
#include <string>
#include <stdint.h>

#include "../src/clk_error.h"
#include "../src/image.h"
#include "../src/ImageStream.h"
#include "../src/datafile.h"
#include "../src/filestream.h"

using namespace std;

bool test_PNG()
{
    string          imageInputName = "flowers.png";
    string          imageOutputName = "flowers_copy.png";

    cout << "In test_PNG()" << endl;

    try {
        ImageInputStream is(imageInputName);

        is.open();
        RGB24BitImage * img =  is.read();
        is.close();

        if (img->getFormat() != PNGImage) {
            throw clk_error("Invalid image format read");
        }

        ImageOutputStream os(imageOutputName);

        os.open();
        os.write(img);
        os.close();

        cout << "Check the two image files..." << endl;

        return true;
    }
    catch (clk_error & ce) {
        cout << "Caught clk_error: " << ce.what() << endl;
        exit(-1);
    }
}

bool test_Bitmap()
{
    string          imageInputName = "flowers.bmp";
    string          imageOutputName = "flowers_copy.bmp";

    cout << "In test_Bitmap()" << endl;
    
    try {
        ImageInputStream is(imageInputName);

        is.open();
        RGB24BitImage * img =  is.read();
        is.close();

        if (img->getFormat() != BitmapImage) {
            throw clk_error("Invalid image format read");
        }

        ImageOutputStream os(imageOutputName);

        os.open();
        os.write(img);
        os.close();

        cout << "Check the two image files..." << endl;

        return true;
    }
    catch (clk_error & ce) {
        cout << "Caught clk_error: " << ce.what() << endl;
        exit(-1);
    }
}

bool test_PNGtoBitmap()
{
    string          imageInputName = "flowers.png";
    string          imageOutputName = "flowers_translated.bmp";

    cout << "In test_PNGtoBitmap()" << endl;
    
    try {
        ImageInputStream is(imageInputName);

        is.open();
        PNG * png =  (PNG *)is.read();
        is.close();

        if (png->getFormat() != PNGImage) {
            throw clk_error("Invalid image format read");
        }

        Bitmap * bmp = new Bitmap(png);

        ImageOutputStream os(imageOutputName);

        os.open();
        os.write(bmp);
        os.close();

        cout << "Check the two image files..." << endl;

        return true;
    }
    catch (clk_error & ce) {
        cout << "Caught clk_error: " << ce.what() << endl;
        exit(-1);
    }
}

bool test_BitmaptoPNG()
{
    string          imageInputName = "flowers.bmp";
    string          imageOutputName = "flowers_translated.png";

    cout << "In test_BitmaptoPNG()" << endl;
    
    try {
        ImageInputStream is(imageInputName);

        is.open();
        Bitmap * bmp =  (Bitmap *)is.read();
        is.close();

        if (bmp->getFormat() != BitmapImage) {
            throw clk_error("Invalid image format read");
        }

        PNG * png = new PNG(bmp);

        ImageOutputStream os(imageOutputName);

        os.open();
        os.write(png);
        os.close();

        cout << "Check the two image files..." << endl;

        return true;
    }
    catch (clk_error & ce) {
        cout << "Caught clk_error: " << ce.what() << endl;
        exit(-1);
    }
}
