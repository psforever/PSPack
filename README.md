# PSPack [![Build Status](https://travis-ci.org/psforever/PSPack.svg?branch=master)](https://travis-ci.org/psforever/PSPack)
A tool (originally developed by @pschord from @psforever) designed to work with PlanetSide 1 Pack files (*.PAK).
Enables support for extracting packs and later repacking them. Currently supports MinGW32 and GCC based C compilers on Windows and Linux. 

## Running PSPack

Running PSPack is relatively simple in Windows. Simply navigate to your PSPack directory and run the application by double clicking `pspack.exe` or from `cmd`:

      .\pspack.exe

On Linux, just run it like any other command line tool:

      ./pspack

Running the command without arguments will enter into interactive mode.
You can also pass arguments in directly to perform a specific action: 

	pspack.exe -x PathToFile.pak
	pspack.exe -c FolderName/


## Building From Source

1. Navigate to the PSPack directory.
2. Use your favorite build tool, IDE, or run the `make` command manually.

To cross-compile using Make, define PREFIX with the compiler tuple you need:

      $ PREFIX=x86_64-w64-mingw32- make

### Cleaning After Building

To clean your build after compilation, run:

    make clean

Or remove all temporary `*.o` (objects) and `*.exe` files.

