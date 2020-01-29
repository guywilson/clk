#include "version.h"

#define __BDATE__      "2020-01-29 07:38:07"
#define __BVERSION__   "1.0.005"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
