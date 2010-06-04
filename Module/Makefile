# VirtuaWin (virtuawin.sourceforge.net)
# Makefile - Module make file for MinGW & Cygwin GNU GCC.
#
# Copyright (c) 2006-2010 VirtuaWin (VirtuaWin@home.se)
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
CFLAGS	= -mno-cygwin -Wall -O2 -DNDEBUG
CFLAGSD = -mno-cygwin -Wall -g
LDFLAGS	= -mwindows -mno-cygwin -O2
LDFLAGSD= -mwindows -mno-cygwin -g
RC      = windres 
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

SRC=Module.c

TARGET	= Module.exe
OBJS    = $(SRC:.c=.o)

TARGETD	= ModuleD.exe
OBJSD   = $(SRC:.c=.od)

.SUFFIXES: .rc .res .coff .c .o .od

.c.o:
	$(CC) $(CFLAGS) $(CUCDEFS) -c -o $@ $<
.c.od:
	$(CC) $(CFLAGSD) $(CUCDEFS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	$(STRIP) $(TARGET)

$(TARGETD): $(OBJSD)
	$(CC) $(LDFLAGSD) -o $(TARGETD) $(OBJSD) $(LIBS)

all:    clean $(TARGET)

alld:   clean $(TARGETD)

clean: 
	rm -f $(OBJS) $(OBJSD)

spotless: clean 
	rm -f $(TARGET) $(TARGETD) vc60.pch
