# file-cloak

This node.js package is implemented by the code from my clk C++ repository and compiled using node-gyp.

Hide and extract an encrypted file within an RGB (24-bit) bitmap or PNG image.
------------------------------------------------------------------------------

The idea is simple, a 24-bit colour bitmap or PNG image uses 3 bytes for each pixel in the image, one each for Red, Green and Blue, so each colour channel is represented by a value between 0 - 255. If we encode a file in the least significant bits (LSBs) of the image data, there will be no visible difference in the image when displayed. At an encoding depth of 1-bit per byte, we need 8 bytes of image data to encode 1 byte of our file.

Clk encrypts your 'secret' data file using the AES-256 cipher in CBC mode prior to encoding it in your chosen image. You will be prompted to enter a password (max 256 chars), the SHA-256 hash of which is used as the key for the pass through AES. 

Clk can also encrypt using a supplied keystream file using simple XOR encryption, the advantage of this mechanism is you can employ a one-time-pad scheme, which providing you stick to the rules for a one-time-pad encryption scheme: 

1) The key is truly random 
2) The key is used once and only once 
3) The key is at least as long as the file being encrypted 

it is mathematically proven to be unbreakable. Of course, any encryption scheme is useless if some third party has got hold of your encryption key.

Some tips regarding password strength
-------------------------------------
A good password is one that cannot be broken using a dictionary attack, e.g. don't use a word from the dictionary or a derivation of. Use a made-up word or phrase with symbols and numbers, better still a random string of characters. In the context of this software, an important aspect is getting the password to your intended audience securely. It is also imperative that you do not re-use a key, it may be prudent to agree a unique and random set of keys with your audience in advance.

References:

https://en.wikipedia.org/wiki/Dictionary_attack

https://en.wikipedia.org/wiki/Password_strength

https://www.random.org/


APIs
-----------
file-cloak implements the following methods:

    getVersion() - returns the version string of the underlying clk library

    getFilelength(filename) - returns the file length of the given file name

    hide(
        inputImageName,         // The input 24-bit PNG or BMP image
        inputFileName,          // The input file to hide in the image
        outputImageName,        // The output image
        quality,                // The 'merge quality', 1, 2, or 4 bits-per-byte
        outputImageFormat,      // The output image format 'PNG' or 'BMP'
        encryptionAlgorithm,    // The encryption algorithm 'AES-256' or 'XOR'
        password)               // The password

    reveal(
        inputImageName,         // The input 24-bit PNG or BMP image
        outputFileName,         // The extracted file hidden in the image
        quality,                // The 'merge quality', 1, 2, or 4 bits-per-byte
        encryptionAlgorithm,    // The encryption algorithm 'AES-256' or 'XOR'
        ignoreCRC,              // Whether to ignore the CRC check or not true or false
        password)               // The password

Tips
----
* In the case of PNG files, you may notice that the output PNG image from Clk is a different size than the input image, they are identical in content, it is simply that PNG compression will likely result in different size files. This is the key difference between PNG images and 24-bit bitmap images, PNG is compressed whereas bitmap (bmp) is generally not. If you use a bitmap (bmp) file with Clk you will see that the input and output .bmp files will be exactly the same size.

Have fun!

