#ifndef _SETUPDIALOG_H_
#define _SETUPDIALOG_H_

#include <windows.h>

int createPropertySheet(HINSTANCE theHinst, HWND theHwndOwner);

extern void reLoadIcons();
extern void unloadModules();
extern void loadModules();

static BOOL APIENTRY mouse(HWND, UINT, UINT, LONG);
static BOOL APIENTRY keys(HWND, UINT, UINT, LONG);
static BOOL APIENTRY misc(HWND, UINT, UINT, LONG);
static BOOL APIENTRY modules(HWND, UINT, UINT, LONG);
static BOOL APIENTRY about(HWND, UINT, UINT, LONG);

#endif

/*
 * $Log$
 */
