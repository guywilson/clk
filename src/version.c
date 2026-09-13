#include "version.h"

#define __BDATE__      "2026-09-13 09:25:59"
#define __BVERSION__   "1.0.017"

const char * getVersion(void) {
    return __BVERSION__;
}

const char * getBuildDate(void) {
    return __BDATE__;
}
