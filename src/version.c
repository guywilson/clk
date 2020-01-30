#include "version.h"

#define __BDATE__      "2020-01-30 22:34:58"
#define __BVERSION__   "1.2.002"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
