#include <iostream>
#include <string>

#include "version.h"

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

int main(int argc, char **argv)
{
    int         i;

    for (i = 1;i < argc;i++) {
        if (argv[i][0] == '-' || argv[i][0] == '/') {
            switch (argv[i][1]) {
                case '?':
                case 'h':
                    printUsage();
                    return 0;

				default:
					cout << "Invalid option (" << argv[i][1] << ") - cloak -? for help" << endl << endl;
					return -1;
            }
        }
    }

    return 0;
}
