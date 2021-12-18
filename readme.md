# NanoShell V3
Here's my latest dive into OS development.

## Build

### Linux build:
Not tested.  You need `i686-elf-gcc` and `nasm`.  The `GNUmakefile` needs to be changed to use generic `nasm` instead of `./tools/nasm/nasm`.

### Windows build:
NOTE: Create a `build` directory before attempting to build the OS.

Run the provided windows `build.bat` with the required tools in `tools/i686-gcc` (the binaries for `i686-elf-gcc`), and `tools/nasm`

Download the `i686-elf` binutils from: https://github.com/lordmilko/i686-elf-tools/releases

