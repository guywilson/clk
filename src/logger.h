#include <string>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#ifndef _INCL_LOGGER
#define _INCL_LOGGER

/*
** Supported log levels...
*/
#define LOG_LEVEL_INFO          0x01
#define LOG_LEVEL_STATUS        0x02
#define LOG_LEVEL_DEBUG         0x04
#define LOG_LEVEL_SQL           0x08
#define LOG_LEVEL_ENTRY         0x10
#define LOG_LEVEL_EXIT          0x20
#define LOG_LEVEL_ENTRYEXIT     (LOG_LEVEL_ENTRY | LOG_LEVEL_EXIT)
#define LOG_LEVEL_ERROR         0x40
#define LOG_LEVEL_FATAL         0x80

#define LOG_LEVEL_ALL           (LOG_LEVEL_INFO | LOG_LEVEL_STATUS | LOG_LEVEL_DEBUG | LOG_LEVEL_SQL | LOG_LEVEL_ENTRYEXIT | LOG_LEVEL_ERROR | LOG_LEVEL_FATAL)

class Logger {
    public:
        static Logger & getInstance() {
            static Logger instance;
            return instance;
        }

    private:
        Logger() {}

        FILE * lfp;
        int loggingLevel;
        pthread_mutex_t mutex;

        static char * trim(const char * src);

        int logMessage(int logLevel, const char * fmt, va_list args);

    public:
        ~Logger() {}

        static int getLogLevelFromString(const std::string & logLevel);

        void init(const std::string & filename, int logLevel);
        void init(int logLevel);
        
        void close();

        void setLogLevel(int logLevel);
        void addLogLevel(int logLevel);
        void clearLogLevel(int logLevel);
        bool isLogLevel(int logLevel);

        void newline();
        int info(const char * fmt, ...);
        int status(const char * fmt, ...);
        int debug(const char * fmt, ...);
        int sql(const char * fmt, ...);
        int entry(const char * fmt, ...);
        int exit(const char * fmt, ...);
        int error(const char * fmt, ...);
        int fatal(const char * fmt, ...);
};

#endif