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

BOOL configChanged;

#endif

/*
 * $Log$
 * Revision 1.2  2001/01/12 18:14:34  jopi
 * Modules will now get a notification when desktop layout has changed since we might have a new current desktop number after a change. Also fixed so that config update notification is sent upon apply and only when something has changed upon hitting ok. Config file will also be written upon every apply and not if cancel is selected
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  jopi
 * Added first time
 *
 */
