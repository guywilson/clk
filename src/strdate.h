#include <iostream>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <cstdint>

#include "logger.h"

#ifndef __INCL_STRDATE
#define __INCL_STRDATE

#define TIME_STAMP_BUFFER_LEN               64
#define DATE_STAMP_BUFFER_LEN               32
#define DATE_STRING_LENGTH                  10

struct TimeComponents {
    std::string year;
    std::string month;
    std::string day;

    std::string hour;
    std::string minute;
    std::string second;

    std::string microsecond;
};

class StrDate {
    public:
        static std::string getTimestamp();
        static std::string getTimestamp(bool includeus);
        static std::string getTimestampToMicrosecond();
};

#endif
