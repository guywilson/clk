#include <iostream>
#include <string>
#include <stdio.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

#include "filefactory.h"
#include "cloakable.h"
#include "encryptable.h"
#include "decryptable.h"
#include "algorithm.h"
#include "pnghost.h"
#include "pngrw.h"
#include "cloak.h"
#include "logger.h"
#include "cmdarg.h"
#include "key.h"
#include "version.h"
#include "xdump.h"

#define OPERATION_MERGE                 "merge"
#define OPERATION_EXTRACT               "extract"

using namespace std;

static CloakSecurity getSecurityLevelArg(const string & arg) {
    CloakSecurity security;

    if (arg == "high" || arg == "hi") {
        security = CloakSecurity::security_high;
    }
    else if (arg == "medium" || arg == "med") {
        security = CloakSecurity::security_medium;
    }
    else if (arg == "low" || arg == "lo") {
        security = CloakSecurity::security_low;
    }
    else if (arg == "none" || arg == "no") {
        security = CloakSecurity::security_none;
    }
    else if (arg.length() == 0) {
        security = CloakSecurity::security_high;
    }
    else {
        throw clk_error(clk_error::buildMsg("Invalid security level supplied, do not understand '%s'", arg.c_str()));
    }

    return security;
}

static AlgorithmType getAlgorithmArg(const string & arg) {
    AlgorithmType algorithm;

    if (arg == "aes" || arg == "aes256") {
        algorithm = AlgorithmType::aes_encryption;
    }
    else if (arg == "xor" || arg == "otp") {
        algorithm = AlgorithmType::xor_encryption;
    }
    else if (arg == "none" || arg == "no") {
        algorithm = AlgorithmType::no_encryption;
    }
    else if (arg.length() == 0) {
        algorithm = AlgorithmType::no_encryption;
    }
    else {
        throw clk_error(clk_error::buildMsg("Invalid algorithm supplied, do not understand '%s'", arg.c_str()));
    }

    return algorithm;
}

static void printUsage() {
    cout << "Usage: clk [merge|extract] [options] file" << endl;
    cout << "Hide or extract, an optionally encrypted file in/from the specifed bitmap based host file" << endl;
    cout << "options:" << endl;
    cout << "    -h | -host [host file] - currently supports 24-bit PNG images only" << endl;
    cout << "    -algo [encryption algorithm] (aes|xor|none)" << endl;
    cout << "    -sl | -security-level [level] (high|medium|low)" << endl;
    cout << "    -k | -key [keyfile] for XOR encryption use the keyfile as the key" << endl;
    cout << "    -g | -generate [keyfile] for XOR encryption, generate and use the keyfile as the key" << endl;
    cout << "    -c | --capacity report the capacity of the host file and exit" << endl;
    cout << "    -? | --help show this help and exit" << endl;
    cout << "    -v | --version print version information and exit" << endl;
    cout << endl;
}

