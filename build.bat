@rem 
@rem Main build script
@rem

@rem I'm not sure we even need this?

@echo off
set backupPath=%path%
set path=%cd%
set laptopshit=C:\Program Files (x86)\GnuWin32\bin
cd /d %path%
set NSPath=%path%
set path=%backupPath%;%NSPath%\tools\i686-gcc\bin;%NSPath%\tools\nasm;%NSPath%\tools;%laptopshit%

make

set path=%backupPath%

:pause
:call run.bat