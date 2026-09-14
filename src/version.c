#include "version.h"

#define __BDATE__      "2026-09-14 09:46:53"
#define __BVERSION__   "1.0.0"

const char * getVersion(void) {
    return __BVERSION__;
}

const char * getBuildDate(void) {
    return __BDATE__;
}
