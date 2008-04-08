@echo off
rem VirtuaWin (virtuawin.sourceforge.net)
rem build - VirtuaWin build script for Microsoft cmd.
rem
rem Copyright (c) 2006-2008 VirtuaWin (VirtuaWin@home.se)
rem
rem See the file VirtuaWin.c for copying and conditions.
rem
set TARGET=
set DTARGET=
set LOGFILE=
set LOGFILEA=
set OPTIONS=
set MAKEFILE=
:build_option
if "%1." == "."    goto build_cont
if "%1" == "-C"    set  TARGET=clean
if "%1" == "-d"    set  DTARGET=VirtuaWinD.exe
if "%1" == "-h"    goto build_help
if "%1" == "-l"    goto build_logf
if "%1" == "-la"   goto build_logfa
if "%1" == "-m"    goto build_mkfl
if "%1" == "-S"    set  TARGET=spotless
if "%1" == "-u"    set  OPTIONS=%OPTIONS% vwUNICODE=1
if "%1" == "-vd"   set  OPTIONS=%OPTIONS% vwVERBOSED=1
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

:build_unicode

:build_cont

if "%TARGET%." == "." set TARGET=%DTARGET%

set MAKE=nmake
if "%MAKEFILE%." == "."       set MAKEFILE=win32v6.mak
if "%MAKEFILE%" == "Makefile" set MAKE=make


if "%LOGFILE%." == "." goto build_applog

echo %MAKE% -f %MAKEFILE% %OPTIONS% %TARGET% > %LOGFILE% 2>&1
%MAKE% -f %MAKEFILE% %OPTIONS% %TARGET% > %LOGFILE% 2>&1

goto build_exit

:build_applog

if "%LOGFILEA%." == "." goto build_nolog

echo %MAKE% -f %MAKEFILE% %OPTIONS% %TARGET% >> %LOGFILEA% 2>&1
%MAKE% -f %MAKEFILE% %OPTIONS% %TARGET% >> %LOGFILEA% 2>&1

goto build_exit

:build_nolog

echo %MAKE% -f %MAKEFILE% %OPTIONS% %TARGET%
%MAKE% -f %MAKEFILE% %OPTIONS% %TARGET%

goto build_exit

:build_help

echo Usage: build [options]
echo .
echo Where options can be:-
echo     -C   : Build clean.
echo     -d   : For debug build (output is VirtuaWinD.exe).
echo     -h   : For this help page.
echo     -l {logfile}
echo          : Set the compile log file.
echo     -la {logfile}
echo          : Append the compile log to the given file.
echo     -m {makefile}
echo            Sets the makefile to use where {makefile} can be:-
echo              Makefile     Build using Cygwin, MinGW or Linux GNU GCC
echo              win32v6.mak  Build using MS VC version 6 onwards
echo     -S   : Build clean spotless.
echo     -u   : Build with UNICODE support.
echo     -vd  : Build with debug verbosity logging (large output).
echo .
echo If you change the build options used do a clean build (build -C) first.

:build_exit
