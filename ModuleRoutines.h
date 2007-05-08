//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  DiskRoutines.h - Module routine function definitions.
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

#ifndef _MODULEROUTINES_H_
#define _MODULEROUTINES_H_

#include <windows.h>

void loadModules(void);
void unloadModules(void);
void sendModuleMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
void postModuleMessage(UINT Msg, WPARAM wParam, LPARAM lParam);

#endif
