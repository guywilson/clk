#include <iostream>
#include <string>
#include <stdio.h>

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

static CloakSecurity getSecurityLevelArg(const std::string & arg) {
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

static AlgorithmType getAlgorithmArg(const std::string & arg) {
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
    std::cout << "Usage: clk [merge|extract] [options] file" << std::endl;
    std::cout << "Hide or extract, an optionally encrypted file in/from the specifed bitmap based host file" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "    -h | -host [host file] - currently supports 24-bit PNG images only" << std::endl;
    std::cout << "    -algo [encryption algorithm] (aes|xor|none)" << std::endl;
    std::cout << "    -sl | -security-level [level] (high|medium|low)" << std::endl;
    std::cout << "    -k | -key [keyfile] for XOR encryption use the keyfile as the key" << std::endl;
    std::cout << "    -g | -generate [keyfile] for XOR encryption, generate and use the keyfile as the key" << std::endl;
    std::cout << "    -c | --capacity report the capacity of the host file and exit" << std::endl;
    std::cout << "    -? | --help show this help and exit" << std::endl;
    std::cout << "    -v | --version print version information and exit" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char ** argv) {
    int defaultLogLevel = LOG_LEVEL_FATAL | LOG_LEVEL_ERROR;
    std::string algo;
    std::string securityLevel;
    std::string operation;
    std::string hostFilename;
    std::string dataFilename;
    std::string keyFilename;
    bool generateKey = false;
    bool reportCapacity = false;

    CmdArg cmdArg = CmdArg(argc, argv);

    if (cmdArg.getNumArgs() == 0) {
        printUsage();
        return -1;
    }

    while (cmdArg.hasMoreArgs()) {
        std::string arg = cmdArg.nextArg();

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
            std::cout << "clk version " << getVersion() << ", build date [" << getBuildDate() << "]" << std::endl << std::endl;
            return 0;
        }
        else if (cmdArg.isLastArg()) {
            dataFilename = arg;
            break;
        }
        else {
            std::cout << "Invalid program argument: Sorry, I do not understand the parameter '" << arg << "'" << std::endl << std::endl;
            printUsage();
            return -1;
        }
    }

    Logger & log = Logger::getInstance();
    log.init("clk.log", defaultLogLevel);

    try {
        PNGReader * reader = new PNGReader();
        reader->open(hostFilename);

        size_t hostCapacity = reader->getCapacity(CLOAKED_LENGTH_BLOCK_SIZE, getSecurityLevelArg(securityLevel));
        
        if (reportCapacity) {
            std::cout <<
                "Host file max capacity at the selected security level is " << 
                std::to_string(hostCapacity) <<
                " bytes." << 
                std::endl;

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
                std::pair<uint8_t *, size_t> keyPair = getKey(algorithm, generateKey, keyFilename, file->size());
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

            std::cout << "Hid '" << dataFilename << "' within host file '" << hostFilename << "'!"<< std::endl;
        }
        else if (operation == OPERATION_EXTRACT) {
            auto file = CloakableFileFactory::createOutputFile(dataFilename, algorithm);

            size_t initBufferSize = file->getInitialisationBlockBufferSize();
            uint8_t * initBuffer = file->getInitialisationBlockBuffer();

            host.extractBlock(reader, initBuffer, initBufferSize);

            file->extractInitialisationBlockFromBuffer(initBuffer);

            if (algorithm != AlgorithmType::no_encryption) {
                std::pair<uint8_t *, size_t> keyPair = getKey(algorithm, generateKey, keyFilename, file->size());
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

            std::cout << "Extracted '" << dataFilename << "' from host file '" << hostFilename << "'!"<< std::endl;
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
        std::cout << "ERROR: caught exception: " << e.what() << std::endl << std::endl;
    }

    log.close();

    return 0;
}
