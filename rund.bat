@rem Run script 

@echo off

set NSPath=Z:\NanoshellV3
cd /d D:\Program Files\qemu
set path=%path%;%NSPath%

qemu-system-i386.exe -d cpu_reset -m 16M -serial stdio -kernel %nspath%/kernel.bin -s -S
rem -s -S 

rem go back
cd /d %NSPath%
