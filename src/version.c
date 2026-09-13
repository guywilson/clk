#include "version.h"

#define __BDATE__      "2026-09-12 20:28:25"
#define __BVERSION__   "1.0.016"

const char * getVersion(void) {
    return __BVERSION__;
}

const char * getBuildDate(void) {
    return __BDATE__;
}
