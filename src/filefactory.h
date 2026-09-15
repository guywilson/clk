#include <string>
#include <memory>

#include "algorithm.h"
#include "cloakable.h"
#include "encryptable.h"
#include "decryptable.h"
#include "logger.h"

#ifndef __INCL_FILEFACTORY
#define __INCL_FILEFACTORY

class CloakableFileFactory {
    public:
        static std::unique_ptr<CloakableInputFile> createInputFile(const std::string & filename, const AlgorithmType & type) {
            if (type == AlgorithmType::aes_encryption) {
                std::unique_ptr<EncryptableFile> file = std::make_unique<AESEncryptableFile>();
                file->open(filename);
                return file;
            }
            else if (type == AlgorithmType::xor_encryption) {
                std::unique_ptr<EncryptableFile> file = std::make_unique<XOREncryptableFile>();
                file->open(filename);
                return file;
            }
            else if (type == AlgorithmType::no_encryption) {
                std::unique_ptr<CloakableInputFile> file = std::make_unique<CloakableInputFile>();
                file->open(filename);
                return file;
            }

            return nullptr;
        }

        static std::unique_ptr<CloakableInputFile> createInputFile(const AlgorithmType & type) {
            if (type == AlgorithmType::aes_encryption) {
                std::unique_ptr<EncryptableFile> file = std::make_unique<AESEncryptableFile>();
                file->open();
                return file;
            }
            else if (type == AlgorithmType::xor_encryption) {
                std::unique_ptr<EncryptableFile> file = std::make_unique<XOREncryptableFile>();
                file->open();
                return file;
            }
            else if (type == AlgorithmType::no_encryption) {
                std::unique_ptr<CloakableInputFile> file = std::make_unique<CloakableInputFile>();
                file->open();
                return file;
            }

            return nullptr;
        }

        static std::unique_ptr<CloakableOutputFile> createOutputFile(const std::string & filename, const AlgorithmType & type) {
            if (type == AlgorithmType::aes_encryption) {
                std::unique_ptr<DecryptableFile> file = std::make_unique<AESDecryptableFile>();
                file->open(filename);
                return file;
            }
            else if (type == AlgorithmType::xor_encryption) {
                std::unique_ptr<DecryptableFile> file = std::make_unique<XORDecryptableFile>();
                file->open(filename);
                return file;
            }
            else if (type == AlgorithmType::no_encryption) {
                std::unique_ptr<CloakableOutputFile> file = std::make_unique<CloakableOutputFile>();
                file->open(filename);
                return file;
            }

            return nullptr;
        }

        static std::unique_ptr<CloakableOutputFile> createOutputFile(const AlgorithmType & type) {
            if (type == AlgorithmType::aes_encryption) {
                std::unique_ptr<DecryptableFile> file = std::make_unique<AESDecryptableFile>();
                file->open();
                return file;
            }
            else if (type == AlgorithmType::xor_encryption) {
                std::unique_ptr<DecryptableFile> file = std::make_unique<XORDecryptableFile>();
                file->open();
                return file;
            }
            else if (type == AlgorithmType::no_encryption) {
                std::unique_ptr<CloakableOutputFile> file = std::make_unique<CloakableOutputFile>();
                file->open();
                return file;
            }

            return nullptr;
        }
};

class EncryptableFileFactory {
    public:
        static std::unique_ptr<EncryptableFile> create(const std::string & filename, const AlgorithmType & type) {
            if (type == AlgorithmType::aes_encryption) {
                std::unique_ptr<EncryptableFile> file = std::make_unique<AESEncryptableFile>();
                file->open(filename);
                return file;
            }
            else if (type == AlgorithmType::xor_encryption) {
                std::unique_ptr<EncryptableFile> file = std::make_unique<XOREncryptableFile>();
                file->open(filename);
                return file;
            }

            return nullptr;
        }

        static std::unique_ptr<EncryptableFile> create(const AlgorithmType & type) {
            if (type == AlgorithmType::aes_encryption) {
                std::unique_ptr<EncryptableFile> file = std::make_unique<AESEncryptableFile>();
                file->open();
                return file;
            }
            else if (type == AlgorithmType::xor_encryption) {
                std::unique_ptr<EncryptableFile> file = std::make_unique<XOREncryptableFile>();
                file->open();
                return file;
            }

            return nullptr;
        }
};

class DecryptableFileFactory {
    public:
        static std::unique_ptr<DecryptableFile> create(const std::string & filename, const AlgorithmType & type) {
            if (type == AlgorithmType::aes_encryption) {
                std::unique_ptr<DecryptableFile> file = std::make_unique<AESDecryptableFile>();
                file->open(filename);
                return file;
            }
            else if (type == AlgorithmType::xor_encryption) {
                std::unique_ptr<DecryptableFile> file = std::make_unique<XORDecryptableFile>();
                file->open(filename);
                return file;
            }

            return nullptr;
        }

        static std::unique_ptr<DecryptableFile> create(const AlgorithmType & type) {
            if (type == AlgorithmType::aes_encryption) {
                std::unique_ptr<DecryptableFile> file = std::make_unique<AESDecryptableFile>();
                file->open();
                return file;
            }
            else if (type == AlgorithmType::xor_encryption) {
                std::unique_ptr<DecryptableFile> file = std::make_unique<XORDecryptableFile>();
                file->open();
                return file;
            }

            return nullptr;
        }
};

#endif
