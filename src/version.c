#include "version.h"

#define __BDATE__      "2021-05-15 22:44:39"
#define __BVERSION__   "2.0.008"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
