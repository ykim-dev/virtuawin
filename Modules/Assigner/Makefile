# VirtuaWin (virtuawin.sourceforge.net)
# Makefile - VWDesktopIcons make file for MinGW & Cygwin GNU GCC.
#
# Copyright (c) 2006-2007 VirtuaWin (VirtuaWin@home.se)
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

OSTYPE   = $(shell uname -msr)

ifeq ($(findstring CYGWIN,$(OSTYPE)),CYGWIN)
CC      = i686-pc-mingw32-gcc
CFLAGS	= -Wall -O2 -DNDEBUG
CFLAGSD = -Wall -g
LDFLAGS	= -mwindows -O2
LDFLAGSD= -mwindows -g
RC      = i686-pc-mingw32-windres 
STRIP	= strip
endif

ifeq ($(findstring MINGW32,$(OSTYPE)),MINGW32)
CC      = gcc
CFLAGS	= -Wall -O2 -DNDEBUG
CFLAGSD = -Wall -g
LDFLAGS	= -mwindows -O2
LDFLAGSD= -mwindows -g
RC      = windres 
STRIP	= strip
endif

ifeq ($(findstring Linux,$(OSTYPE)),Linux)
CC      = i586-mingw32msvc-gcc
CFLAGS  = -Wall -O2 -DNDEBUG
CFLAGSD = -Wall -g
LDFLAGS = -mwindows -O2
LDFLAGSD= -mwindows -g
RC	= i586-mingw32msvc-windres
STRIP	= strip
endif

ifeq ($(vwUNICODE),1)
CUCDEFS = -DUNICODE -D_UNICODE
endif

SRC     = assigner.c
COFFS   = assigner.coff
OBJRES  = assigner.res
LIBS	= -lcomctl32

TARGET	= VWAssigner.exe
OBJS    = $(SRC:.c=.o)

TARGETD	= VWAssignerD.exe
OBJSD   = $(SRC:.c=.od)

.SUFFIXES: .rc .res .coff .c .o .od

.c.o:
	$(CC) $(CFLAGS) $(CUCDEFS) -c -o $@ $<
.c.od:
	$(CC) $(CFLAGSD) $(CUCDEFS) -c -o $@ $<

$(TARGET): $(OBJS) $(COFFS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(COFFS) $(LIBS)
	$(STRIP) $(TARGET)

$(TARGETD): $(OBJSD) $(COFFS)
	$(CC) $(LDFLAGSD) -o $(TARGETD) $(OBJSD) $(COFFS) $(LIBS)

assigner.coff: assigner.rc
	$(RC) --input-format rc --output-format coff -o $@ -i $<

all:    clean $(TARGET)

alld:   clean $(TARGETD)

clean: 
	rm -f $(OBJS) $(OBJSD) $(COFFS)

spotless: clean 
	rm -f $(TARGET) $(TARGETD) $(OBJRES) vc60.pch
