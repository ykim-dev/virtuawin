# VirtuaWin (virtuawin.sourceforge.net)
# win32v6.mak - WinList make file for Microsoft MSVC v6.0
#
# Copyright (c) 2006-2008 VirtuaWin (VirtuaWin@home.se)
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
CFLAGS	= -nologo -MD -W3 -YX -GX -O2 -DNDEBUG -DWIN32
CFLAGSD = -nologo -MDd -W3 -YX -GX -Z7 -GZ -Yd -Od -D_DEBUG -DWIN32
LDFLAGS	= -nologo -subsystem:windows -incremental:no -machine:I386
LDFLAGSD= -nologo -subsystem:windows -incremental:no -machine:I386 -debug
LIBS	= shell32.lib user32.lib gdi32.lib comctl32.lib

SRC	= VWPreview.c
COFFS   = VWPreview.coff
OBJRES  = VWPreview.res

TARGET	= VWPreview.exe
OBJS    = $(SRC:.c=.o)
HDRS    = Resource.h

TARGETD	= VWPreviewD.exe
OBJSD   = $(SRC:.c=.od)

.SUFFIXES: .rc .res .coff .c .o .od
.c.o:
	$(CC) $(CFLAGS) $(CUCDEFS) -c $< -Fo$@
.c.od:
	$(CC) $(CFLAGSD) $(CUCDEFS) -c $< -Fo$@
.rc.res:	
	$(RC) -v -i "$(TOOLSDIR)\include" -i "$(TOOLSDIR)\mfc\include" -fo $@ $*.rc 

$(TARGET): $(OBJS) $(OBJRES)
	$(LD) $(LDFLAGS) /out:$@ $(OBJS) $(OBJRES) $(LIBS)

$(TARGETD): $(OBJSD) $(OBJRES)
	$(LD) $(LDFLAGSD) /out:$@ $(OBJSD) $(OBJRES) $(LIBS)

all:    clean $(TARGET)

alld:   clean $(TARGETD)

$(OBJSD) $(OBJRES): $(HDRS)

clean: 
	- erase $(OBJS) $(OBJSD) $(OBJRES) vc60.pch

spotless: clean
	- erase $(TARGET) $(TARGETD) $(COFFS)
