# Wedit Makefile for project VirtuaWin
INCLUDEPATH=c:\lcc\include
CFLAGS=-I$(INCLUDEPATH) -O
LINKFLAGS=-subsystem windows
CC=lcc.exe
TARGET=VirtuaWin.exe
OBJS = VirtuaWin.res VirtuaWin.obj DiskRoutines.obj SetupDialog.obj ModuleRoutines.obj

LIBS = shell32.lib

$(TARGET):	$(OBJS) Makefile
	lcclnk $(LINKFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Build VirtuaWin.res
VIRTUAWIN_RC=\
	$(INCLUDEPATH)\windows.h \
	$(INCLUDEPATH)\win.h \
	$(INCLUDEPATH)\limits.h \
	$(INCLUDEPATH)\stdarg.h \
	resource.h \

VirtuaWin.res:	$(VIRTUAWIN_RC) VirtuaWin.rc
	lrc -Ic:\VirtuaWin -Ic:\lcc\include  VirtuaWin.rc

# Build VIRTUAWIN.C
VIRTUAWIN_C=\
	VirtuaWin.h \
	Resource.h \
	$(INCLUDEPATH)\windows.h \
	$(INCLUDEPATH)\win.h\
	$(INCLUDEPATH)\limits.h\
	$(INCLUDEPATH)\stdarg.h\
	$(INCLUDEPATH)\shellapi.h\
	$(INCLUDEPATH)\stdio.h\
	$(INCLUDEPATH)\_syslist.h\
	$(INCLUDEPATH)\stdlib.h\
	$(INCLUDEPATH)\stddef.h\
	$(INCLUDEPATH)\string.h\
	$(INCLUDEPATH)\commctrl.h\
	$(INCLUDEPATH)\math.h\
	$(INCLUDEPATH)\io.h\
	$(INCLUDEPATH)\sys\stat.h\

VirtuaWin.obj: $(VIRTUAWIN_C) VirtuaWin.c 
	$(CC) -c $(CFLAGS) VirtuaWin.c 

DiskRoutines.obj: $(DISKROUTINES_C) DiskRoutines.c
	$(CC) -c $(CFLAGS) DiskRoutines.c 

SetupDialog.obj: $(SETUPDIALOG_C) Setupdialog.c
	$(CC) -c $(CFLAGS) Setupdialog.c 

ModuleRoutines.obj: $(MODULEROUTINES_C) ModuleRoutines.c
	$(CC) -c $(CFLAGS) ModuleRoutines.c 

clean: 
	@rm $(OBJS)

all: clean $(TARGET)
