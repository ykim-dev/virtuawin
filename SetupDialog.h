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

static BOOL APIENTRY mouse(HWND, UINT, UINT, LONG);
static BOOL APIENTRY keys(HWND, UINT, UINT, LONG);
static BOOL APIENTRY misc(HWND, UINT, UINT, LONG);
static BOOL APIENTRY modules(HWND, UINT, UINT, LONG);
static BOOL APIENTRY about(HWND, UINT, UINT, LONG);

int CALLBACK propCallBack(HWND hwndDlg, UINT uMsg, LPARAM lParam);

#endif

/*
 * $Log$
 * Revision 1.4  2001/02/05 21:13:08  jopi
 * Updated copyright header
 *
 * Revision 1.3  2001/01/28 16:26:56  jopi
 * Configuration behaviour change. It is now possible to test all settings by using apply and all changes will be rollbacked if cancel is pressed
 *
 * Revision 1.2  2001/01/12 18:14:34  jopi
 * Modules will now get a notification when desktop layout has changed since we might have a new current desktop number after a change. Also fixed so that config update notification is sent upon apply and only when something has changed upon hitting ok. Config file will also be written upon every apply and not if cancel is selected
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  jopi
 * Added first time
 *
 */
