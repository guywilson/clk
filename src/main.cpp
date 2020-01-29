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
    if (test_encrypt_AES()) {
        passCount++;
    }
    if (test_seededXOR()) {
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

void hide(
        string & inputImageName, 
        string & inputFileName, 
        string & outputImageName, 
        CloakHelper::MergeQuality quality, 
        ImageFormat outputImageFormat, 
        uint8_t * key,
        uint32_t keyLength)
{
    EncryptionHelper    encryptionHelper;
    CloakHelper         cloakHelper;
    clk_length_struct   ls;

    try {
        ImageInputStream is(inputImageName);

        /*
        ** Open the input image...
        */
        is.open();
        RGB24BitImage * inputImage = is.read();
        is.close();

        FileInputStream fis(inputFileName);

        fis.open();
        DataFile * inputFile = fis.read();
        fis.close();

        /*
        ** Step 1: Encrypt the input file using AES-256...
        */
        DataFile * encrypted = 
            encryptionHelper.encrypt(inputFile, EncryptionHelper::AES_256, key, keyLength);

        /*
        ** Step 2: Encrypt the file from step 1 using the seeded XOR 
        ** algorithm. Use the image data length as the seed...
        */
        DataFile * xored =
            encryptionHelper.seededXOR(encrypted, inputImage->getDataLength());

        /*
        ** Setup the length structure...
        */
        ls.originalLength = inputFile->getDataLength();
        ls.encryptedLength = encrypted->getDataLength();

        cout << "Merge:" << endl;
        cout << "    Original file len: " << ls.originalLength << endl;
        cout << "    Encrypted len:     " << ls.encryptedLength << endl << endl;

        /*
        ** Step 3: Merge the encrypted data with the 
        ** source image...
        */
        RGB24BitImage * mergedImage = 
            cloakHelper.merge(inputImage, xored, &ls, quality);

        ImageOutputStream os(outputImageName);

        if (outputImageFormat == PNGImage && mergedImage->getFormat() == BitmapImage) {
            RGB24BitImage * outImg = new PNG((Bitmap *)mergedImage);

            os.open();
            os.write(outImg);
            os.close();

            delete outImg;
        }
        else if (outputImageFormat == BitmapImage && mergedImage->getFormat() == PNGImage) {
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
        delete encrypted;
        delete xored;
        delete mergedImage;
        delete inputImage;
    }
    catch (clk_error & e) {
        cout << "Failed to hide file " << inputFileName << " in image file " << inputImageName << " - " << e.what() << endl;
        throw e;
    }
}

void reveal(
        string & inputImageName, 
        string & outputFileName, 
        CloakHelper::MergeQuality quality, 
        uint8_t * key,
        uint32_t keyLength)
{
    EncryptionHelper    encryptionHelper;
    CloakHelper         cloakHelper;
    clk_length_struct   ls;

    try {
        ImageInputStream is(inputImageName);

        /*
        ** Open the input image...
        */
        is.open();
        RGB24BitImage * inputImage = is.read();
        is.close();

        /*
        ** Step 1: Extract the data file from the source image...
        */
        DataFile * extracted = 
            cloakHelper.extract(inputImage, &ls, quality);

        cout << "Extract:" << endl;
        cout << "    Original file len: " << ls.originalLength << endl;
        cout << "    Encrypted len:     " << ls.encryptedLength << endl << endl;

        /*
        ** Step 2: Decrypt the file from step 1 using the seeded XOR 
        ** algorithm. Use the image data length as the seed...
        */
        DataFile * xored =
            encryptionHelper.seededXOR(extracted, inputImage->getDataLength());

        /*
        ** Step 3: Decrypt the file from step 2 using AES-256...
        */
        DataFile * outputFile = 
            encryptionHelper.decrypt(xored, EncryptionHelper::AES_256, ls.originalLength, key, keyLength);

        FileOutputStream fos(outputFileName);

        fos.open();
        fos.write(outputFile);
        fos.close();

        delete extracted;
        delete xored;
        delete outputFile;
        delete inputImage;
    }
    catch (clk_error & e) {
        cout << "Failed to reveal file from image file " << inputImageName << " - " << e.what() << endl;
        throw e;
    }
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
            hide(
                inputImageName, 
                inputFileName, 
                outputImageName, 
                quality, 
                outputImageFmt, 
                key, 
                keyLength);
        }
        else {
            reveal(
                inputImageName, 
                outputFileName, 
                quality, 
                key, 
                keyLength);
        }

        free(key);

        cout << "Operation completed successfully..." << endl << endl;
    }
    catch (clk_error & e) {
        cout << "main() : Caught clk_error: " << e.what() << endl << endl;
    }

    return 0;
}
