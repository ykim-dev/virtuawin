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

#ifndef _SETUPDIALOG_H_
#define _SETUPDIALOG_H_

#include <windows.h>

int createPropertySheet(HINSTANCE theHinst, HWND theHwndOwner);

extern void registerAllKeys();
extern void unRegisterAllKeys();
extern void reLoadIcons();
extern void unloadModules();
extern void loadModules();
extern void enableMouse(BOOL);
extern void setMouseKey();

BOOL APIENTRY mouse(HWND, UINT, UINT, LONG);
BOOL APIENTRY keys(HWND, UINT, UINT, LONG);
BOOL APIENTRY misc(HWND, UINT, UINT, LONG);
BOOL APIENTRY modules(HWND, UINT, UINT, LONG);
BOOL APIENTRY expert(HWND, UINT, UINT, LONG);
BOOL APIENTRY about(HWND, UINT, UINT, LONG);

int CALLBACK propCallBack(HWND hwndDlg, UINT uMsg, LPARAM lParam);

#endif

/*
 * $Log$
 * Revision 1.9  2004/01/10 11:15:52  jopi
 * Updated copyright for 2004
 *
 * Revision 1.8  2003/01/27 20:23:54  jopi
 * Updated copyright header for 2003
 *
 * Revision 1.7  2002/12/23 14:16:48  jopi
 * Added a new setup tab, "expert" and moved some settings from misc.
 *
 * Revision 1.6  2002/02/14 21:23:40  Johan Piculell
 * Updated copyright header
 *
 * Revision 1.5  2001/02/10 11:11:54  Administrator
 * Removed the context help icon since there is no functionality for this
 *
 * Revision 1.4  2001/02/05 21:13:08  Administrator
 * Updated copyright header
 *
 * Revision 1.3  2001/01/28 16:26:56  Administrator
 * Configuration behaviour change. It is now possible to test all settings by using apply and all changes will be rollbacked if cancel is pressed
 *
 * Revision 1.2  2001/01/12 18:14:34  Administrator
 * Modules will now get a notification when desktop layout has changed since we might have a new current desktop number after a change. Also fixed so that config update notification is sent upon apply and only when something has changed upon hitting ok. Config file will also be written upon every apply and not if cancel is selected
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  Administrator
 * Added first time
 *
 */
