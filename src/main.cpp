#include <iostream>
#include <string>
#include <stdlib.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

extern "C" {
#include "version.h"
}

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
    cout << "    clk --help (show this help)" << endl;
    cout << "    clk --test (self test)" << endl;
    cout << "    clk --version (show version info)" << endl;
    cout << "    clk [options] source-image" << endl << endl;
    cout << "    options: -o [output file]" << endl;
    cout << "             -of [output image format] BMP or PNG" << endl;
    cout << "             -f [file to cloak]" << endl;
    cout << "             -q [merge quality] either 1, 2, or 4 bits per byte" << endl << endl;
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

    PasswordManager mgr(PasswordManager::Medium);

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
    string                      outputImageFormat;
    ImageFormat                 outputImageFmt;

    if (argc > 1) {
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
                else if (strncmp(arg, "--version", 10) == 0) {
                    cout << "clk v" << getVersion() << " [" << getBuildDate() << "]" << endl << endl;
                    return 0;
                }
                else if (strncmp(arg, "-f", 2) == 0) {
                    inputFileName.assign(argv[i + 1]);
                }
                else if (strncmp(arg, "-of", 3) == 0) {
                    outputImageFormat.assign(argv[i + 1]);

                    if (outputImageFormat.compare("PNG") == 0) {
                        outputImageFmt = PNGImage;
                    }
                    else if (outputImageFormat.compare("BMP") == 0) {
                        outputImageFmt = BitmapImage;
                    }
                    else {
                        cout << "Invalid image format supplied, only BMP and PNG are supported..." << endl << endl;
                        return -1;
                    }
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
    }
    else {
        printUsage();
        return -1;
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
        clk_length_struct ls;

        keyLength = PasswordManager::getKeyByteLength();

        key = (uint8_t *)malloc(keyLength);

        if (key == NULL) {
            throw clk_error("Failed to allocate memory for key", __FILE__, __LINE__);
        }

        /*
        ** Get the key...
        */
        getUserKey(key);

        if (isMerge) {
            FileInputStream fis(inputFileName);

            fis.open();
            DataFile * inputFile = fis.read();
            fis.close();

            /*
            ** Step 1: Encrypt the input (secret) file...
            */
            DataFile * encryptedInputFile = 
                encryptionHelper.encrypt(inputFile, EncryptionHelper::AES_256, key, keyLength);

            /*
            ** Step 2: Compress the encrypted file...
            */
            DataFile * compressedInputFile = 
                compressionHelper.compress(encryptedInputFile, 6);

            /*
            ** Setup the length structure...
            */
            ls.originalLength = inputFile->getDataLength();
            ls.encryptedLength = encryptedInputFile->getDataLength();
            ls.compressedLength = compressedInputFile->getDataLength();

            cout << "Merge: Original file len: " << ls.originalLength << ", encrypted len: " << ls.encryptedLength << ", compressed len: " << ls.compressedLength << endl;

            /*
            ** Step 3: Merge the encrypted & compressed data with the 
            ** source image...
            */
            RGB24BitImage * mergedImage = 
                cloakHelper.merge(inputImage, compressedInputFile, &ls, quality);

            ImageOutputStream os(outputImageName);

            if (outputImageFmt == PNGImage && mergedImage->getFormat() == BitmapImage) {
                RGB24BitImage * outImg = new PNG((Bitmap *)mergedImage);

                os.open();
                os.write(outImg);
                os.close();

                delete outImg;
            }
            else if (outputImageFmt == BitmapImage && mergedImage->getFormat() == PNGImage) {
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
            delete encryptedInputFile;
            delete compressedInputFile;
            delete mergedImage;
        }
        else {
            /*
            ** Step 1: Extract the data file from the source image...
            **
            ** The extracted data file should be encoded with the 
            ** un-compressed length
            */
            DataFile * encodedOutputFile = 
                cloakHelper.extract(inputImage, &ls, quality);

            cout << "Extract: Original file len: " << ls.originalLength << ", encrypted len: " << ls.encryptedLength << ", compressed len: " << ls.compressedLength << endl;

            /*
            ** Step 2: Inflate the input data file...
            **
            ** Output is encoded with the unencrypted length
            */
            DataFile * encryptedOutputFile = 
                compressionHelper.inflate(encodedOutputFile, ls.encryptedLength);

            /*
            ** Step 3: Decrypt the input data file
            **
            ** Output is the raw data that was merged with the image, data is 
            ** encoded with the original file length
            */
            DataFile * outputFile = 
                encryptionHelper.decrypt(encryptedOutputFile, EncryptionHelper::AES_256, ls.originalLength, key, keyLength);

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
