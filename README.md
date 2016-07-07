# PSPack [![Build Status](https://travis-ci.org/psforever/PSPack.svg?branch=master)](https://travis-ci.org/psforever/PSPack)
A tool developed to work with PlanetSide 1 Pack files (.PAK). Currently only supports pack extraction.

## Building
Make and GCC (either Mingw or regular GCC). MSVC is not supported at the moment due to some header and function incompatibilities. 

Just type

      $ make

To cross-compile, define PREFIX with the compiler tuple you need

      $ PREFIX=x86_64-w64-mingw32- make
