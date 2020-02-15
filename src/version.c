#include "version.h"

#define __BDATE__      "2020-02-09 15:52:06"
#define __BVERSION__   "1.3.007"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
