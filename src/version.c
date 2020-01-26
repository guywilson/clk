#include "version.h"

#define __BDATE__      "2020-01-26 21:03:42"
#define __BVERSION__   "1.0.001"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
