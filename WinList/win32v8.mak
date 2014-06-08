# VirtuaWin (virtuawin.sourceforge.net)
# win32v8.mak - WinList make file for Microsoft MSVC v8.0
#
# Copyright (c) 2006-2014 VirtuaWin (VirtuaWin@home.se)
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
CFLAGS	= -nologo -W2 -EHsc -O2 -MT -DNDEBUG -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NON_CONFORMING_SWPRINTFS
CFLAGSD = -nologo -W2 -Z7 -RTC1 -EHsc -Od -MTd -D_DEBUG -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NON_CONFORMING_SWPRINTFS
LDFLAGS	= -nologo -subsystem:windows -incremental:no -machine:I386
LDFLAGSD= -nologo -debug -subsystem:windows -incremental:no -machine:I386
LIBS	= shell32.lib user32.lib gdi32.lib comctl32.lib

!IFDEF vwUNICODE
CUCDEFS = -DUNICODE -D_UNICODE
!ENDIF

SRC	= winlist.c
COFFS   = winlist.coff
OBJRES  = winlist.res

TARGET	= WinList.exe
OBJS    = $(SRC:.c=.o)

TARGETD	= WinListD.exe
OBJSD   = $(SRC:.c=.od)

.SUFFIXES: .rc .res .coff .c .o .od
.c.o:
	$(CC) $(CFLAGS) $(CUCDEFS) -c $< -Fo$@
.c.od:
	$(CC) $(CFLAGSD) $(CUCDEFS) -c $< -Fo$@
.rc.res:	
	$(RC) -v -fo $@ $*.rc 


$(TARGET): $(OBJS) $(OBJRES)
	$(LD) $(LDFLAGS) /out:$@ $(OBJS) $(OBJRES) $(LIBS)

$(TARGETD): $(OBJSD) $(OBJRES)
	$(LD) $(LDFLAGSD) /out:$@ $(OBJSD) $(OBJRES) $(LIBS)

all:    clean $(TARGET)

alld:   clean $(TARGETD)

clean: 
	- erase $(OBJS) $(OBJSD) $(OBJRES) vc60.pch

spotless: clean
	- erase $(TARGET) $(TARGETD) $(COFFS)
