#include "version.h"

#define __BDATE__      "2020-01-30 07:47:12"
#define __BVERSION__   "1.0.006"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
