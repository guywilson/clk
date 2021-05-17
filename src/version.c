#include "version.h"

#define __BDATE__      "2021-05-17 20:34:21"
#define __BVERSION__   "2.1.003"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
