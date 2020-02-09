#include "version.h"

#define __BDATE__      "2020-02-09 14:37:36"
#define __BVERSION__   "1.3.006"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
