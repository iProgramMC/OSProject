@rem 
@rem Main build script
@rem

@rem I'm not sure we even need this?

@echo off
set backupPath=%path%
set path=%cd%
cd /d %path%
set NSPath=%path%
set path=%backupPath%;%NSPath%\tools\i686-gcc\bin;%NSPath%\tools\nasm

make

:pause
:call run.bat