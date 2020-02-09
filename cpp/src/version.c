#include "version.h"

#define __BDATE__      "2020-02-09 12:00:19"
#define __BVERSION__   "1.3.005"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}
