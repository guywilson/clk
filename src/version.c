#include "version.h"

#define __BDATE__      "2020-02-02 12:37:40"
#define __BVERSION__   "1.3.004"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
