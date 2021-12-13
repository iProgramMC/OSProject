@rem Run script 

@echo off

set NSPath=Z:\NanoshellV3
set path=%path%;%NSPath%;%NSPath%\tools\i686-gcc\bin

i686-elf-gdb kernel.bin
