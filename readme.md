# NanoShell
NanoShell 3rd Edition is a preemptively multi-tasked 32-bit operating system with a windowed GUI.

Be warned that building this is not for the average Linux user nor is it for the faint of heart.  Soon I'll work on a way to compile this in an easier way.

## Goals

- [x] Hello World
#### Primordial tasks
- [x] Paging and dynamic memory allocation
- [x] Loading an ELF at some virtual address with its own heap and executing it
- [x] Task Switching (concurrent threads)
#### User
- [x] A basic shell, can execute applications
- [x] Get a GUI running (Also in a task)
- [x] Applications can use the GUI API
- [ ] Port DOOM
- [ ] (ultimate boss) Port Mesa and Super Mario 64 (based on the DOS port, please)
#### Drivers
- [x] PS/2 Mouse and Keyboard
- [x] EGA Text mode
- [x] VBE Graphics mode (fast)
- [x] Bochs/QEMU debugcon (E9 hack)
- [ ] Serial port
- [ ] PCI
- [ ] Sound devices (e.g. SoundBlaster 16)
- [ ] USB
#### Permanent memory
- [x] Block storage abstraction base
- [x] Optional RAM disk images can be loaded and used as block devices
- [ ] ATA device driver
- [ ] USB stick device driver
#### File system
- [x] Root contains several files which are packed neatly into the executable
- [ ] Mounting other file systems to the main one
- [ ] Cleaning up after a while of not having used a certain directory page (can't apply to root or ram disks, you know why)

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
