#include <string>
#include <stdio.h>

#include "image.h"

using namespace std;

#ifndef _INCL_IMAGESTREAM

class ImageStream
{
    private:
        FILE *          fp;
        string          filename;

    protected:
        string &        getFilename() {
            return this->filename;
        }

        FILE *          getFilePtr() {
            return fp;
        }
        void            setFilePtr(FILE * fptr) {
            this->fp = fptr;
        }

        bool            isPNG(uint8_t * header, int size);
        bool            isBMP(uint8_t * header, int size);

    public:
        ImageStream() {}
        ImageStream(string & filename) {
            this->filename = filename;
        }

        virtual void    open() = 0;
        virtual void    open(string & filename) = 0;

        void            close() {
            fclose(fp);
        }
};

class ImageInputStream : public ImageStream
{
    private:
        PNG *       _readPNG();
        Bitmap *    _readBMP();

    public:
        ImageInputStream();
        ImageInputStream(string & filename);

        virtual void    open();
        virtual void    open(string & filename);

        RGB24BitImage * read();
};

class ImageOutputStream : public ImageStream
{
    public:
        ImageOutputStream();
        ImageOutputStream(string & filename);

        virtual void    open();
        virtual void    open(string & filename);

        void            write(uint8_t * buffer, uint32_t bufferLength);
};

#endif
