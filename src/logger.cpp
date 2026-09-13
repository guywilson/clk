#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>

#include "strdate.h"
#include "logger.h"

char * Logger::trim(const char * src) {
    if (src == NULL) {
        return NULL;
    }

    int originalLength = strlen(src);
    
    int i = 0;

    while (isspace(src[i++]));
    int startPos = i - 1;

    i = originalLength - 1;

    while (isspace(src[i--]));
    int endPos = i + 2;

    int newLength = endPos - startPos;

    char * trimmedString = (char *)malloc(newLength + 1);

    int j = 0;
    
    for (i = startPos;i < endPos;i++) {
        trimmedString[j++] = src[i];
    }

    trimmedString[j] = 0;

    return trimmedString;
}

void Logger::init(const std::string & filename, int logLevel) {
    this->loggingLevel = logLevel;

    if (filename.length() > 0) {
        this->lfp = fopen(filename.c_str(), "wt");

        if (this->lfp == NULL) {
	        syslog(LOG_INFO, "Failed to open log file %s", filename.c_str());
            this->lfp = stdout;
        }
    }
    else {
        this->lfp = stdout;
    }
}

void Logger::init(int logLevel) {
    this->loggingLevel = logLevel;
    this->lfp = stdout;
}

void Logger::close() {
    if (lfp != stdout) {
        fclose(lfp);
    }
}

void Logger::setLogLevel(int logLevel) {
    this->loggingLevel = logLevel;
}

void Logger::addLogLevel(int logLevel) {
    this->loggingLevel |= logLevel;
}

void Logger::clearLogLevel(int logLevel) {
    this->loggingLevel &= ~logLevel;
}

bool Logger::isLogLevel(int logLevel) {
    return ((this->loggingLevel & logLevel) == logLevel ? true : false);
}

int Logger::getLogLevelFromString(const std::string & logLevel) {
    int             level = 0;

    char * pszLogLevel = strdup(logLevel.c_str());
    char * reference = pszLogLevel;

    char * pszToken = trim(strtok_r(pszLogLevel, "|", &reference));

    while (pszToken != NULL) {
        if (strncmp(pszToken, "LOG_LEVEL_INFO", 14) == 0) {
            level |= LOG_LEVEL_INFO;
        }
        else if (strncmp(pszToken, "LOG_LEVEL_STATUS", 16) == 0) {
            level |= LOG_LEVEL_STATUS;
        }
        else if (strncmp(pszToken, "LOG_LEVEL_DEBUG", 15) == 0) {
            level |= LOG_LEVEL_DEBUG;
        }
        else if (strncmp(pszToken, "LOG_LEVEL_SQL", 13) == 0) {
            level |= LOG_LEVEL_SQL;
        }
        else if (strncmp(pszToken, "LOG_LEVEL_ERROR", 15) == 0) {
            level |= LOG_LEVEL_ERROR;
        }
        else if (strncmp(pszToken, "LOG_LEVEL_FATAL", 15) == 0) {
            level |= LOG_LEVEL_FATAL;
        }

        free(pszToken);

        pszToken = trim(strtok_r(NULL, "|", &reference));
    }

    free(pszLogLevel);

    return level;
}

int Logger::logMessage(int logLevel, const char * fmt, va_list args) {
    int         bytesWritten = 0;

    if (this->loggingLevel & logLevel) {
        std::string buffer = "[" + StrDate::getTimestampToMicrosecond() + "] ";

        switch (logLevel) {
            case LOG_LEVEL_DEBUG:
                buffer += "[DEBUG]";
                break;

            case LOG_LEVEL_SQL:
                buffer += "[SQL]";
                break;

            case LOG_LEVEL_STATUS:
                buffer += "[STATUS]";
                break;

            case LOG_LEVEL_INFO:
                buffer += "[INFO]";
                break;

            case LOG_LEVEL_ENTRY:
                buffer += "[ENTRY]";
                break;

            case LOG_LEVEL_EXIT:
                buffer += "[EXIT]";
                break;

            case LOG_LEVEL_ERROR:
                buffer += "[ERROR]";
                break;

            case LOG_LEVEL_FATAL:
                buffer += "[FATAL]";
                break;
        }

        buffer += fmt;
        buffer += "\n";

	    pthread_mutex_lock(&mutex);

        bytesWritten = vfprintf(this->lfp, buffer.c_str(), args);
        fflush(this->lfp);

	    pthread_mutex_unlock(&mutex);
    }

    return bytesWritten;
}

void Logger::newline() {
    fprintf(this->lfp, "\n");
}

int Logger::info(const char * fmt, ...) {
    va_list     args;
    int         bytesWritten;

    va_start (args, fmt);
    
    bytesWritten = logMessage(LOG_LEVEL_INFO, fmt, args);
    
    va_end(args);
    
    return bytesWritten;
}

int Logger::status(const char * fmt, ...) {
    va_list     args;
    int         bytesWritten;

    va_start (args, fmt);
    
    bytesWritten = logMessage(LOG_LEVEL_STATUS, fmt, args);
    
    va_end(args);
    
    return bytesWritten;
}

int Logger::debug(const char * fmt, ...) {
    va_list     args;
    int         bytesWritten;

    va_start (args, fmt);
    
    bytesWritten = logMessage(LOG_LEVEL_DEBUG, fmt, args);
    
    va_end(args);
    
    return bytesWritten;
}

int Logger::sql(const char * fmt, ...) {
    va_list     args;
    int         bytesWritten;

    va_start (args, fmt);
    
    bytesWritten = logMessage(LOG_LEVEL_SQL, fmt, args);
    
    va_end(args);
    
    return bytesWritten;
}

int Logger::entry(const char * fmt, ...) {
    va_list     args;
    int         bytesWritten;

    va_start (args, fmt);
    
    bytesWritten = logMessage(LOG_LEVEL_ENTRY, fmt, args);
    
    va_end(args);
    
    return bytesWritten;
}

int Logger::exit(const char * fmt, ...) {
    va_list     args;
    int         bytesWritten;

    va_start (args, fmt);
    
    bytesWritten = logMessage(LOG_LEVEL_EXIT, fmt, args);
    
    va_end(args);
    
    return bytesWritten;
}

int Logger::error(const char * fmt, ...) {
    va_list     args;
    int         bytesWritten;

    va_start (args, fmt);
    
    bytesWritten = logMessage(LOG_LEVEL_ERROR, fmt, args);
    
    va_end(args);
    
    return bytesWritten;
}

int Logger::fatal(const char * fmt, ...) {
    va_list     args;
    int         bytesWritten;

    va_start (args, fmt);
    
    bytesWritten = logMessage(LOG_LEVEL_FATAL, fmt, args);
    
    va_end(args);
    
    return bytesWritten;
}
