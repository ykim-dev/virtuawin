//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  ConfigParameters.h - Constant definitions used.
// 
//  Copyright (c) 1999-2005 Johan Piculell
//  Copyright (c) 2006-2007 VirtuaWin (VirtuaWin@home.se)
// 
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
//  USA.
//

#ifndef _DEFINES_H_
#define _DEFINES_H

#include <windows.h>

// application name and version defines
#define vwVIRTUAWIN_NAME         _T("VirtuaWin")
#define vwVIRTUAWIN_CLASSNAME    _T("VirtuaWinMainClass")
#define vwVIRTUAWIN_EMAIL        _T("VirtuaWin@home.se")
#define vwVIRTUAWIN_NAME_VERSION _T("VirtuaWin v3.2")

// Various defines used on several places 
#define vwHOTKEY_MAX      40       // max number of hotkeys
#define vwWINDOW_MAX     160       // max number of windows to handle
#define vwWINHASH_SIZE   509       // size of the window hash table
#define vwDESKTOP_MAX     20       // max number of desktops
#define vwDESKTOP_SIZE    (vwDESKTOP_MAX + 2)
#define vwCLASSNAME_MAX   64       // class name buffer size
#define vwWINDOWNAME_MAX 128       // window name buffer size
#define vwMODULENAME_MAX  79       // Maximum length of a module name (buffer needs to be n+1 long)

#define MAXMODULES        10       // max number of modules to handle

// Internal windows messages
#define VW_SYSTRAY        (WM_USER + 1)  // Sent to us by the systray
#define VW_MOUSEWARP      (WM_USER + 9)  // Mouse thread message

#define vwTRICKY_POSITION 1
#define vwTRICKY_WINDOW   2

#define vwHOTKEY_ALT     MOD_ALT
#define vwHOTKEY_CONTROL MOD_CONTROL
#define vwHOTKEY_SHIFT   MOD_SHIFT
#define vwHOTKEY_WIN     MOD_WIN
#define vwHOTKEY_EXT     0x10

#define vwWINFLAGS_INITIALIZED     0x0001
#define vwWINFLAGS_FOUND           0x0002
#define vwWINFLAGS_ACTIVATED       0x0004
#define vwWINFLAGS_VISIBLE         0x0008
#define vwWINFLAGS_MINIMIZED       0x0010
#define vwWINFLAGS_WINDOW          0x0020
#define vwWINFLAGS_MANAGED         0x0040
#define vwWINFLAGS_SHOWN           0x0080
#define vwWINFLAGS_SHOW            0x0100
#define vwWINFLAGS_STICKY          0x0200
#define vwWINFLAGS_TRICKY          0x0C00
#define vwWINFLAGS_TRICKY_TYPE     0x0400
#define vwWINFLAGS_TRICKY_POS      0x0800
#define vwWINFLAGS_NO_TASKBAR_BUT  0x1000
#define vwWINFLAGS_RM_TASKBAR_BUT  0x2000

typedef unsigned int   vwUInt ;
typedef unsigned short vwUShort ;
typedef unsigned char  vwUByte ;

// Holds data far a non-managed window
typedef struct vwWindowBase { 
    struct vwWindowBase *next ;
    struct vwWindowBase *hash ;
    HWND                 handle ;
    vwUInt               flags ;
} vwWindowBase;

// Holds data far a managed window, start must be the same as vwWindowBase
typedef struct vwWindow { 
    struct vwWindow     *next ;
    struct vwWindow     *hash ;
    HWND                 handle ;
    vwUInt               flags ;
    HWND                 owner;
    long                 exStyle;
    vwUInt               zOrder[vwDESKTOP_SIZE] ;
    vwUShort             menuId ;
    vwUByte              desk;
} vwWindow ;

typedef struct vwWindowType {
    struct vwWindowType *next;
    TCHAR               *match;
    vwUByte              desk;
    vwUByte              type;
} vwWindowType ;

// Holds data for modules
typedef struct {
    HWND      handle;
    TCHAR     description[vwMODULENAME_MAX+1];
    vwUByte   disabled;
} moduleType;

// Holds disabled modules
typedef struct {
    TCHAR     moduleName[vwMODULENAME_MAX+1];
} disModules;

typedef struct {
    TCHAR    *name;
    HICON     icon; 
    vwUInt    zOrder ;
    vwUShort  id;
    vwUByte   desk;
    vwUByte   sticky;
} vwMenuItem ;

typedef struct {
    ATOM     atom ;
    vwUByte  key ;
    vwUByte  modifier ;
    vwUByte  command ;
    vwUByte  desk ;
} vwHotkey ;

#endif
