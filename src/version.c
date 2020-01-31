#include "version.h"

#define __BDATE__      "2020-01-31 21:29:43"
#define __BVERSION__   "1.3.001"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
