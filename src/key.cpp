#include <iostream>
#include <string>
#include <stdio.h>

#include <gcrypt.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

#include "cloakable.h"
#include "algorithm.h"
#include "clk_error.h"
#include "key.h"

#define MAX_PASSWORD_LENGTH             256

using namespace std;

static int __getch(void) {
	int		ch;

#ifndef _WIN32
	struct termios current;
	struct termios original;

	tcgetattr(fileno(stdin), &original); /* grab old terminal i/o settings */
	current = original; /* make new settings same as old settings */
	current.c_lflag &= ~ICANON; /* disable buffered i/o */
	current.c_lflag &= ~ECHO; /* set echo mode */
	tcsetattr(fileno(stdin), TCSANOW, &current); /* use these new terminal i/o settings now */
#endif

#ifdef _WIN32
    ch = _getch();
#else
    ch = getchar();
#endif

#ifndef _WIN32
	tcsetattr(0, TCSANOW, &original);
#endif

    return ch;
}

static pair<uint8_t *, size_t> getKeyFromUser() {
    size_t keyBufferLength = (size_t)gcry_md_get_algo_dlen(GCRY_MD_SHA3_256);

    uint8_t * keyBuffer = (uint8_t *)malloc(keyBufferLength);

    if (keyBuffer == NULL) {
        throw clk_error(
                clk_error::buildMsg(
                    "Failed to allocate %zu bytes for the key", 
                    keyBufferLength), 
                __FILE__, 
                __LINE__);
    }

    string password;
	int i = 0;
    int ch = 0;

    cout << "Enter password: ";

    while (ch != '\n') {
        ch = __getch();

        if (ch == EOF) {
            break;
        }

        if (i == MAX_PASSWORD_LENGTH - 1) {
            break;
        }

        if (ch != '\n' && ch != '\r') {
            password += (char)ch;
        }

        i++;
    }

    std::cout << std::endl;
    fflush(stdout);

	gcry_md_hash_buffer(GCRY_MD_SHA3_256, keyBuffer, password.c_str(), password.length());
    password.clear();

    pair<uint8_t *, size_t> key = {keyBuffer, keyBufferLength};

    return key;
}

static pair<uint8_t *, size_t> getKeyFromFile(const string & keyFilename) {
    CloakableInputFile keyFile;
    keyFile.open(keyFilename);

    size_t keyBufferLength = keyFile.size();

    uint8_t * keyBuffer = (uint8_t *)malloc(keyBufferLength);

    if (keyBuffer == NULL) {
        keyFile.close();

        throw clk_error(
                clk_error::buildMsg(
                    "Failed to allocate %zu bytes for the key", 
                    keyBufferLength), 
                __FILE__, 
                __LINE__);
    }

    keyFile.read(keyBuffer, keyBufferLength);

    keyFile.close();

    pair<uint8_t *, size_t> key = {keyBuffer, keyBufferLength};

    return key;
}

static pair<uint8_t *, size_t> generateOTP(const string & keyFilename, size_t keyLength) {
    uint8_t * keyBuffer = (uint8_t *)malloc(keyLength);

    if (keyBuffer == NULL) {
        throw clk_error(
                clk_error::buildMsg(
                    "Failed to allocate %zu bytes for the key", 
                    keyLength), 
                __FILE__, 
                __LINE__);
    }

    FILE * fptrRand = fopen("/dev/random", "rb");

    if (fptrRand == NULL) {
        throw clk_fatal("Failed to open random device for reading", __FILE__, __LINE__);
    }

    FILE *fptrOutput = fopen(keyFilename.c_str(), "wb");

    if (fptrOutput == NULL) {
        throw clk_error(
                clk_error::buildMsg(
                    "Failed to open otp output file '%s' for writing", 
                    keyFilename.c_str()),
                __FILE__,
                __LINE__);
    }

    cout << "Wrote " << to_string(keyLength) << " random bytes to '" << keyFilename << "'" << endl << endl;

    fread(keyBuffer, sizeof(uint8_t), keyLength, fptrRand);
    fwrite(keyBuffer, sizeof(uint8_t), keyLength, fptrOutput);

    fclose(fptrRand);
    fclose(fptrOutput);

    pair<uint8_t *, size_t> key = {keyBuffer, keyLength};

    return key;
}

pair<uint8_t *, size_t> getKey(AlgorithmType & algorithm, bool generateKey, const string & keyFilename, size_t keyLength) {
    pair<uint8_t *, size_t> keyPair;

    if (!generateKey) {
        if (algorithm == AlgorithmType::aes_encryption) {
            keyPair = getKeyFromUser();
        }
        else if (algorithm == AlgorithmType::xor_encryption) {
            keyPair = getKeyFromFile(keyFilename);
        }
    }
    else {
        if (algorithm == AlgorithmType::xor_encryption) {
            keyPair = generateOTP(keyFilename, keyLength);
        }
    }

    return keyPair;
}
