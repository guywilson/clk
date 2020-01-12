#include "image.h"

#ifndef _INCL_IMAGEFACTORY

class ImageFactory
{
    public:
        static RGB24BitImage create(FILE * fp);
};

#endif
