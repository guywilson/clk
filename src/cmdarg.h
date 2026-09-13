#include <iostream>
#include <string>
#include <vector>

#include <stdbool.h>

#include "clk_error.h"

#ifndef __INCL_CMDARG
#define __INCL_CMDARG

class CmdArg {
    private:
        int argPointer;
        std::vector<std::string> args;

    public:
        CmdArg(int argc, char ** argv) {
            for (int i = 1;i < argc;i++) {
                args.push_back(argv[i]);
            }

            argPointer = 0;
        }

        ~CmdArg() {
            args.clear();
        }

        inline int getNumArgs() {
            return args.size();
        }

        inline bool hasMoreArgs() {
            return (argPointer < getNumArgs());
        }

        inline bool isLastArg() {
            return (argPointer >= (getNumArgs() - 1));
        }

        inline std::string nextArg() {
            if (argPointer >= getNumArgs()) {
                throw clk_error("Command index overrun");
            }

            return args[argPointer++];
        }

        inline std::string getArg(int i) {
            if (i >= getNumArgs()) {
                throw clk_error("Command index overrun");
            }

            return args[i];
        }

        inline void dumpArgs() {
            std::cout << "Num args captured: " << getNumArgs() << std::endl;
            for (int i = 0;i < getNumArgs();i++) {
                std::string arg = args[i];
                std::cout << "Arg " << i << " = '" << arg << "'" << std::endl;
            }
        }
};
 
#endif
