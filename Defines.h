//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K
// 
//  Copyright (c) 1999, 2000, 2001 jopi
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

// Various defines used on several places 

#define MAXWIN 100  // max number of windows to handle
#define MAXUSER 10  // max number of user windows to search for
#define UWM_SYSTRAY (WM_USER + 1) // Sent to us by the systray
#define MAXMODULES 10 // max number of modules to handle

// Internal messages for mouse controlling
#define VW_MOUSEWARP WM_USER + 90
#define VW_MOUSEUP   WM_USER + 91
#define VW_MOUSEDOWN WM_USER + 92
#define VW_MOUSELEFT WM_USER + 93
#define VW_MOUSERIGHT WM_USER + 94
#define VW_MOUSERESET WM_USER + 95

#endif

/*
 * $Log$
 * Revision 1.3  2001/01/28 16:10:24  jopi
 * Added #ifndef/#define
 *
 * Revision 1.2  2000/08/21 20:54:32  jopi
 * More than MAXMODULES modules would corrupt data and cause undefined behaviour, also increased value from 5 to 10
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  jopi
 * Added first time
 *
 */
