#include <iostream>
#include <string>
#include <stdlib.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

#include "version.h"
#include "clk_error.h"
#include "memutil.h"
#include "passwordmgr.h"
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

/*
** Under windows, this uses getch(), under a different OS
** getch() probably won't be available, so you will have
** to use a different mechanism...
*/
void getpwd(char * pszPassword, int maxLen)
{
	int		ch;
	int		i;

#ifndef _WIN32
	struct termios current;
	struct termios original;

	tcgetattr(fileno(stdin), &original); /* grab old terminal i/o settings */
	current = original; /* make new settings same as old settings */
	current.c_lflag &= ~ICANON; /* disable buffered i/o */
	current.c_lflag &= ~ECHO; /* set echo mode */
	tcsetattr(fileno(stdin), TCSANOW, &current); /* use these new terminal i/o settings now */
#endif

	i = 0;
	ch = -1;

	while (ch != 0) {
	#ifdef _WIN32
		ch = _getch();
	#else
		ch = getchar();
	#endif

		if (ch != '\n' && ch != '\r') {
			if (i < maxLen) {
				cout << "*";
				cout.flush();
				pszPassword[i++] = (char)ch;
			}
			else {
				throw clk_error(clk_error::buildMsg("The password is too long, must be < %d", maxLen), __FILE__, __LINE__);
			}
		}
		else {
			cout << endl;
			cout.flush();
            ch = 0;
			pszPassword[i] = ch;
		}
	}

#ifndef _WIN32
	tcsetattr(0, TCSANOW, &original);
#endif
}

int getUserKey(uint8_t * key)
{
    char    szPassword[256];

	cout << "Enter password: ";
	cout.flush();

	getpwd(szPassword, 256);

    PasswordManager mgr;

    mgr.getKey(key, szPassword);

    memclr(szPassword, strlen(szPassword));

	return 0;
}

int main(int argc, char **argv)
{
    int                         i;
    CloakHelper::MergeQuality   quality = CloakHelper::High;
    bool                        isMerge = false;
    char *                      arg;
    uint8_t *                   key = NULL;
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

    cout << "Input image: " << inputImageName << " input file: " << inputFileName << " output image name: " << outputImageName << endl;
    cout.flush();

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

        keyLength = PasswordManager::getKeyLength();

        key = (uint8_t *)malloc(keyLength);

        if (key == NULL) {
            throw clk_error("Failed to allocate memory for key", __FILE__, __LINE__);
        }

        getUserKey(key);

        if (isMerge) {
            FileInputStream fis(inputFileName);

            fis.open();
            DataFile * inputFile = fis.read();
            fis.close();

            /*
            ** Step 1: Encrypt the input (secret) file...
            **
            ** Output is encoded with the original file length
            */
            LengthEncodedDataFile * encryptedInputFile = 
                encryptionHelper.encrypt(inputFile, EncryptionHelper::AES_256, key, keyLength);

            /*
            ** Step 2: Compress the encrypted file...
            **
            ** Output is encoded with the file length from step 1
            */
            LengthEncodedDataFile * compressedInputFile = 
                compressionHelper.compress(encryptedInputFile, 6);

            /*
            ** Step 3: Merge the encrypted & compressed data with the 
            ** source image...
            */
            RGB24BitImage * outputImage = 
                cloakHelper.merge(inputImage, compressedInputFile, quality);

            ImageOutputStream os(outputImageName);

            os.open();
            os.write(outputImage);
            os.close();

            delete inputFile;
            delete encryptedInputFile;
            delete compressedInputFile;
            delete outputImage;
        }
        else {
            /*
            ** Step 1: Extract the data file from the source image...
            **
            ** The extracted data file should be encoded with the 
            ** un-compressed length
            */
            LengthEncodedDataFile * encodedOutputFile = 
                cloakHelper.extract(inputImage, quality);

            /*
            ** Step 2: Inflate the input data file...
            **
            ** Output is encoded with the unencrypted length
            */
            printf("Output length = %u\n", encodedOutputFile->getEncodedLength());
                       
            LengthEncodedDataFile * encryptedOutputFile = 
                compressionHelper.inflate(encodedOutputFile, encodedOutputFile->getEncodedLength());

            /*
            ** Step 3: Decrypt the input data file
            **
            ** Output is the raw data that was merged with the image, data is 
            ** encoded with the original file length
            */
            DataFile * outputFile = 
                encryptionHelper.decrypt(encryptedOutputFile, EncryptionHelper::AES_256, key, keyLength);

            FileOutputStream fos(outputFileName);

            fos.open();
            fos.write(outputFile);
            fos.close();

            delete encodedOutputFile;
            delete encryptedOutputFile;
            delete outputFile;
        }

        free(key);

        delete inputImage;

        cout << "Operation completed successfully..." << endl << endl;
    }
    catch (clk_error & e) {
        cout << "main() : Caught clk_error: " << e.what() << endl << endl;
    }

    return 0;
}
