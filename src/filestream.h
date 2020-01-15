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

        virtual void    open() = 0;

        void            close() {
            fclose(fp);
        }
};

class FileInputStream : public FileStream
{
    public:
        FileInputStream(string & filename) : FileStream(filename) {}

        virtual void    open();

        DataFile *      read();
};

class FileOutputStream : public FileStream
{
    public:
        FileOutputStream(string & filename) : FileStream(filename) {}

        virtual void    open();

        void            write(DataFile * df);
};

#endif
