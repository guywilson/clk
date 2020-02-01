#include "version.h"

#define __BDATE__      "2020-02-01 20:57:45"
#define __BVERSION__   "1.3.003"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
