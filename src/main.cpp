#include <iostream>
#include <string>
#include <stdlib.h>

#include "version.h"
#include "clk_error.h"
#include "filestream.h"
#include "datafile.h"
#include "ImageStream.h"
#include "image.h"
#include "encrypt.h"
#include "compress.h"
#include "cloak.h"

#include "../test/testsuite.h"

using namespace std;

void printUsage()
{
    cout << "Usage:" << endl;
    cout << "    cloak -a/e -v -i [infile] -o [outfile] -s [secretfile] -k (keystream file) -b[bits per byte] -l[compression level]" << endl << endl;
    cout << "    Where: -a = 'add' mode to add 'secretfile' to 'infile'" << endl;
    cout << "           -e = 'extract' mode to extract 'outfile' from 'infile'" << endl;
    cout << "           -v = 'verbose' mode, print bitmap header info" << endl;
    cout << "           -bn = Bits per byte, must be 1, 2 or 4" << endl;
    cout << "           -ln = Compression level, must be 0 to 9" << endl;
    cout << "           infile  = an input bitmap" << endl;
    cout << "           outfile = output bitmap (add mode) or extracted file" << endl;
    cout << "           secretfile = secret input file to add to 'infile'" << endl << endl;
	cout.flush();
}

void runTests()
{
    int     passCount = 0;
    int     runCount = 10;

    cout << "Running test suite..." << endl << endl;

    if (test_getKey()) {
        passCount++;
    }
    if (test_encrypt()) {
        passCount++;
    }
    if (test_compress()) {
        passCount++;
    }
    if (test_PNG()) {
        passCount++;
    }
    if (test_Bitmap()) {
        passCount++;
    }
    if (test_copy()) {
        passCount++;
    }
    if (test_read_write()) {
        passCount++;
    }
    if (test_merge()) {
        passCount++;
    }
    if (test_PNGtoBitmap()) {
        passCount++;
    }
    if (test_BitmaptoPNG()) {
        passCount++;
    }

    cout << endl << "Ran " << runCount << " tests, " << passCount << " passed, " << (runCount - passCount) << " failed." << endl;
}

int main(int argc, char **argv)
{
    int                         i;
    CloakHelper::MergeQuality   quality = CloakHelper::High;
    bool                        isMerge = false;
    char *                      arg;
    uint8_t *                   key;
    uint32_t                    keyLength;
    string                      inputImageName;
    string                      outputImageName;
    string                      inputFileName;
    string                      outputFileName;

    for (i = 1;i < argc;i++) {
        arg = argv[i];

        if (arg[0] == '-') {
            if (strncmp(arg, "--help", 6) == 0) {
                printUsage();
                return 0;
            }
            else if (strncmp(arg, "--test", 6) == 0) {
                runTests();
                return 0;
            }
            else if (strncmp(arg, "-f", 2) == 0) {
                inputFileName.assign(argv[i + 1]);
            }
            else if (strncmp(arg, "-o", 2) == 0) {
                outputFileName.assign(argv[i + 1]);
            }
            else if (strncmp(arg, "-q", 2) == 0) {
                int q = atoi(argv[i + 1]);

                switch (q) {
                    case 1:
                        quality = CloakHelper::MergeQuality::High;
                        break;

                    case 2:
                        quality = CloakHelper::MergeQuality::Medium;
                        break;

                    case 4:
                        quality = CloakHelper::MergeQuality::Low;
                        break;

                    default:
                        cout << "Invaliid quality supplied, valid quality values are 1, 2, or 4 bits..." << endl << endl;
                        return -1;
                }
            }
            else {
                cout << "Invalid option (" << arg << ") - clk --help for help" << endl << endl;
                return -1;
            }
        }
    }

    if (inputFileName.length() > 0) {
        outputImageName.assign(outputFileName);
        outputFileName.clear();

        isMerge = true;
    }

    inputImageName = strdup(argv[argc - 1]);

    try {
        ImageInputStream is(inputImageName);

        /*
        ** Open the input image...
        */
        is.open();
        RGB24BitImage * inputImage = is.read();
        is.close();

        EncryptionHelper encryptionHelper;
        CompressionHelper compressionHelper;
        CloakHelper cloakHelper;

        if (isMerge) {
            FileInputStream fis(inputFileName);

            fis.open();
            DataFile * inputFile = fis.read();
            fis.close();

            DataFile * encryptedInputFile = encryptionHelper.encrypt(inputFile, EncryptionHelper::AES_256, key, keyLength);

            DataFile * compressedInputFile = compressionHelper.compress(encryptedInputFile, 6);

            RGB24BitImage * outputImage = cloakHelper.merge(inputImage, compressedInputFile, quality);

            ImageOutputStream os(outputImageName);

            os.open();
            os.write(outputImage);
            os.close();
        }
        else {
            DataFile * encodedOutputFile = cloakHelper.extract(inputImage, quality);

            LengthEncodedDataFile * compressedOutputFile = new LengthEncodedDataFile(encodedOutputFile, 100);

            DataFile * encryptedOutputFile = compressionHelper.inflate(compressedOutputFile, compressedOutputFile->getEncodedLength());

            DataFile * outputFile = encryptionHelper.decrypt(encryptedOutputFile, EncryptionHelper::AES_256, key, keyLength);

            FileOutputStream fos(outputFileName);

            fos.open();
            fos.write(outputFile);
            fos.close();
        }

        cout << "Operation completed successfully..." << endl << endl;
    }
    catch (clk_error & e) {
        cout << "main() : Caught clk_error: " << e.what() << endl << endl;
    }

    return 0;
}
