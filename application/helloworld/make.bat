@echo off

set kpath=Z:\NanoshellV3
set path=%path%;%kpath%\tools\i686-gcc\bin;%kpath%\tools\nasm;

echo compiling
i686-elf-gcc -c main.c -o main.o -I %kpath%\tools\i686-gcc\include -ffreestanding -g -Wall -Wextra -fno-exceptions -std=c99

echo linking
i686-elf-gcc -T link.ld -o main.elf -ffreestanding -g -nostdlib main.o -lgcc

echo Done!