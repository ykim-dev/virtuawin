//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  ListStructures.h - Type definition of structures used.
// 
//  Copyright (c) 1999-2005 Johan Piculell
//  Copyright (c) 2006 VirtuaWin (VirtuaWin@home.se)
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

#ifndef _LISTSTRUCTURES_H_
#define _LISTSTRUCTURES_H_

// Standard includes
#include <windows.h>
#include "Defines.h"

#define vwTRICKY_POSITION 1
#define vwTRICKY_WINDOW   2

#define vwVISIBLE_NO      0
#define vwVISIBLE_YES     1
#define vwVISIBLE_TEMP    2
#define vwVISIBLE_YESTEMP 3

typedef struct { // Holds the windows in the list
    HWND           Handle;
    HWND           Owner;
    long           Style;
    long           ExStyle;
    unsigned long  ZOrder[MAXDESK] ;
    unsigned short Desk;
    unsigned short menuId ;
    unsigned char  Sticky;
    unsigned char  Tricky;
    unsigned char  Visible;
    unsigned char  State;
} windowType;

typedef struct { // Holds data for modules
    HWND Handle;
    BOOL Disabled;
    char description[vwMODULENAME_MAX+1];
} moduleType;

typedef struct { // Holds disabled modules
    char moduleName[vwMODULENAME_MAX+1];
} disModules;

typedef struct _MenuItems
{
    char          *name;
    HICON          icon; 
    unsigned short id;
    unsigned short desk;
    unsigned char  sticky;
} MenuItem;

typedef struct { // Holds desktop assigned windows
    char          *match;
    unsigned short desk;
    unsigned char  type;
} vwWindowMatch ;

windowType winList[MAXWIN];               // list for holding windows
moduleType moduleList[MAXMODULES];        // list that holds modules
disModules disabledModules[MAXMODULES*2]; // list with disabled modules

vwWindowMatch userList[MAXUSER];          // list for holding user added applications
vwWindowMatch stickyList[MAXWIN];         // list with saved sticky windows
vwWindowMatch trickyList[MAXWIN];         // list with saved tricky windows
vwWindowMatch assignedList[MAXWIN];       // list with all windows that have a predefined desktop

#endif
