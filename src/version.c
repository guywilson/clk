#include "version.h"

#define __BDATE__      "2021-05-17 20:23:58"
#define __BVERSION__   "2.1.002"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
