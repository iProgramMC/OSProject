@rem Run script 

@echo off

set backupPath=%path%
set NSPath=%CD%
cd /d c:\Program Files\qemu
set path=%path%;%NSPath%

qemu-system-i386.exe -d cpu_reset -m 16M -kernel %nspath%/kernel.bin
: -debugcon stdio
: -monitor telnet:127.0.0.1:55555,server,nowait
: -debugcon stdio
:qemu-system-i386 -m 16M -drive file=\\.\PHYSICALDRIVE1,format=raw
rem -s -S 

rem go back
cd /d %NSPath%

set path=%backupPath%
