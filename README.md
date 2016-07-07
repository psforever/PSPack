# PSPack Windows MinGW32
## PSForever PlanetSide 1 Pack File Tool

A tool (originally developed by @pschord from @psforever) designed to work with PlanetSide 1 Pack files (*.PAK) with 
extraction support. Currently supports MinGW32 based C compilers on Windows. 

### Building From Source

1. Navigate to the PSPack directory.
2. Use your favorite build tool, IDE, or run the `make` command manually.

#### Cleaning Before Building

To clean your build after compilation, run:

    make clean

### After Build

Running PSPack is relatively simple in Windows. Simply navigate to your PSPack directory and run the application:

    pspack.exe

You can also pass arguments in directly: 

	pspack.exe -x PathToFile.pak
	pspack.exe -c FolderName/