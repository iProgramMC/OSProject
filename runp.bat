@rem Run physical script 

@echo off

set backupPath=%path%
set NSPath=%CD%
copy kernel.bin D:\boot\kernel.bin
cd /d c:\Program Files\qemu
set path=%path%;%NSPath%

qemu-system-i386 -m 16M -drive file=\\.\PHYSICALDRIVE1,format=raw -debugcon stdio
rem -s -S 

rem go back
cd /d %NSPath%

set path=%backupPath%
