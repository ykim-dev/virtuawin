@echo off
rem VirtuaWin - virtuawin.sourceforge.net
rem build - VirtuaWin build script for MS windows
rem Copyright (C) 2006 Johan Piculell (virtuawin.sourceforge.net)
rem See the file VirtuaWin.c for copying and conditions.
set OPTIONS=
set LOGFILE=
set LOGFILEA=
set VWDEBUG=
set MAKEFILE=
:build_option
if "%1." == "."    goto build_cont
if "%1" == "-C"    set  OPTIONS=clean
if "%1" == "-d"    set  VWDEBUG=VirtuaWinD.exe
if "%1" == "-h"    goto build_help
if "%1" == "-l"    goto build_logf
if "%1" == "-la"   goto build_logfa
if "%1" == "-m"    goto build_mkfl
if "%1" == "-S"    set  OPTIONS=spotless
shift
goto build_option

:build_logf
shift
set LOGFILE=%1
shift
goto build_option

:build_logfa
shift
set LOGFILEA=%1
shift
goto build_option

:build_mkfl
shift
set MAKEFILE=%1
shift
goto build_option

:build_cont

if "%OPTIONS%." == "." set OPTIONS=%VWDEBUG%

set MAKE=nmake
if "%MAKEFILE%." == "."       set MAKEFILE=win32v6.mak
if "%MAKEFILE%" == "Makefile" set MAKE=make


if "%LOGFILE%." == "." goto build_applog

echo %MAKE% -f %MAKEFILE% %OPTIONS% > %LOGFILE% 2>&1
%MAKE% -f %MAKEFILE% %OPTIONS% > %LOGFILE% 2>&1

goto build_exit

:build_applog

if "%LOGFILEA%." == "." goto build_nolog

echo %MAKE% -f %MAKEFILE% %OPTIONS% >> %LOGFILEA% 2>&1
%MAKE% -f %MAKEFILE% %OPTIONS% >> %LOGFILEA% 2>&1

goto build_exit

:build_nolog

echo %MAKE% -f %MAKEFILE% %OPTIONS%
%MAKE% -f %MAKEFILE% %OPTIONS%

goto build_exit

:build_help

echo Usage: build [options]
echo .
echo Where options can be:-
echo     -C   : Build clean.
echo     -d   : For debug build (output is VirtuaWinD.exe)
echo     -h   : For this help page
echo     -l {logfile}
echo          : Set the compile log file
echo     -la {logfile}
echo          : Append the compile log to the given file
echo     -m {makefile}
echo            Sets the makefile to use where {makefile} can be:-
echo              Makefile     Build using Cygwin, MinGW or Linux GNU GCC
echo              win32v6.mak  Build using MS VC version 6 onwards
echo     -S   : Build clean spotless.

:build_exit

