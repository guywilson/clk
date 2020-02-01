#include "version.h"

#define __BDATE__      "2020-02-01 15:16:08"
#define __BVERSION__   "1.3.002"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
