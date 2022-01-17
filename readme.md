# NanoShell
NanoShell 3rd Edition is a preemptively multi-tasked 32-bit operating system with a windowed GUI.

Be warned that building this is not for the average Linux user nor is it for the faint of heart.  Soon I'll work on a way to compile this in an easier way.

## Build instructions

### Linux build:
Not supported.  Could be supported soon, but currently not supported.

### Windows build:

#### Preparation

Create the `build` folder.  Inside it create the folders `asm`, `kapp`, `fs` and `icons`.

Create the `tools` directory.  Download [the i686-elf GCC+binutils](https://github.com/lordmilko/i686-elf-tools/releases/download/7.1.0/i686-elf-tools-windows.zip), and extract it into `tools/i686-gcc/`.
Also download NASM and place it inside `tools/nasm` so that it is reachable at `tools/nasm/nasm.exe`.

Make sure that `make` is easily accessible by opening a command prompt anywhere and typing `make`.

Compile `tools_src/fsmaker` and `tools_src/icontest`.

Place `fsmaker.exe` inside `tools/`.
Place `icontest.exe` inside `tools/icc/`. (create the directory, if necessary)

#### The Moment of Truth

Run `buildall.bat`.  It should start building.  Once it's done you should have a `kernel.bin` in the repo root.

## Installation

If you don't have grub2, install it.  It's relatively easy.

Once you have a grub installed on your favorite USB drive, create a `grub.cfg` inside the `grub` directory.
Add the following lines:
```
menuentry "NanoShell" {
	multiboot /boot/kernel.bin
	set gfxpayload=1024x768x32
	boot
}
```

Note that you can place your kernel image anywhere, but I prefer `/boot/kernel.bin`.  Place your kernel image so that grub can find it, and then restart.

And you're done! You should be in NanoShell now.  Type `w` to go to the GUI mode.