int main(int argc, char ** argv) {
    int defaultLogLevel = LOG_LEVEL_FATAL | LOG_LEVEL_ERROR;
    string algo;
    string securityLevel;
    string operation;
    string hostFilename;
    string dataFilename;
    string keyFilename;
    bool generateKey = false;
    bool reportCapacity = false;

    CmdArg cmdArg = CmdArg(argc, argv);

#ifndef RUN_IN_DEBUGGER
    if (cmdArg.getNumArgs() > 0) {
        while (cmdArg.hasMoreArgs()) {
            string arg = cmdArg.nextArg();

            if (arg == OPERATION_MERGE || arg == OPERATION_EXTRACT) {
                operation = arg;
            }
            else if (arg == "-algo") {
                algo = cmdArg.nextArg();
            }
            else if (arg =="-security-level" || arg == "-sl") {
                securityLevel = cmdArg.nextArg();
            }
            else if (arg == "-host" || arg == "-h") {
                hostFilename = cmdArg.nextArg();
            }
            else if (arg == "-key" || arg == "-k") {
                keyFilename = cmdArg.nextArg();
            }
            else if (arg == "-generate" || arg == "-g") {
                keyFilename = cmdArg.nextArg();
                generateKey = true;
            }
            else if (arg == "--capacity" || arg == "-c") {
                reportCapacity = true;
            }
            else if (arg == "--help" || arg == "-?") {
                printUsage();
                return 0;
            }
            else if (arg == "--version" || arg == "-v") {
                cout << "clk version " << getVersion() << ", build date [" << getBuildDate() << "]" << endl << endl;
                return 0;
            }
            else if (cmdArg.isLastArg()) {
                dataFilename = arg;
                break;
            }
            else {
                cout << "Invalid program argument: Sorry, I do not understand the parameter '" << arg << "'" << endl << endl;
                printUsage();
                return -1;
            }
        }
    }
    else {
        printUsage();
        return -1;
    }
#else
    operation = "extract";
    algo = "aes256";
    securityLevel = "high";
    hostFilename = "/Users/guy/flowers.png";
    dataFilename = "/Users/guy/out.pptx";
#endif

    Logger & log = Logger::getInstance();
    log.init("clk.log", defaultLogLevel);

    try {
        PNGReader * reader = new PNGReader();
        reader->open(hostFilename);

        size_t hostCapacity = reader->getCapacity(CLOAKED_LENGTH_BLOCK_SIZE, getSecurityLevelArg(securityLevel));
        
        if (reportCapacity) {
            cout << 
                "Host file max capacity at the selected security level is " << 
                to_string(hostCapacity) << 
                " bytes." << 
                endl;

            reader->close();
            delete reader;

            log.close();

            return 0;
        }

        AlgorithmType algorithm = getAlgorithmArg(algo);

        generateKey = (algorithm == AlgorithmType::aes_encryption) ? false : generateKey;

        PNGHost host;
        host.setCloakSecurityLevel(getSecurityLevelArg(securityLevel));

        if (operation == OPERATION_MERGE) {
            auto file = CloakableFileFactory::createInputFile(dataFilename, algorithm);

            size_t initBufferSize = file->getInitialisationBlockBufferSize();
            uint8_t * initBuffer = file->getInitialisationBlockBuffer();

            file->fillInitialisationBlockBuffer(initBuffer);

            hostCapacity = reader->getCapacity(initBufferSize, getSecurityLevelArg(securityLevel));

            if (file->size() > hostCapacity) {
                reader->close();
                delete reader;
                file->close();

                log.close();

                throw clk_error(
                    clk_error::buildMsg(
                        "The selected file '%s' (%zu bytes) is too large for the selected host '%s' (capacity %zu bytes)",
                        dataFilename.c_str(),
                        file->size(),
                        hostFilename.c_str(),
                        hostCapacity), 
                    __FILE__, __LINE__);
            }

            // Add initialisation block to image...
            host.addBlock(reader, initBuffer, initBufferSize);

            if (algorithm != AlgorithmType::no_encryption) {
                pair<uint8_t *, size_t> keyPair = getKey(algorithm, generateKey, keyFilename, file->size());
                file->setKey(keyPair.first, keyPair.second);
            }

            size_t blockSize = file->getBlockSize();
            uint8_t * buffer = file->getAllocatedBlock();

            while (file->hasMoreBlocks()) {
                file->readBlock(buffer);
                host.addBlock(reader, buffer, blockSize);
            }

            file->close();

            PNGWriter writer;
            writer.assignImageDetails(reader->getPNGDetails());

            reader->close();

            writer.open(hostFilename);
            writer.close();

            cout << "Hid '" << dataFilename << "' within host file '" << hostFilename << "'!"<< endl;
        }
        else if (operation == OPERATION_EXTRACT) {
            auto file = CloakableFileFactory::createOutputFile(dataFilename, algorithm);

            size_t initBufferSize = file->getInitialisationBlockBufferSize();
            uint8_t * initBuffer = file->getInitialisationBlockBuffer();

            host.extractBlock(reader, initBuffer, initBufferSize);

            file->extractInitialisationBlockFromBuffer(initBuffer);

            if (algorithm != AlgorithmType::no_encryption) {
                pair<uint8_t *, size_t> keyPair = getKey(algorithm, generateKey, keyFilename, file->size());
                file->setKey(keyPair.first, keyPair.second);
            }

            uint8_t * buffer = file->getAllocatedBlock();
            size_t blockSize = file->getBlockSize();

            while (file->getBytesLeftToWrite() > 0) {
                host.extractBlock(reader, buffer, blockSize);
                file->writeBlock(buffer);
            }

            file->close();
            reader->close();

            cout << "Extracted '" << dataFilename << "' from host file '" << hostFilename << "'!"<< endl;
        }
        else {
            reader->close();
            delete reader;

            throw clk_error(
                    clk_error::buildMsg(
                        "Invalid operation supplied '%s'", 
                        operation.c_str()));
        }

        delete reader;
    }
    catch (clk_error & e) {
        cout << "ERROR: caught exception: " << e.what() << endl << endl;
    }

    log.close();

    return 0;
}
