#include <iostream>
#include <string>
#include <vector>

#include <stdbool.h>

#include "clk_error.h"

using namespace std;

#ifndef __INCL_CMDARG
#define __INCL_CMDARG

class CmdArg {
    private:
        int argPointer;
        vector<string> args;

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

        inline string nextArg() {
            if (argPointer >= getNumArgs()) {
                throw clk_error("Command index overrun");
            }

            return args[argPointer++];
        }

        inline string getArg(int i) {
            if (i >= getNumArgs()) {
                throw clk_error("Command index overrun");
            }

            return args[i];
        }

        inline void dumpArgs() {
            cout << "Num args captured: " << getNumArgs() << endl;
            for (int i = 0;i < getNumArgs();i++) {
                string arg = args[i];
                cout << "Arg " << i << " = '" << arg << "'" << endl;
            }
        }
};
 
#endif
