@rem 
@rem Main build script
@rem

@rem I'm not sure we even need this?

@echo off

echo NanoShell Application Build

set backupPath=%path%
set path=%cd%
cd /d %path%
set NSPath=%path%
set path=%backupPath%;%NSPath%\..\..\tools\i686-gcc\bin;%NSPath%\..\..\tools\nasm

make

: to be able to detect a kernel rebuild
touch ../../src/asm/builtin.asm

set path=%backupPath%

:pause
:call run.bat