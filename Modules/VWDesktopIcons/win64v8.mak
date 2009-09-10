# VirtuaWin (virtuawin.sourceforge.net)
# win32v6.mak - WinList make file for Microsoft MSVC v6.0
#
# Copyright (c) 2006-2009 VirtuaWin (VirtuaWin@home.se)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 675 Mass Ave, Cambridge, MA 02139, USA.
#
##############################################################################

CC      = cl
RC      = rc
LD	= link
MT      = mt
CFLAGS	= -nologo -Gy -W3 -Wp64 -GS- -Ox -MD -D_DLL -DNDEBUG  -DWIN32 -DPC -DWINNT -DVC_EXTRALEAN -DWIN32_LEAN_AND_MEAN -DNOSERVICE -DNOMCX -DNOIME -DNOSOUND -DNOCOMM -DNOKANJI -DNORPC -DNOPROXYSTUB -DNOIMAGE -DNOTAPE -DNOMINMAX -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1 -D_CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES_COUNT=1 -D_CRT_SECURE_NO_DEPRECATE=1 -D_CRT_NONSTDC_NO_DEPRECATE=1 -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 
CFLAGSD = -nologo -Gy -W3 -Wp64 -GS- -Od -MDd -Zi -D_DLL  -DWIN32 -DPC -DWINNT -DVC_EXTRALEAN -DWIN32_LEAN_AND_MEAN -DNOSERVICE -DNOMCX -DNOIME -DNOSOUND -DNOCOMM -DNOKANJI -DNORPC -DNOPROXYSTUB -DNOIMAGE -DNOTAPE -DNOMINMAX -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1 -D_CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES_COUNT=1 -D_CRT_SECURE_NO_DEPRECATE=1 -D_CRT_NONSTDC_NO_DEPRECATE=1 -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 
LDFLAGS	= /SUBSYSTEM:windows /NOLOGO /INCREMENTAL:no /MACHINE:AMD64
LDFLAGSD= /DEBUG /SUBSYSTEM:windows /NOLOGO /INCREMENTAL:no /MACHINE:AMD64
LIBS	= shell32.lib user32.lib advapi32.lib gdi32.lib comctl32.lib

!IFDEF vwUNICODE
CUCDEFS = -DUNICODE -D_UNICODE
!ENDIF

SRC	= VWDesktopIcons.c
COFFS   = VWDesktopIcons.coff
OBJRES  = VWDesktopIcons.res

TARGET	= VWDesktopIcons.exe
OBJS    = $(SRC:.c=.o)

TARGETD	= VWDesktopIconsD.exe
OBJSD   = $(SRC:.c=.od)

.SUFFIXES: .rc .res .coff .c .o .od
.c.o:
	$(CC) $(CFLAGS) $(CUCDEFS) -c $< -Fo$@
.c.od:
	$(CC) $(CFLAGSD) $(CUCDEFS) -c $< -Fo$@
.rc.res:	
	$(RC) -v -fo $@ $*.rc 

$(TARGET): $(OBJS) $(OBJRES)
	$(LD) $(LDFLAGS) /MANIFEST /MANIFESTFILE:$@.manifest /out:$@ $(OBJS) $(OBJRES) $(LIBS)
        $(MT) /nologo /outputresource:$@;#1 /manifest $@.manifest

$(TARGETD): $(TARGETD)
$(TARGETD): $(OBJSD) $(OBJRES)
	$(LD) $(LDFLAGSD) /MANIFEST /MANIFESTFILE:$@.manifest /out:$@ $(OBJSD) $(OBJRES) $(LIBS)

all:    clean $(TARGET)

alld:   clean $(TARGETD)

clean: 
	- erase $(OBJS) $(OBJSD) $(OBJRES) vc60.pch

spotless: clean
	- erase $(TARGET) $(TARGETD) $(TARGET).manifest $(TARGETD).manifest $(COFFS)
