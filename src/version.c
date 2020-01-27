#include "version.h"

#define __BDATE__      "2020-01-27 17:20:10"
#define __BVERSION__   "1.0.004"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
