# CLK

A new version of cloak, re-designed and re-engineered from the bottom up.

Hide and extract an encrypted file within an RGB (24-bit) PNG image.

The idea is simple, a 24-bit colour PNG image uses 3 bytes for each pixel in the image, one each for the Red, Green and Blue channels, so each colour channel is represented by a value between 0 - 255. If we encode a file in the least significant bits (LSBs) of the image data, there will be no visible difference in the image when displayed. At an encoding depth of 1-bit per byte, we need 8 bytes of image data to encode 1 byte of our file.

Cloak can encrypt your 'secret' data file using either the AES-256 (Rijndael) cipher (in CBC mode) or XOR encryption prior to encoding it in your chosen image. With AES encryption, you will be prompted to enter a password, the SHA-512 hash of which is used as the key for the encryption with AES-256. With XOR encryption, you must either supply a keystream file using the -k option, or specify the file to create with the -g option. The OTP generate function uses the /dev/random device on *nix systems.

The advantage with XOR encryption is you can employ a one-time-pad scheme, which providing you stick to the rules for a one-time-pad encryption scheme, is mathematically proven to be unbreakable.

The rules are simple, but break one or more of them, it becomes relatively easy to crack:

1. The key is truly random
2. The key is used once and only once
3. The key is at least as long as the file being encrypted

Of course, any encryption scheme is useless if some third party has got hold of your encryption key.

## Some tips regarding password strength

A good password is one that cannot be broken using a dictionary attack, e.g. don't use a word from the dictionary or a derivation of. Use a made-up word or phrase with symbols and numbers, better still a random string of characters. In the context of this software, an important aspect is getting the password or keystream to your intended audience securely. It is also imperative that you do not re-use a key, it may be prudent to agree a unique and random set of keys with your audience in advance.

### References:

https://en.wikipedia.org/wiki/Dictionary_attack

https://en.wikipedia.org/wiki/Password_strength

https://www.random.org/

## Building clk

Cloak is written in C++ and I have provided a makefile for Unix/Linux using the g++ compiler (tested on Mac OS). Cloak depends on the 3rd party libraries libpng (http://libpng.org) and libgcrypt (https://www.gnupg.org/software/libgcrypt/index.html) (for the encryption and hashing algorithms, part of GPG).

Generate the makefile and build with a C++20 compiler, the libpng and
libgcrypt development headers/libraries, and Pandoc (for the manual page):

```sh
./configure
make
```

After editing `configure.ac`, regenerate `configure` with `autoconf`.
The original hand-written makefile is preserved as `makefile.original`;
edit `makefile.in` to change the generated makefile.

Release builds use `-O2`. For a debug build using `-g -O0`:

```sh
make clean
make DEBUG=1
```

Run `make clean` when switching build modes so all objects are rebuilt.
Both modes retain `-Wall -pedantic` and C++20 support. Configure accepts
`CC`, `CXX`, `CPPFLAGS`, `CFLAGS`, `CXXFLAGS`, and `LDFLAGS`; for example,
use `CPPFLAGS=-I/path/include LDFLAGS=-L/path/lib ./configure` for libraries
installed outside the compiler's default search paths.

The `all`, `install`, `version`, and `clean` targets remain available.
`./configure --prefix=/path` changes the default `/usr/local` installation
prefix, and `make install DESTDIR=/staging/path` supports staged installs.
`make version` requires the existing `vbuild` utility.

## Using clk

Type clk --help to get help on the command line parameters:

```
Usage: clk [merge|extract] [options] file
Hide or extract, an optionally encrypted file in/from the specifed bitmap based host file
options:
    -h | -host [host file] - currently supports 24-bit PNG images only
    -algo [encryption algorithm] (aes|xor|none)
    -sl | -security-level [level] (high|medium|low)
    -k | -key [keyfile] for XOR encryption use the keyfile as the key
    -g | -generate [keyfile] for XOR encryption, generate and use the keyfile as the key
    -c | --capacity report the capacity of the host file and exit
    -? | --help show this help and exit
    -v | --version print version information and exit
```

![car](car.png)

I have included a sample PNG file with this distribution - car.png which has a PDF document encoded within it, the password used to encrypt the file is 'password', however you should use a strong password in real-world applications, see the tips on password strength above.

For example, to 'cloak' a file within car.png I used the following command:

```clk merge -security-level high -algo aes -h car.png clk.pdf```

This tells clk to use merge the file 'clk.pdf' into the image 'car.png' using an encoding depth of 1-bit per byte.

To 'uncloak' the file from car.png, you can use the following command:

```clk extract -security-level high -algo aes -h car.png out.pdf```

This tells clk to use extract mode to extract the file 'out.pdf' from the input image 'car.png', again using 1-bit per byte.

Have fun!
