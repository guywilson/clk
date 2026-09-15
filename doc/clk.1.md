% CLK(1) CLK User Manual
% Guy Wilson
% January 23, 2026

# NAME

clk - cloak

# SYNOPSIS

clk [merge|extract] [*options*] file

# DESCRIPTION

Hide and extract an encrypted file within an RGB (24-bit) PNG image.

The idea is simple, a 24-bit colour PNG image uses 3 bytes for each pixel in the image, one each for the Red, Green and Blue channels, so each colour channel is represented by a value between 0 - 255. If we encode a file in the least significant bits (LSBs) of the image data, there will be no visible difference in the image when displayed. At an encoding depth of 1-bit per byte, we need 8 bytes of image data to encode 1 byte of our file.

# OPTIONS

**-h, -host [host file]**

Specifies the filename of the host. Currently, clk only supports 24-bit PNG images. That is PNG images that store their image data with 8-bits per colour channel (RGB).

**`--aes256` or `--aes`**

Encrypt the file prior to merging, and decrypt after extracting using AES-256, this is the default. The user will be prompted to enter a password (make it a good one).

**`--xor` or `--otp`**

Encrypt the file prior to merging, and decrypt after extracting using XOR encryption. If this option is specified, the user must also specify a key file using the -k or -g option.

**`--no-encryption`**

Do not encrypt the file prior to merging, or decrypt after extracting. Just hide the file in the image.

**`--high` or `--hi`**

The most secure, uses 8 bytes of image data for each byte of your file. There will be no visible difference before and after merging a file into the image.

**`--medium` or `--med`**

Uses 4 bytes of image data for each byte of your file. There will be little visible difference before and after merging a file into the image.

**`--low` or `--lo`**

Not recommended. Uses 2 bytes of image data for each byte of your file. There will be a very visible difference before and after merging a file into the image, it will appear as “grain”.

**`--no-security`**

DO NOT USE, FOR TESTING ONLY! Uses 1 bytes of image data for each byte of your file., i.e. will simply replace the image with your data.

**-k, -key [key file]**

Supply a key file for XOR encryption, clk will abort with an error if key file is smaller than your file you want to hide/extract. If your intention is to use the key file as a One Time Pad (OTP), in order to be secure, you must follow these 3 rules:

1. The key must be truly random
2. The key must be used **only** once
3. The key must be at least as long as the file to encrypt (clk checks this)

**-g, -generate [key file]**

This will generate random data the same length as your file to encrypt with XOR encryption and save it to your key file, clk will use the data to encrypt your file before exiting, so can be used in the same way as the -k option. See the -k option for the rules around OTP encryption.

**-c, `--capacity`**

Report the capacity of the host file specified with -host at the specified security level and exit.

**`--version`, -v**

Prints the version information of the clk program and exits.

**`--help`, -?**

Prints the supported command line options and exits.

# EXAMPLES

To 'cloak' file clk.dat using AES-256 encryption within test.png using the highest security level:

```
	clk merge --high --aes256 -host test.png clk.dat
```

To 'uncloak' file out.dat from test.png using AES-256 encryption:

```
	clk extract --high --aes256 -host test.png out.dat
```

To report the capacity of test.png using the highest security level:

```
	clk merge --capacity --high -host test.png
```

# AUTHOR

Written by Guy Wilson.

# COPYRIGHT

Copyright (c) 2026 Guy Wilson.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

You can contact the author at wilson.guy@gmail.com