#include "version.h"

#define __BDATE__      "2026-09-13 09:40:19"
#define __BVERSION__   "1.0.018"

const char * getVersion(void) {
    return __BVERSION__;
}

const char * getBuildDate(void) {
    return __BDATE__;
}
