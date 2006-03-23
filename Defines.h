//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K/XP
// 
//  Copyright (c) 1999-2003, 2004 Johan Piculell
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

// application name and version defines
#define vwVIRTUAWIN_NAME    "VirtuaWin"
#define vwVIRTUAWIN_VERSION "v3.0 beta 2"
#define vwVIRTUAWIN_EMAIL   "VirtuaWin@home.se"
#define vwVIRTUAWIN_NAME_VERSION vwVIRTUAWIN_NAME " " vwVIRTUAWIN_VERSION

// Various defines used on several places 
#define MAXWIN 100                // max number of windows to handle
#define MAXDESK 11                // max number of desktops (0 - not used, 1 - 9 - normal, 10 private) 
#define MAXUSER 10                // max number of user windows to search for
#define UWM_SYSTRAY (WM_USER + 1) // Sent to us by the systray
#define MAXMODULES 10             // max number of modules to handle
#define vwCLASSNAME_MAX  47       // maximum length of a class name (buffer needs to be n+1 long)
#define vwMODULENAME_MAX 79       // Maximum length of a module name (buffer needs to be n+1 long)
#define vwDESK_PRIVATE1  10       // Deesk number of the first private desk

// Internal messages for mouse controlling
#define VW_MOUSEWARP  WM_USER + 90
#define VW_MOUSEUP    WM_USER + 91
#define VW_MOUSEDOWN  WM_USER + 92
#define VW_MOUSELEFT  WM_USER + 93
#define VW_MOUSERIGHT WM_USER + 94
#define VW_MOUSERESET WM_USER + 95

#endif
