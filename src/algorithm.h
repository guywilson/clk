#include <vector>
#include <cstddef>

#include <gcrypt.h>

#ifndef __INCL_ALGORITHM
#define __INCL_ALGORITHM

#define DEFAULT_BLOCK_SIZE                  64U

enum class AlgorithmType {
    aes_encryption,
    xor_encryption,
    no_encryption
};

class EncryptionAlgorithm {
    public:
        virtual ~EncryptionAlgorithm() {

        }
        
        virtual void encryptBlock(uint8_t * buffer, size_t bufferLength, uint8_t * key, size_t keyLength) {
            return;
        }

        virtual void decryptBlock(uint8_t * buffer, size_t bufferLength, uint8_t * key, size_t keyLength) {
            return;
        }

        virtual size_t getBlockSize() {
            return (size_t)DEFAULT_BLOCK_SIZE;
        }

        virtual uint8_t * getIV() {
            return NULL;
        }

        virtual void setIV(uint8_t * buffer, size_t bufferLength) {
            return;
        }
};

class AESEncryptionAlgorithm : public EncryptionAlgorithm {
    private:
        gcry_cipher_hd_t handle;
        int blockNum;

        uint8_t * iv;

        bool isFirstBlock() {
            return blockNum == 0 ? true : false;
        }

        void open();
        void generateIV();
        void setIVFromBuffer(uint8_t * iv);
        void setKey(uint8_t * key, size_t keyLength);
        void encryptBlock(uint8_t * buffer);
        void decryptBlock(uint8_t * buffer);

    public:
        AESEncryptionAlgorithm();
        ~AESEncryptionAlgorithm();

        void encryptBlock(uint8_t * buffer, size_t blockSize, uint8_t * key, size_t keyLength) override;
        void decryptBlock(uint8_t * buffer, size_t blockSize, uint8_t * key, size_t keyLength) override;

        size_t getBlockSize() override;

        uint8_t * getIV() override;
        void setIV(uint8_t * buffer, size_t bufferLength) override;
};

class XOREncryptionAlgorithm : public EncryptionAlgorithm {
    public:
        void encryptBlock(uint8_t * buffer, size_t bufferLength, uint8_t * key, size_t keyLength) override;
        void decryptBlock(uint8_t * buffer, size_t bufferLength, uint8_t * key, size_t keyLength) override;
};

#endif
