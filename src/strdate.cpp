#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <locale>
#include <ctype.h>
#include <cstdint>

#include "strdate.h"

static void fillTimeStruct(TimeComponents * time) {
    auto twoDigits = [](int v) {
        std::string s = std::to_string(v);

        if (s.size() < 2) {
            s.insert(s.begin(), '0');
        }

        return s;
    };

    auto fourDigits = [](int v) {
        std::string s = std::to_string(v);

        while (s.size() < 4) {
            s.insert(s.begin(), '0');
        }

        return s;
    };

    struct timeval tv;
    gettimeofday(&tv, nullptr);

    struct tm ltime;
    time_t t = tv.tv_sec;

    localtime_r(&t, &ltime);

    time->year = fourDigits((int)(ltime.tm_year + 1900));
    time->month = twoDigits((int)(ltime.tm_mon + 1));
    time->day = twoDigits(ltime.tm_mday);

    time->hour = twoDigits((int)ltime.tm_hour);
    time->minute = twoDigits((int)ltime.tm_min);
    time->second = twoDigits((int)ltime.tm_sec);

    time->microsecond = std::to_string(static_cast<int>(tv.tv_usec));
}

std::string StrDate::getTimestamp() {
    return getTimestamp(false);
}

std::string StrDate::getTimestampToMicrosecond() {
    return getTimestamp(true);
}

std::string StrDate::getTimestamp(bool includeus) {
    TimeComponents tc;
    fillTimeStruct(&tc);

    std::string ts =
        tc.year + "-" +
        tc.month + "-" +
        tc.day + " " +
        tc.hour + ":" +
        tc.minute + ":" +
        tc.second;

    if (includeus) {
        ts += ".";
        ts += tc.microsecond;
    }

    return ts;
}
