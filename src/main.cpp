#include <iostream>
#include <string>
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <configmgr.h>
#include <logger.h>
#include <sys/types.h>
#include <sys/stat.h>

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
#include "crc32.h"
#include "filestream.h"
#include "datafile.h"
#include "ImageStream.h"
#include "image.h"
#include "encrypt.h"
#include "posixthread.h"
#include "threads.h"
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
    cout << "             -k [keystream file for one-time pad encryption]" << endl;
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
    if (test_calculateCRC()) {
        passCount++;
    }
    if (test_encrypt_AES()) {
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

void daemonise()
{
	pid_t			pid;
	pid_t			sid;

	fprintf(stdout, "Starting daemon...\n");
	fflush(stdout);

	do {
		pid = fork();
	}
	while ((pid == -1) && (errno == EAGAIN));

	if (pid < 0) {
		fprintf(stderr, "Forking daemon failed...\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}
	if (pid > 0) {
		fprintf(stdout, "Exiting child process...\n");
		fflush(stdout);
		exit(EXIT_SUCCESS);
	}

	sid = setsid();
	
	if(sid < 0) {
		fprintf(stderr, "Failed calling setsid()...\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);    
	
	umask(0);

	if((chdir("/") == -1)) {
		fprintf(stderr, "Failed changing directory\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}
	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
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
        EncryptionHelper::Algorithm algo,
        uint8_t * key,
        uint32_t keyLength)
{
    EncryptionHelper    encryptionHelper;
    CloakHelper         cloakHelper;
    clk_info_struct     info;
    uint32_t            crc32;

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
        ** Calculate the CRC32 for the data...
        */
        CRC32Helper crcHelper;
        crc32 = crcHelper.calculateCRC(inputFile);

        /*
        ** Step 1: Encrypt the input file using the supplied algorithm...
        */
        DataFile * encrypted =
            encryptionHelper.encrypt(inputFile, algo, key, keyLength);

        /*
        ** Setup the length structure...
        */
        info.originalLength = inputFile->getDataLength();
        info.encryptedLength = encrypted->getDataLength();
        info.crc = crc32;

        cout << "Merge:" << endl;
        cout << "    Original file len: " << info.originalLength << endl;
        cout << "    Encrypted len:     " << info.encryptedLength << endl;
        cout << "    CRC:               " << info.crc << endl << endl;

        /*
        ** Step 2: Merge the encrypted data with the 
        ** source image...
        */
        RGB24BitImage * mergedImage = 
            cloakHelper.merge(inputImage, encrypted, &info, quality);

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
        EncryptionHelper::Algorithm algo,
        bool ignoreCRC,
        uint8_t * key,
        uint32_t keyLength)
{
    EncryptionHelper    encryptionHelper;
    CloakHelper         cloakHelper;
    clk_info_struct     info;
    uint32_t            crc32;

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
            cloakHelper.extract(inputImage, &info, quality);

        cout << "Extract:" << endl;
        cout << "    Original file len: " << info.originalLength << endl;
        cout << "    Encrypted len:     " << info.encryptedLength << endl;

        if (!ignoreCRC) {
            cout << "    CRC:               " << info.crc << endl << endl;
        }
        else {
            cout << endl;
        }

        /*
        ** Step 2: Decrypt the file from step 2 using the supplied algorithm...
        */
        DataFile * outputFile = 
            encryptionHelper.decrypt(extracted, algo, info.originalLength, key, keyLength);

        if (!ignoreCRC) {
            /*
            ** Calculate the CRC32 for the data...
            */
            CRC32Helper crcHelper;
            crc32 = crcHelper.calculateCRC(outputFile);

            if (crc32 != info.crc) {
                throw clk_error(
                    "CRC validation failure. Either the image is corrupted or the password is incorrect...", 
                    __FILE__, 
                    __LINE__);
            }
        }

        FileOutputStream fos(outputFileName);

        fos.open();
        fos.write(outputFile);
        fos.close();

        delete extracted;
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
    bool                        ignoreCRC = false;
	bool						isDaemon = false;
    char *                      arg;
    uint8_t *                   key = NULL;
    uint32_t                    keyLength;
    string                      inputImageName;
    string                      outputImageName;
    string                      inputFileName;
    string                      outputFileName;
    string                      keystreamFileName;
    string                      outputImageFormat;
	string						listenPort;
	string						webResourceDir;
	string						configFile;
    ImageFormat                 outputImageFmt = UnsupportedFormat;

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
                else if (strncmp(arg, "--ignore-crc", 12) == 0) {
                    ignoreCRC = true;
                }
				else if (strncmp(arg, "--daemon", 8) == 0 || strncmp(arg, "-d", 2) == 0) {
					isDaemon = true;
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
                else if (strncmp(arg, "-k", 2) == 0) {
                    keystreamFileName.assign(argv[i + 1]);
                }
                else if (strncmp(arg, "-o", 2) == 0) {
                    outputFileName.assign(argv[i + 1]);
                }
				else if (strncmp(arg, "-cfg", 4) == 0) {
					configFile.assign(argv[i + 1]);
					cout << "Configured with file " << configFile << endl;
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

	if (isDaemon) {
		daemonise();
		
		ConfigManager & cfg = ConfigManager::getInstance();
		cfg.initialise(configFile.c_str());
		
		Logger & log = Logger::getInstance();
		log.initLogger(cfg.getValue("log.logfilename"), cfg.getValue("log.leglevel"));
		
		log.logDebug("In daemon mode...");
		
		ThreadMgr & threadMgr = ThreadMgr::getInstance();
		
		threadMgr.startThreads();
		
		while (1) {
			PosixThread::sleep(PosixThread::seconds, 1);
		}
	}
	
    if (inputFileName.length() > 0) {
        outputImageName.assign(outputFileName);
        outputFileName.clear();

        isMerge = true;
    }

    inputImageName = strdup(argv[argc - 1]);

    try {
        EncryptionHelper::Algorithm     alg;

        if (keystreamFileName.length() > 0) {
            /*
            ** Encrypt using the supplied keystream file using
            ** the XOR algorithm (e.g. one-time pad encryption)
            */
            FileInputStream ks(keystreamFileName);

            ks.open();
            DataFile * keystreamFile = ks.read();
            ks.close();

            keyLength = keystreamFile->getDataLength();

            key = (uint8_t *)malloc(keyLength);

            if (key == NULL) {
                throw clk_error("Failed to allocate memory for key", __FILE__, __LINE__);
            }

            /*
            ** Copy the key...
            */
            memcpy(key, keystreamFile->getData(), keyLength);

            delete keystreamFile;

            alg = EncryptionHelper::Algorithm::XOR;
        }
        else {
            /*
            ** Encrypt using the AES-256 algorithm in CBC mode...
            */
            keyLength = PasswordManager::getKeyByteLength();

            key = (uint8_t *)malloc(keyLength);

            if (key == NULL) {
                throw clk_error("Failed to allocate memory for key", __FILE__, __LINE__);
            }

            /*
            ** Get the key...
            */
            getUserKey(key);

            alg = EncryptionHelper::Algorithm::AES_256;
        }

        if (isMerge) {
            hide(
                inputImageName, 
                inputFileName, 
                outputImageName, 
                quality, 
                outputImageFmt, 
                alg,
                key, 
                keyLength);
        }
        else {
            reveal(
                inputImageName, 
                outputFileName, 
                quality, 
                alg,
                ignoreCRC,
                key, 
                keyLength);
        }

        /*
        ** Wipe the key from memory...
        */
        memclr(key, keyLength);

        free(key);

        cout << "Operation completed successfully..." << endl << endl;
    }
    catch (clk_error & e) {
        cout << "main() : Caught clk_error: " << e.what() << endl << endl;
    }

    return 0;
}
