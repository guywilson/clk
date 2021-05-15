#include <string>
#include <stdio.h>

#include "datafile.h"

using namespace std;

#ifndef _INCL_FILESTREAM
#define _INCL_FILESTREAM

class FileStream
{
    private:
        FILE *          fp;
        string          filename;

    protected:
        FILE *          getFilePtr() {
            return this->fp;
        }
        void            setFilePtr(FILE * ptr) {
            this->fp = ptr;
        }
        string &        getFileName() {
            return this->filename;
        }

    public:
        FileStream(string & filename) {
            this->filename = filename;
        }
        FileStream(const char * szFileName) {
            this->filename = szFileName;
        }

        virtual void    open() = 0;

        void            close() {
            fclose(fp);
        }
};

class FileInputStream : public FileStream
{
    public:
        FileInputStream(string & filename) : FileStream(filename) {}
        FileInputStream(const char * filename) : FileStream(filename) {}

        virtual void    open();

        uint32_t        getFilelength();

        DataFile *      read();
};

/*
** This class reads in data and compresses it into the DataFile returned
** from the read() method
*/
class FileZippedInputStream : public FileInputStream
{
    private:
        const uint32_t  blockSize = 8192;
        
    public:
        FileZippedInputStream(string & filename) : FileInputStream(filename) {}
        FileZippedInputStream(const char * filename) : FileInputStream(filename) {}

        virtual void    open();

        DataFile *      read();
};

class FileOutputStream : public FileStream
{
    public:
        FileOutputStream(string & filename) : FileStream(filename) {}
        FileOutputStream(const char * filename) : FileStream(filename) {}

        virtual void    open();

        void            write(DataFile * df);
};

/*
** This class writes out uncompressed data, i.e. the DataFile parameter
** to the write() method must be compressed data from a FileZippedInputStream
*/
class FileZippedOutputStream : public FileOutputStream
{
    private:
        const uint32_t  blockSize = 8192;
        
    public:
        FileZippedOutputStream(string & filename) : FileOutputStream(filename) {}
        FileZippedOutputStream(const char * filename) : FileOutputStream(filename) {}

        virtual void    open();

        void            write(DataFile * df);
};

#endif
