//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K/XP
// 
//  Copyright (c) 1999, 2000, 2001, 2002, 2003 Johan Piculell
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

#ifndef _MODULEROUTINES_H_
#define _MODULEROUTINES_H_

#include <windows.h>

void loadModules();
void addModule(struct _finddata_t*);
void unloadModules();
BOOL checkDisabledList(char*);
void sendModuleMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
void postModuleMessage(UINT Msg, WPARAM wParam, LPARAM lParam);

#endif

/*
 * $Log$
 * Revision 1.4  2002/12/29 15:20:39  jopi
 * Fixed copyright info.
 *
 * Revision 1.3  2002/02/14 21:23:40  jopi
 * Updated copyright header
 *
 * Revision 1.2  2001/02/05 21:13:08  jopi
 * Updated copyright header
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  jopi
 * Added first time
 *
 */
