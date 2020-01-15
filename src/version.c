#include "version.h"

#define __BDATE__      "2020-01-15 07:53:54"
#define __BVERSION__   "0.1.002"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
