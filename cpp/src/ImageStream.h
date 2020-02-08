#include <string>
#include <stdio.h>

#include "image.h"

using namespace std;

#ifndef _INCL_IMAGESTREAM
#define _INCL_IMAGESTREAM

#define NUM_CHANNELS		 3

/*
** Header sizes
*/
#define BMP_HEADER_SIZE		14
#define WINV3_HEADER_SIZE   40

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
        ImageStream(string & filename) {
            this->filename = filename;
        }

        virtual void    open() = 0;

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
        ImageInputStream(string & filename);

        virtual void    open();

        RGB24BitImage * read();
};

class ImageOutputStream : public ImageStream
{
    private:
        void        _writePNG(PNG * png);
        void        _writeBMP(Bitmap * bmp);
        
    public:
        ImageOutputStream(string & filename);

        virtual void    open();

        void            write(RGB24BitImage * image);
};

#endif
