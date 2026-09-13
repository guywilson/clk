#include <exception>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef __CLK_ERROR
#define __CLK_ERROR

#define MESSAGE_BUFFER_LEN                  4096

class clk_error : public std::exception
{
    private:
        std::string          message;

    public:
        const char * getTitle() {
            return "CLK error: ";
        }

        clk_error() {
            this->message.assign(getTitle());
        }

        clk_error(const char * msg) : clk_error() {
            this->message.append(msg);
        }

        clk_error(const char * msg, const char * file, int line) : clk_error() {
            char lineNumBuf[8];

            snprintf(lineNumBuf, 8, ":%d", line);

            this->message.append(msg);
            this->message.append(" at ");
            this->message.append(file);
            this->message.append(lineNumBuf);
        }

        virtual const char * what() const noexcept {
            return this->message.c_str();
        }

        static char * buildMsg(const char * fmt, ...) {
            va_list     args;
            char *      buffer;

            va_start(args, fmt);
            
            buffer = (char *)malloc(MESSAGE_BUFFER_LEN);
            
            vsnprintf(buffer, MESSAGE_BUFFER_LEN, fmt, args);
            
            va_end(args);

            return buffer;
        }
};

class clk_fatal : public clk_error {
    public:
        const char * getTitle() {
            return "Fatal error: ";
        }

        clk_fatal() : clk_error() {}
        clk_fatal(const char * msg) : clk_error(msg) {}
        clk_fatal(const char * msg, const char * file, int line) : clk_error(msg, file, line) {}
};

#endif
