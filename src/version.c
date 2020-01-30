#include "version.h"

#define __BDATE__      "2020-01-30 11:15:56"
#define __BVERSION__   "1.2.001"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
