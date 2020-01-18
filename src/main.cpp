#include <iostream>
#include <string>

#include "version.h"
#include "../test/testsuite.h"

using namespace std;

void printUsage()
{
    cout << "Usage:" << endl;
    cout << "    cloak -a/e -v -i [infile] -o [outfile] -s [secretfile] -k (keystream file) -b[bits per byte] -l[compression level]" << endl << endl;
    cout << "    Where: -a = 'add' mode to add 'secretfile' to 'infile'" << endl;
    cout << "           -e = 'extract' mode to extract 'outfile' from 'infile'" << endl;
    cout << "           -v = 'verbose' mode, print bitmap header info" << endl;
    cout << "           -bn = Bits per byte, must be 1, 2 or 4" << endl;
    cout << "           -ln = Compression level, must be 0 to 9" << endl;
    cout << "           infile  = an input bitmap" << endl;
    cout << "           outfile = output bitmap (add mode) or extracted file" << endl;
    cout << "           secretfile = secret input file to add to 'infile'" << endl << endl;
	cout.flush();
}

void runTests()
{
    cout << "Running test suite..." << endl << endl;

    test_getKey();
    test_encrypt();
}

int main(int argc, char **argv)
{
    int         i;
    char *      arg;

    for (i = 1;i < argc;i++) {
        arg = argv[i];

        if (arg[0] == '-') {
            if (strncmp(arg, "--help", 6) == 0) {
                printUsage();
                return 0;
            }
            else if (strncmp(arg, "--test", 6) == 0) {
                runTests();
                return 0;
            }
            else {
                cout << "Invalid option (" << arg << ") - clk --help for help" << endl << endl;
                return -1;
            }
        }
    }

    return 0;
}