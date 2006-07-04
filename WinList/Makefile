# VirtuaWin (virtuawin.sourceforge.net)
# Makefile - WinList make file for MinGW & Cygwin GNU GCC.
#
# Copyright (c) 2006 VirtuaWin (VirtuaWin@home.se)
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
CC      = gcc
CFLAGS	= -mno-cygwin -Wall -O2
CFLAGSD = -mno-cygwin -Wall -g
LDFLAGS	= -mwindows -mno-cygwin -O2
LDFLAGSD= -mwindows -mno-cygwin -g
RC      = windres 
STRIP	= strip
endif

ifeq ($(findstring MINGW32,$(OSTYPE)),MINGW32)
CC      = gcc
CFLAGS	= -Wall -O2
CFLAGSD = -Wall -g
LDFLAGS	= -mwindows -O2
LDFLAGSD= -mwindows -g
RC      = windres 
STRIP	= strip
endif

ifeq ($(findstring Linux,$(OSTYPE)),Linux)
CC      = i586-mingw32msvc-gcc
CFLAGS  = -Wall -O2
CFLAGSD = -Wall -g
LDFLAGS = -mwindows -O2
LDFLAGSD= -mwindows -g
RC	= i586-mingw32msvc-windres
STRIP	= strip
endif

SRC	= winlist.c
COFFS   = winlist.coff
OBJRES  = winlist.res

TARGET	= WinList.exe
OBJS    = $(SRC:.c=.o)

TARGETD	= WinListD.exe
OBJSD   = $(SRC:.c=.od)

.SUFFIXES: .rc .res .coff .c .o .od
.c.o:
	$(CC) $(CFLAGS) $(CVBDEFS) $(CVDDEFS) $(CVTDEFS) -c -o $@ $<
.c.od:
	$(CC) $(CFLAGSD) $(CVBDEFS) $(CVDDEFS) $(CVTDEFS) -c -o $@ $<

$(TARGET): $(OBJS) $(COFFS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(COFFS) $(LIBS)
	$(STRIP) $(TARGET)

$(TARGETD): $(OBJSD) $(COFFS)
	$(CC) $(LDFLAGSD) -o $(TARGETD) $(OBJSD) $(COFFS) $(LIBS)

winlist.coff: winlist.rc
	$(RC) --input-format rc --output-format coff -o $@ -i $<

all:    clean $(TARGET)

alld:   clean $(TARGETD)

clean: 
	rm -f $(OBJS) $(OBJSD) $(COFFS)

spotless: clean 
	rm -f  $(TARGET) $(TARGETD) $(OBJRES) vc60.pch
