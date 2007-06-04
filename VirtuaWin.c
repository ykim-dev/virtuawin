//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  VirtuaWin.c - Core VirtuaWin routines.
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

// Includes
#include "VirtuaWin.h"
#include "DiskRoutines.h"
#include "SetupDialog.h"
#include "ConfigParameters.h"
#include "Messages.h"
#include "Resource.h"
#include "ModuleRoutines.h"
#include "regex.h"

// Standard includes
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <commctrl.h>

/*#define _WIN32_MEMORY_DEBUG*/
#ifdef _WIN32_MEMORY_DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#define calculateDesk(x,y) (((y) * nDesksX) - (nDesksX - (x)))

// Variables
HWND hWnd;		  // handle to VirtuaWin
HANDLE hMutex;
BOOL taskbarFixRequired;  // TRUE if tricky window tasks need to be continually hidden
BOOL mouseEnabled=TRUE;   // Status of the mouse thread, always running at startup 
HANDLE mouseThread;       // Handle to the mouse thread
int curAssigned = 0;      // how many predefined desktop belongings we have (saved)
int curSticky = 0;        // how many stickywindows we have (saved)
int curTricky = 0;        // how many tricky windows we have (saved)
int curUser = 0;          // how many user applications we have

int taskbarEdge;
int desktopWorkArea[2][4] ;

UINT RM_Shellhook;

BOOL keysRegistred = FALSE;	// if the switch keys are registrered
BOOL hotKeysRegistred = FALSE;	// if the switch hot keys are registrered
BOOL cyclingKeysRegistered = FALSE; // if the cycling keys are registrered
BOOL menuHotKeyRegistered = FALSE;  // if the menu hotkey is registered

ATOM vwLeft;
ATOM vwRight;
ATOM vwUp;
ATOM vwDown;
ATOM vwDesk[MAXDESK];
ATOM vwWList;
ATOM vwWMenu;
ATOM cyclingKeyUp;
ATOM cyclingKeyDown;
ATOM stickyKey;
ATOM dismissKey;

BOOL enabled = TRUE;		// if VirtuaWin enabled or not
BOOL isDragging = FALSE;	// if we are currently dragging a window

HINSTANCE hInst;		// current instance
HWND taskHWnd;                  // handle to taskbar
HWND desktopHWnd;		// handle to the desktop window
HWND deskIconHWnd;		// handle to the desktop window holding the icons
HWND lastFGHWnd;		// handle to the last foreground window
int  lastFGStyle;               // style of the last foreground window
HWND lastBOFGHWnd;		// handle to the last but one foreground window
int  lastBOFGStyle;             // style of the last but one foreground window
HWND lastLostHWnd;		// handle to the last lost window
HWND setupHWnd;       		// handle to the setup dialog, NULL if not open
BOOL setupOpen;         

// vector holding icon handles for the systray
HICON icons[MAXDESK];           // 0=disabled, 1=9=nromal desks, 10=private desk
NOTIFYICONDATA nIconD;

// Config parameters, see ConfigParameters.h for descriptions
int nOfModules = 0;  
int nWin = 0;        
int currentDeskX = 1;
int currentDeskY = 1;
int currentDesk = 1; 
int nDesksX = 2;     
int nDesksY = 2;     
int warpLength = 60;
int knockMode = 2;
int mouseDelay = 20;
int preserveZOrder = 0;      
int hiddenWindowAct = 1;
int vwLogFlag = 0 ;
FILE *vwLogFile ;
BOOL useMouseKey = FALSE;
BOOL noMouseWrap = TRUE;
BOOL keyEnable = TRUE;		
BOOL hotKeyEnable = FALSE;      
BOOL releaseFocus = FALSE;	
BOOL minSwitch = TRUE;		
UINT mouseEnable = 0; 
UINT modAlt = 0;		
UINT modShift = 0;		
UINT modCtrl = 0;		
UINT modWin = MOD_WIN;		
UINT mouseModAlt = 0;	
UINT mouseModShift = 0;	
UINT mouseModCtrl = 1;	
BOOL saveSticky = FALSE;        
BOOL refreshOnWarp = FALSE;     
BOOL stickyKeyRegistered = FALSE;
BOOL dismissKeyRegistered = FALSE;
BOOL deskWrap = FALSE;          
BOOL invertY = FALSE;           
short stickyMenu = 1;         
short assignMenu = 1;
short directMenu = 1;
short compactMenu = 0;
BOOL useDeskAssignment = FALSE;
BOOL saveLayoutOnExit = FALSE;
BOOL assignOnlyFirst = FALSE;
BOOL assignImmediately = TRUE;
BOOL cyclingKeysEnabled = FALSE;
BOOL displayTaskbarIcon = TRUE;
BOOL noTaskbarCheck = FALSE;
BOOL trickyWindows = TRUE;
BOOL permanentSticky = FALSE;

UINT MOUSEKEY = 0;           // Holds the modifier for enabling mouse warp
UINT deskHotkey[MAXDESK];
UINT deskHotkeyMod[MAXDESK];
UINT deskHotkeyWin[MAXDESK];
UINT hotCycleUp = 0;
UINT hotCycleUpMod = 0;
UINT hotCycleUpWin = 0;
UINT hotCycleDown = 0;
UINT hotCycleDownMod = 0;
UINT hotCycleDownWin = 0;
UINT hotkeyWListEn = 0;
UINT hotkeyWList = 0;
UINT hotkeyWListMod = 0;
UINT hotkeyWListWin = 0;
UINT hotkeyWMenuEn = 0;
UINT hotkeyWMenu = 0;
UINT hotkeyWMenuMod = 0;
UINT hotkeyWMenuWin = 0;
UINT hotkeyStickyEn = 0;
UINT hotkeySticky = 0;
UINT hotkeyStickyMod = 0;
UINT hotkeyStickyWin = 0;
UINT hotkeyDismissEn = 0;
UINT hotkeyDismiss = 0;
UINT hotkeyDismissMod = 0;
UINT hotkeyDismissWin = 0;

int curDisabledMod = 0; 
unsigned long vwZOrder=0 ;
unsigned long timerCounter=0 ;

/* desk image generation variables */
int         deskImageCount=-1 ;
HBITMAP     deskImageBitmap=NULL ;
BITMAPINFO  deskImageInfo ;
void       *deskImageData=NULL ;

// icon for sticky checkmarks
HICON   checkIcon;

enum {
    OSVERSION_UNKNOWN=0,
    OSVERSION_31,
    OSVERSION_9X,
    OSVERSION_NT,
    OSVERSION_2000,
    OSVERSION_XP
} ;
int osVersion ;
    
#define windowIsNotHung(hWnd,waitTime) (SendMessageTimeout(hWnd,(int)NULL,0,0,SMTO_ABORTIFHUNG|SMTO_BLOCK,waitTime,NULL))

#define vwSHWIN_TRYHARD   0x01
static BOOL showHideWindow(windowType *aWindow, int shwFlags, unsigned char show);
static int changeDesk(int newDesk, WPARAM msgWParam) ;

void
vwLogPrint(const TCHAR *format, ...)
{
    if(vwLogEnabled())
    {
        SYSTEMTIME stime;
        va_list ap;
    
        GetLocalTime (&stime);
        _ftprintf(vwLogFile,_T("[%04d-%02d-%02d %02d:%02d:%02d:%03d] "),
                  stime.wYear, stime.wMonth, stime.wDay,
                  stime.wHour, stime.wMinute, stime.wSecond, stime.wMilliseconds) ;
        va_start(ap, format);
        _vftprintf(vwLogFile,format,ap) ;
        fflush(vwLogFile) ;
        va_end(ap);
    }
}

/************************************************
 * Locks the window list protection
 */
void lockMutex(void)
{
    if (hMutex != (HANDLE)0 && WaitForSingleObject(hMutex,0) == WAIT_TIMEOUT) {
        WaitForSingleObject(hMutex,INFINITE);
    }
}

/************************************************
 * Releases the window list protection
 */
void releaseMutex(void)
{
    if (hMutex != (HANDLE)0) ReleaseMutex(hMutex);
}

/*************************************************
 * Checks if mouse button pressed on title bar (i.e. dragging window)
 */
static unsigned char
checkMouseState(BOOL force)
{
    static unsigned char lastState=0, lastBState=0 ;
    unsigned char thisBState ;
    LPARAM lParam ;
    HWND hwnd ;
    POINT pt ;
    DWORD rr ;
    
    // Check the state of mouse buttons
    if(GetSystemMetrics(SM_SWAPBUTTON))
    {
        thisBState = (HIWORD(GetAsyncKeyState(VK_RBUTTON)) != 0) ;
        if(HIWORD(GetAsyncKeyState(VK_MBUTTON)))
            thisBState |= 2 ;
        if(HIWORD(GetAsyncKeyState(VK_LBUTTON)))
            thisBState |= 4 ;
    }
    else
    {
        thisBState = (HIWORD(GetAsyncKeyState(VK_LBUTTON)) != 0) ;
        if(HIWORD(GetAsyncKeyState(VK_MBUTTON)))
            thisBState |= 2 ;
        if(HIWORD(GetAsyncKeyState(VK_RBUTTON)))
            thisBState |= 4 ;
    }
    if((thisBState != lastBState) || (force && (thisBState == 1)))
    {
        lastState = (thisBState) ? 4:0 ;
        if((thisBState == 1) || (thisBState == 2))
        {
            GetCursorPos(&pt);
            if(((hwnd=WindowFromPoint(pt)) == deskIconHWnd) || (hwnd == desktopHWnd))
            {
                if(thisBState == 2)
                    lastState = 3 ;
            }
            else if((hwnd != NULL) && (hwnd != taskHWnd))
            {
                lParam = (((int)(short) pt.y) << 16) | ((int)(short) pt.x) ;
                if((SendMessageTimeout(hwnd,WM_NCHITTEST,0,lParam,SMTO_ABORTIFHUNG|SMTO_BLOCK,50,&rr) ||
                    (Sleep(1),SendMessageTimeout(hwnd,WM_NCHITTEST,0,lParam,SMTO_ABORTIFHUNG|SMTO_BLOCK,100,&rr))) &&
                   (rr == HTCAPTION))
                    lastState = thisBState ;
            }
        }
        vwLogVerbose((_T("Got new state %d (%d %d %d %x %d) %x %x %x\n"),(int) lastState,(int) thisBState,pt.x,pt.y,hwnd,rr,desktopHWnd,deskIconHWnd,taskHWnd)) ;
        lastBState = thisBState ;
    }
    return lastState ;
}

/*************************************************
 * The mouse thread. This function runs in a thread and checks the mouse
 * position every 50ms. If mouse support is disabled, the thread will be in 
 * suspended state.
 */
DWORD WINAPI MouseProc(LPVOID lpParameter)
{
    unsigned char mode, lastMode=0, state[4], newState, wlistState=0, wmenuState=0 ;
    int ii, newPos, pos[4], statePos[4], delayTime[4], wlistX=0, wlistY=0 ;
    POINT pt;
    
    state[0] = state[1] = state[2] = state[3] = 0 ;
    // infinite loop
    while(1)
    {
        Sleep(25); 
        
        mode = checkMouseState(0) ;
        if(mouseEnable & 0x0c)
        {
            if(mode == 3)
            {
                GetCursorPos(&pt);
                if(wlistState == 0)
                {
                    wlistState = 1 ;
                    wlistX = pt.x ;
                    wlistY = pt.y ;
                }
                else if(mouseEnable & 8)
                {
                    if((ii=(warpLength >> 1)) < 10)
                        ii = 10 ;
                    newPos = -1 ;
                    if(abs(pt.x - wlistX) < abs(pt.y - wlistY))
                    {
                        if((pt.y - wlistY) >= ii)
                            newPos = 3 ;
                        else if((wlistY - pt.y) >= ii)
                            newPos = 1 ;
                    }
                    else
                    {
                        if((pt.x - wlistX) >= ii)
                            newPos = 2 ;
                        else if((wlistX - pt.x) >= ii)
                            newPos = 0 ;
                    }
                    if(newPos >= 0)
                    {
                        vwLogBasic((_T("Mouse mddle button desk change %d (%d,%d)\n"),newPos,pt.x - wlistX,pt.y - wlistY)) ;
                        /* send the switch message and wait until done */
                        SendMessage(hWnd, VW_MOUSEWARP, 0, MAKELPARAM(0,newPos)) ;
                        Sleep(100) ;
                        GetCursorPos(&pt);
                        wlistX = pt.x ;
                        wlistY = pt.y ;
                        wlistState = 2 ;
                    }
                }
            }
            else if(wlistState)
            {
                if((mode == 0) && (wlistState == 1) && (mouseEnable & 4))
                {
                    vwLogBasic((_T("Mouse wlist %d\n"),wlistState)) ;
                    SendMessage(hWnd, VW_MOUSEWARP, 0, MAKELPARAM(0,4)) ;
                }
                wlistState = 0 ;
            }
        }
        if((mouseEnable & 2) && ((mode == 2) || wmenuState))
        {
            if(mode == 2)
                wmenuState = 1 ;
            else
            {
                if((mode == 0) && (wmenuState == 1))
                {
                    vwLogBasic((_T("Mouse wmenu %d\n"),wmenuState)) ;
                    SendMessage(hWnd, VW_MOUSEWARP, 0, MAKELPARAM(0,5)) ;
                }
                wmenuState = 0 ;
            }
        }
        if((mouseEnable & 1) && ((mode == 0) || (mode == 1)) &&
           (!useMouseKey || HIWORD(GetAsyncKeyState(MOUSEKEY))))
        {
            GetCursorPos(&pt);
            pos[0] = pt.x - desktopWorkArea[mode][0] ;
            pos[1] = pt.y - desktopWorkArea[mode][1] ;
            pos[2] = desktopWorkArea[mode][2] - pt.x ;
            pos[3] = desktopWorkArea[mode][3] - pt.y ;
            ii = 3 ;
            if(mode != lastMode)
            {
                /* the state of the left button has changed, rest the mode. Must also try
                 * to handle clicks on the taskbar and other non-workarea locations */
                do
                    state[ii] = (pos[ii] >= warpLength) ;
                while(--ii >= 0) ;
                lastMode = mode ;
            }
            else
            {
                do {
                    if((newState = state[ii]) == 4)
                    {
                        if(pos[ii] > 0)
                            newState = 3 ;
                        else if(++delayTime[ii] >= mouseDelay)
                        {
                            vwLogBasic((_T("Mouse desk change on edge %d (%d,%d)\n"),ii,(int) pt.x,(int) pt.y)) ;
                            /* send the switch message and wait until done */
                            SendMessage(hWnd, VW_MOUSEWARP, 0, MAKELPARAM(mode|2,ii)) ;
                            newState = 1 ;
                        }
                        else
                            continue ;
                    }
                    else if(pos[ii] >= warpLength)
                    {
                        if(newState == 0)
                            newState = 1 ;
                    }
                    else if(pos[ii] <= 0)
                    {
                        if((knockMode & 1) && ((newState == 0) || ((newState == 1) && (knockMode & 2))))
                            newState = 2 ;
                        else if((newState == 3) || (newState <= 1))
                            newState = 4 ;
                    }
                    else if((newState == 2) && (pos[ii] >= (warpLength >> 2)))
                        newState = 3 ;
                    if(newState != state[ii])
                    {
                        newPos = (ii & 0x01) ? pt.x:pt.y ;
                        if((state[ii] > 1) && (abs(statePos[ii]-newPos) > warpLength))
                        {
                            /* newState must also be greater than 2, check the mouse movement is accurate enough */
                            vwLogBasic((_T("State %d (%d %d): Changed %d -> 1 (position: %d,%d)\n"),ii,mode,pos[ii],state[ii],(int) pt.x,(int) pt.y)) ;
                            state[ii] = 1 ;
                        }
                        else
                        {
#ifdef vwLOG_VERBOSE
                            if(vwLogEnabled())
#else
                            if(vwLogEnabled() && ((newState > 1) || (state[ii] > 1)))
#endif
                                vwLogPrint(_T("State %d (%d %d): Change %d -> %d (%d,%d)\n"),ii,mode,pos[ii],state[ii],newState,(int) pt.x,(int) pt.y) ;
                            state[ii] = newState ;
                            if(newState == 2)
                                statePos[ii] = newPos ;
                            delayTime[ii] = 0 ;
                        }
                    }
                    else if((state[ii] > 1) && (++delayTime[ii] >= 20))
                    {
                        /* burnt in 1sec timer, a knock must take no more than 1 sec second (2 * (20 * 25ms)) */
                        vwLogBasic((_T("State %d (%d %d): Changed %d -> 1 (timer)\n"),ii,mode,pos[ii],newState)) ;
                        state[ii] = 1 ;
                    }
                } while(--ii >= 0) ;
            }
        }
        else
            lastMode = -1 ;
    }
    
    return TRUE;
}

/************************ *************************
 * Turns on/off the mouse thread. Makes sure that the the thread functions
 * only is called if needed.
 */
void enableMouse(BOOL turnOn)
{
    // Try to turn on thread if not already running
    if( turnOn && !mouseEnabled )
    {
        ResumeThread(mouseThread);
        mouseEnabled = TRUE;
    }
    // Try to turn of thread if already not stopped
    else if( !turnOn && mouseEnabled )
    {
        SuspendThread(mouseThread);
        mouseEnabled = FALSE;
    }
}

/*************************************************
 * Sets the icon in the systray and updates the currentDesk variable
 */
static void setIcon(int theNumber)
{
    nIconD.hIcon = icons[theNumber];
    if(displayTaskbarIcon)
        Shell_NotifyIcon(NIM_MODIFY, &nIconD);
}

/************************ *************************
 * Loads the icons for the systray according to the current setup
 */
static void loadIcons(void)
{
    int xIcon = GetSystemMetrics(SM_CXSMICON);
    int yIcon = GetSystemMetrics(SM_CYSMICON);
    int ii, iconId ;
    TCHAR buff[16] ;
    
    if(nDesksY != 2 || nDesksX != 2) // if 2 by 2 mode
        iconId = IDI_ST_0 ;
    else if(osVersion > OSVERSION_2000)
        iconId = IDI_ST_DIS_2 ;
    else
        iconId = IDI_ST_DIS_1 ;
    
    _tcscpy(buff,_T("icons/X.ico")) ;
    ii = nDesksY * nDesksX ;
    do {
        /* Try to load user defined icons */
        buff[6] = '0' + ii ;
        icons[ii] = (HICON) LoadImage(hInst, buff, IMAGE_ICON, xIcon, yIcon, LR_LOADFROMFILE);
        if(icons[ii] == NULL)
            /* use the built in standard */
            icons[ii] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(iconId+ii), IMAGE_ICON, xIcon, yIcon, 0);
    } while(--ii >= 0) ;
    if(deskHotkey[vwDESK_PRIVATE1] != 0)
        /* create icon for the private desktop - use the disabled icon */
        icons[vwDESK_PRIVATE1] = icons[0] ;
    // Load checkmark icon for sticky
    checkIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_CHECK));
}

/*************************************************
 * Resets all icons and reloads them
 */
void reLoadIcons(void)
{
    int ii=MAXDESK-1;
    do
        icons[ii] = NULL;
    while(--ii >= 0) ;
    loadIcons();
    setIcon(currentDesk);
}

/*************************************************
 * Sets the modifier key for switching desktop with number mouse
 */
void setMouseKey(void)
{
    if(mouseModAlt == TRUE)
        MOUSEKEY = VK_MENU;
    else if(mouseModShift == TRUE)
        MOUSEKEY = VK_SHIFT;
    else if(mouseModCtrl == TRUE)
        MOUSEKEY = VK_CONTROL;
}

/*************************************************
 * Register the keys to use for switching desktop with the arrow keys
 */
static BOOL registerKeys(void)
{
    UINT MODKEY;
    if(keyEnable && !keysRegistred) {
        keysRegistred = TRUE;
        MODKEY = modAlt | modShift | modCtrl | modWin;
        vwLeft = GlobalAddAtom(_T("atomKeyLeft"));
        if((RegisterHotKey(hWnd, vwLeft, MODKEY, VK_LEFT) == FALSE))
            return FALSE;
        vwRight = GlobalAddAtom(_T("atomKeyRight"));
        if((RegisterHotKey(hWnd, vwRight, MODKEY, VK_RIGHT) == FALSE))
            return FALSE;
        vwUp = GlobalAddAtom(_T("atomKeyUp"));
        if((RegisterHotKey(hWnd, vwUp, MODKEY, VK_UP) == FALSE))
            return FALSE;
        vwDown = GlobalAddAtom(_T("atomKeyDown"));
        if((RegisterHotKey(hWnd, vwDown, MODKEY, VK_DOWN) == FALSE))
            return FALSE;
        return TRUE;
    }
    return TRUE;
}

/*************************************************
 * Unegister the keys to use for switching desktop
 */
static void unRegisterKeys(void)
{
    if(keysRegistred) {
        keysRegistred = FALSE;
        UnregisterHotKey(hWnd, vwLeft);
        UnregisterHotKey(hWnd, vwRight);
        UnregisterHotKey(hWnd, vwUp);
        UnregisterHotKey(hWnd, vwDown);
    }
}

/*************************************************
 * Translates virtual key codes to "hotkey codes"
 */
static UINT hotKey2ModKey(UINT vModifiers)
{
    WORD mod = 0;
    if (vModifiers & HOTKEYF_ALT)
        mod |= MOD_ALT;
    if (vModifiers & HOTKEYF_CONTROL)
        mod |= MOD_CONTROL;
    if (vModifiers & HOTKEYF_SHIFT)
        mod |= MOD_SHIFT;
    return mod;
}

/*************************************************
 * Register the hotkeys to use for switching desktop and the winlist hotkey
 */
static BOOL registerHotKeys(void)
{
    TCHAR buff[16];
    int ii ;
    if(!hotKeysRegistred)
    {
        hotKeysRegistred = TRUE;
        _tcscpy(buff,_T("atomKeyP")) ;
        if(deskHotkey[vwDESK_PRIVATE1])
        {
            // private desktop
            vwDesk[vwDESK_PRIVATE1] = GlobalAddAtom(buff);
            if((RegisterHotKey(hWnd, vwDesk[vwDESK_PRIVATE1], hotKey2ModKey(deskHotkeyMod[vwDESK_PRIVATE1]) | deskHotkeyWin[vwDESK_PRIVATE1],
                               deskHotkey[vwDESK_PRIVATE1]) == FALSE))
                return FALSE;
        }
        if(hotKeyEnable)
        {
            ii = nDesksY * nDesksX ;
            do {
                if(deskHotkey[ii])
                {
                    buff[7] = '0'+ii ;
                    vwDesk[ii] = GlobalAddAtom(buff);
                    if((RegisterHotKey(hWnd, vwDesk[ii], hotKey2ModKey(deskHotkeyMod[ii]) | deskHotkeyWin[ii],
                                       deskHotkey[ii]) == FALSE))
                        return FALSE;
                }
            } while(--ii > 0) ;
        }
    }
    return TRUE;
}

/*************************************************
 * Unregister the hot keys
 */
static void unRegisterHotKeys(void)
{
    int ii ;
    if(hotKeysRegistred) {
        hotKeysRegistred = FALSE;
        ii = MAXDESK-1 ;
        do {
            if(vwDesk[ii] != 0)
            {
                UnregisterHotKey(hWnd,vwDesk[ii]);
                vwDesk[ii] = 0 ;
            }
        } while(--ii > 0) ;
    }
}

/*************************************************
 * Register the desktop cycling hot keys, if defined
 */
static BOOL registerCyclingKeys(void)
{
    if(!cyclingKeysRegistered && cyclingKeysEnabled)
    {
        cyclingKeysRegistered = TRUE;
        cyclingKeyUp = GlobalAddAtom(_T("VWCyclingKeyUp"));
        cyclingKeyDown = GlobalAddAtom(_T("VWCyclingKeyDown"));
        if(hotCycleUp && (RegisterHotKey(hWnd, cyclingKeyUp, hotKey2ModKey(hotCycleUpMod) | hotCycleUpWin, hotCycleUp) == FALSE))
            return FALSE;
        if(hotCycleDown && (RegisterHotKey(hWnd, cyclingKeyDown, hotKey2ModKey(hotCycleDownMod) | hotCycleDownWin, hotCycleDown) == FALSE))
            return FALSE;
    }
    return TRUE;
}

/*************************************************
 * Unregister the cycling  hot keys, if previosly registred
 */
static void unRegisterCyclingKeys(void)
{
    if(cyclingKeysRegistered)
    {
        cyclingKeysRegistered = FALSE;
        UnregisterHotKey(hWnd, cyclingKeyUp);
        UnregisterHotKey(hWnd, cyclingKeyDown);
    }
}

/*************************************************
 * Register the window menu hot key, if defined
 */
static BOOL registerMenuHotKey(void)
{
    if(!menuHotKeyRegistered)
    {
        menuHotKeyRegistered = TRUE;
        vwWList = GlobalAddAtom(_T("atomKeyWList"));
        vwWMenu = GlobalAddAtom(_T("atomKeyWMenu"));
        if(hotkeyWListEn && hotkeyWList && (RegisterHotKey(hWnd, vwWList, hotKey2ModKey(hotkeyWListMod) | hotkeyWListWin, hotkeyWList) == FALSE))
            return FALSE;
        if(hotkeyWMenuEn && hotkeyWMenu && (RegisterHotKey(hWnd, vwWMenu, hotKey2ModKey(hotkeyWMenuMod) | hotkeyWMenuWin, hotkeyWMenu) == FALSE))
            return FALSE;
    }
    return TRUE;
}

/*************************************************
 * Unregister the window menu hot key, if previosly registred
 */
static void unRegisterMenuHotKey(void)
{
    if(menuHotKeyRegistered)
    {
        menuHotKeyRegistered = FALSE ;
        UnregisterHotKey(hWnd,vwWList) ;
        UnregisterHotKey(hWnd,vwWMenu) ;
    }
}

/*************************************************
 * Register the sticky hot key, if defined
 */
static BOOL registerStickyKey(void)
{
    if(!stickyKeyRegistered && hotkeyStickyEn && hotkeySticky) {
        stickyKeyRegistered = TRUE;
        stickyKey = GlobalAddAtom(_T("VWStickyKey"));
        if((RegisterHotKey(hWnd, stickyKey, hotKey2ModKey(hotkeyStickyMod) | hotkeyStickyWin, hotkeySticky) == FALSE))
            return FALSE;
        else
            return TRUE;
    }
    return TRUE;
}

/*************************************************
 * Unregister the sticky hot key, if previosly registred
 */
static void unRegisterStickyKey(void)
{
    if(stickyKeyRegistered) {
        stickyKeyRegistered = FALSE;
        UnregisterHotKey(hWnd, stickyKey);
    }
}

/*************************************************
 * Register the dismiss hot key, if defined
 */
static BOOL registerDismissKey(void)
{
    if(!dismissKeyRegistered && hotkeyDismissEn && hotkeyDismiss) {
        dismissKeyRegistered = TRUE;
        dismissKey = GlobalAddAtom(_T("VWDismissKey"));
        if((RegisterHotKey(hWnd, dismissKey, hotKey2ModKey(hotkeyDismissMod) | hotkeyDismissWin, hotkeyDismiss) == FALSE))
            return FALSE;
        else
            return TRUE;
    }
    return TRUE;
}

/*************************************************
 * Unregister the dismiss hot key, if previosly registred
 */
static void unRegisterDismissKey(void)
{
    if(dismissKeyRegistered)
    {
        dismissKeyRegistered = FALSE;
        UnregisterHotKey(hWnd, dismissKey);
    }
}

/************************************************
 * Convinient function for registering all hotkeys
 */
void registerAllKeys(void)
{
    if(!registerKeys())
        MessageBox(hWnd,_T("Invalid key modifier combination, check cursor hotkeys!"),
                   vwVIRTUAWIN_NAME _T(" Error"), MB_ICONWARNING);
    if(!registerCyclingKeys())
        MessageBox(hWnd,_T("Invalid key modifier combination, check cycling hotkeys!"),
                   vwVIRTUAWIN_NAME _T(" Error"), MB_ICONWARNING);
    if(!registerHotKeys())
        MessageBox(hWnd,_T("Invalid key modifier combination, check hotkeys!"),
                   vwVIRTUAWIN_NAME _T(" Error"), MB_ICONWARNING);
    if(!registerMenuHotKey())
        MessageBox(hWnd,_T("Invalid key modifier combination, check menu hotkey!"), 
                   vwVIRTUAWIN_NAME _T(" Error"), MB_ICONWARNING);
    if(!registerStickyKey())
        MessageBox(hWnd,_T("Invalid key modifier combination, check sticky hotkey!"),
                   vwVIRTUAWIN_NAME _T(" Error"), MB_ICONWARNING);
    if(!registerDismissKey())
        MessageBox(hWnd,_T("Invalid key modifier combination, check dismiss window hotkey!"),
                   vwVIRTUAWIN_NAME _T(" Error"), MB_ICONWARNING);
}

/************************************************
 * Convinient function for unregistering all hotkeys
 */
void unRegisterAllKeys(void)
{
    unRegisterDismissKey();
    unRegisterStickyKey();
    unRegisterMenuHotKey();
    unRegisterHotKeys();
    unRegisterCyclingKeys();
    unRegisterKeys();
}

/* The virtual screen size system matrix values were only added for WINVER >= 0x0500 (Win2k) */
#ifndef SM_CMONITORS
#define SM_XVIRTUALSCREEN       76
#define SM_YVIRTUALSCREEN       77
#define SM_CXVIRTUALSCREEN      78
#define SM_CYVIRTUALSCREEN      79
#define SM_CMONITORS            80
#endif
/************************************************
 * Get screen width and height and store values in
 * global variables
 */
static void getScreenSize(void)
{
    if((desktopWorkArea[0][2] = GetSystemMetrics(SM_CXVIRTUALSCREEN)) <= 0)
    {
        /* The virtual screen size system matrix values are not supported on
         * this OS (Win95 & NT), use the desktop window size */
        RECT r;
        GetClientRect(desktopHWnd, &r);
        desktopWorkArea[0][0] = r.left;
        desktopWorkArea[0][1] = r.top;
        desktopWorkArea[0][2] = r.right - 1 ;
        desktopWorkArea[0][3] = r.bottom - 1 ;
    }
    else
    {
        desktopWorkArea[0][0]  = GetSystemMetrics(SM_XVIRTUALSCREEN);
        desktopWorkArea[0][1]  = GetSystemMetrics(SM_YVIRTUALSCREEN);
        desktopWorkArea[0][2] += desktopWorkArea[0][0] - 1 ;
        desktopWorkArea[0][3]  = GetSystemMetrics(SM_CYVIRTUALSCREEN) + desktopWorkArea[0][1] - 1;
    }
    vwLogBasic((_T("Got screen size: %d %d -> %d %d\n"),
                desktopWorkArea[0][0],desktopWorkArea[0][1],desktopWorkArea[0][2],desktopWorkArea[0][3])) ;
}

/************************************************
 * Tries to locate the handle to the taskbar
 */
static void goGetTheTaskbarHandle(void)
{
    if(!noTaskbarCheck)
    {
        HWND hwndTray = FindWindowEx(NULL, NULL,_T("Shell_TrayWnd"), NULL);
        HWND hwndBar = FindWindowEx(hwndTray, NULL,_T("ReBarWindow32"), NULL );
        
        // Maybe "RebarWindow32" is not a child to "Shell_TrayWnd", then try this
        if(hwndBar == NULL)
            hwndBar = hwndTray;
        
        taskHWnd = FindWindowEx(hwndBar, NULL,_T("MSTaskSwWClass"), NULL);
        
        if(taskHWnd == NULL)
            MessageBox(hWnd,_T("Could not locate handle to the taskbar.\n This will disable the ability to hide troublesome windows correctly."),vwVIRTUAWIN_NAME _T(" Error"), 0); 
    }
}

/************************************************
 * Grabs and stores the workarea of the screen
 */
void getWorkArea(void)
{
    RECT r;
    
    if((GetSystemMetrics(SM_CMONITORS) <= 1) && SystemParametersInfo(SPI_GETWORKAREA,0,&r,0))
    {
        desktopWorkArea[1][0] = r.left;
        desktopWorkArea[1][1] = r.top;
        desktopWorkArea[1][2] = r.right - 1 ; 
        desktopWorkArea[1][3] = r.bottom - 1 ;
    }
    else
    {
        desktopWorkArea[1][0] = desktopWorkArea[0][0] ;
        desktopWorkArea[1][1] = desktopWorkArea[0][1] ;
        desktopWorkArea[1][2] = desktopWorkArea[0][2] ;
        desktopWorkArea[1][3] = desktopWorkArea[0][3] ;
    }
    vwLogBasic((_T("Got work area (%d %d): %d %d -> %d %d  &  %d %d -> %d %d\n"),noTaskbarCheck,taskbarEdge,
                desktopWorkArea[0][0],desktopWorkArea[0][1],desktopWorkArea[0][2],desktopWorkArea[0][3],
                desktopWorkArea[1][0],desktopWorkArea[1][1],desktopWorkArea[1][2],desktopWorkArea[1][3])) ;
}

/************************************************
 * desk image generation functions
 */
static int
createDeskImage(int deskNo, int createDefault)
{
    HDC deskDC ;
    HDC bitmapDC ;
    HBITMAP oldmap;
    TCHAR fname[MAX_PATH] ;
    FILE *fp ;
    int ret ;
    
    if((deskImageCount == 0) ||
       ((deskDC = GetDC(desktopHWnd)) == NULL) ||
       ((bitmapDC = CreateCompatibleDC(deskDC)) == NULL))
        return 0 ;
    
    oldmap = (HBITMAP) SelectObject(bitmapDC,deskImageBitmap);
    
    if(createDefault || (deskNo >= vwDESK_PRIVATE1))
    {
        /* create a default image */
        RECT rect;
        rect.left   = 0 ;
        rect.top    = 0 ;
        rect.right  = deskImageInfo.bmiHeader.biWidth ;
        rect.bottom = deskImageInfo.bmiHeader.biHeight ;
        FillRect(bitmapDC,&rect,(HBRUSH) (COLOR_BACKGROUND+1)) ;
    }
    else
    {
        /* can set to HALFTONE for better quality, but not supported on Win95/98/Me */
        SetStretchBltMode(bitmapDC,COLORONCOLOR);
        StretchBlt(bitmapDC,0,0,deskImageInfo.bmiHeader.biWidth,deskImageInfo.bmiHeader.biHeight,deskDC,
                   desktopWorkArea[0][0],desktopWorkArea[0][1],desktopWorkArea[0][2]-desktopWorkArea[0][0]+1,
                   desktopWorkArea[0][3]-desktopWorkArea[0][1]+1,SRCCOPY);    
    }

    /* Create the desk_#.bmp file */ 
    GetFilename(vwFILE_COUNT,1,fname) ;
    _stprintf(fname+_tcslen(fname),_T("desk_%d.bmp"),deskNo) ;
    if(GetDIBits(bitmapDC,deskImageBitmap,0,deskImageInfo.bmiHeader.biHeight,deskImageData,&deskImageInfo,DIB_RGB_COLORS) &&
       ((fp = _tfopen(fname,_T("wb+"))) != NULL))
    {
        BITMAPFILEHEADER hdr ;
        hdr.bfType = 0x4d42 ;
        hdr.bfOffBits = (DWORD) (sizeof(BITMAPFILEHEADER) + deskImageInfo.bmiHeader.biSize) ;
        hdr.bfSize = hdr.bfOffBits + deskImageInfo.bmiHeader.biSizeImage ; 
        hdr.bfReserved1 = 0 ;
        hdr.bfReserved2 = 0 ;
        
        ret = ((fwrite(&hdr,sizeof(BITMAPFILEHEADER),1,fp) == 1) &&  
               (fwrite(&deskImageInfo,deskImageInfo.bmiHeader.biSize,1,fp) == 1) &&
               (fwrite(deskImageData,deskImageInfo.bmiHeader.biSizeImage,1,fp) == 1)) ;
        
        fclose(fp) ;
        if(!ret)
            DeleteFile(fname) ;
    }
    else
        ret = 0 ;
    SelectObject(bitmapDC,oldmap);
    DeleteDC(bitmapDC);
    ReleaseDC(desktopHWnd,deskDC) ;
    vwLogBasic((_T("createDeskImage: %d: %d %d - %d %d\n"),ret,deskNo,createDefault,(int) deskImageInfo.bmiHeader.biWidth,(int) deskImageInfo.bmiHeader.biHeight)) ;
    return ret ;
}

static int
disableDeskImage(int count)
{
    if(deskImageCount <= 0)
        return 0 ;
    if((deskImageCount -= count) <= 0)
    {
        if(deskImageData != NULL)
        {
            free(deskImageData) ;
            deskImageData = NULL ;
        }
        if(deskImageBitmap != NULL)
        {
            DeleteObject(deskImageBitmap) ;
            deskImageBitmap = NULL ;
        }
        deskImageInfo.bmiHeader.biHeight = 0 ;
        deskImageCount = 0 ;
    }
    return 1 ;
}

static int
enableDeskImage(int height)
{
    int width, biSizeImage, count=deskImageCount ;
    
    if(height <= 0)
        return 0 ;
    if((count <= 0) || (deskImageInfo.bmiHeader.biHeight < height))
    {
        HDC deskDC = GetDC(desktopHWnd) ;
        
        if(count > 0)
            disableDeskImage(count) ;
        
        if((width = (height * (desktopWorkArea[0][2]-desktopWorkArea[0][0]+1)) / (desktopWorkArea[0][3]-desktopWorkArea[0][1]+1)) <= 0)
            width = 1 ;
        /* the GetDIBits function returns 24 bit RGB even if the bitmap is set to 32 bit rgba so fix
         * the BMP to 24 bit RGB, not sure what would happen on a palette based system. However VW
         * crashes if deskImageData is only w*h*3, found w*h*4 avoids the problem. */
        biSizeImage = (((width * 3) + 3) & ~3) * height ; 
        if(((deskImageBitmap = CreateCompatibleBitmap(deskDC,width,height)) != NULL) &&
           ((deskImageData = malloc(biSizeImage)) != NULL))
        {
            deskImageInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
            deskImageInfo.bmiHeader.biWidth = width ; 
            deskImageInfo.bmiHeader.biHeight = height ; 
            deskImageInfo.bmiHeader.biPlanes = 1 ; 
            deskImageInfo.bmiHeader.biBitCount = 24 ; 
            deskImageInfo.bmiHeader.biCompression = BI_RGB ;
            deskImageInfo.bmiHeader.biSizeImage = biSizeImage ;
            deskImageInfo.bmiHeader.biXPelsPerMeter = 0 ;
            deskImageInfo.bmiHeader.biYPelsPerMeter = 0 ;
            deskImageInfo.bmiHeader.biClrUsed = 0 ;
            deskImageInfo.bmiHeader.biClrImportant = 0 ;
            vwLogBasic((_T("initDeskImage succeeded: %d - %d %d\n"),
                        height,(int) deskImageInfo.bmiHeader.biWidth,(int) deskImageInfo.bmiHeader.biHeight)) ;
        }
        else
            vwLogBasic((_T("initDeskImage failed: %d, %x, %x\n"),height,(int) deskImageBitmap,(int) deskImageData)) ; 
        ReleaseDC(desktopHWnd,deskDC);
        if(deskImageData == NULL)
            return 0 ;
    }
    if(count < 0)
    {
        /* first time enabled, create default images for all desks */
        count = nDesksX * nDesksY ;
        do
            createDeskImage(count,1) ;
        while(--count > 0) ;
        count = 0 ;
    }
    deskImageCount = count + 1 ;
    return 1 ;
}

/************************************************
 * Show the VirtuaWin help pages
 */
void showHelp(HWND aHWnd, UINT context)
{
    TCHAR buff[MAX_PATH];
    GetFilename(vwHELP,0,buff);
    if(context)
        WinHelp(aHWnd, buff, HELP_CONTEXT, context) ;
    else
        WinHelp(aHWnd, buff, HELP_CONTENTS, 0) ;
}

/*************************************************
 * Returns index if window is found, -1 otherwise
 */
static int winListFind(HWND hwnd)
{
    int index = nWin ;
    while(--index >= 0)
        if(winList[index].Handle == hwnd)
            break ;
    return index ;
}

static BOOL checkIfWindowMatch(vwWindowMatch *wm, TCHAR *className, TCHAR *windowName)
{
    /* this is not very optimal, ideally the match string and class/window
     * name would all be in multibyte form as the regex does not support
     * WCHAR. But because of the way the user list is currently implemented
     * it is more efficient to convert the WCHAR to a byte string here */
    if(wm->type & 0x02)
    {
        static meRegex regex ;
        int ll ;
#ifdef _UNICODE
        char name[1024] ;
        
        if(!WideCharToMultiByte(CP_ACP,0,wm->match,-1,name,1024, 0, 0) ||
           (meRegexComp(&regex,name,0) != meREGEX_OKAY) ||
           !WideCharToMultiByte(CP_ACP,0,(wm->type & 0x01) ? windowName:className,-1,name,1024, 0, 0))
            return 0 ;
#else
        char *name ;
        if(meRegexComp(&regex,wm->match,0) != meREGEX_OKAY)
            return 0 ;
        name = (wm->type & 0x01) ? windowName:className ;
#endif
        ll = strlen(name) ;
        return meRegexMatch(&regex,name,ll,0,ll,(meREGEX_BEGBUFF|meREGEX_ENDBUFF)) ;
    }
    else if(wm->type & 0x01)
        return (!_tcscmp(wm->match,windowName)) ;
    return (!_tcscmp(wm->match,className)) ;
}

/************************************************
 * Checks if this window is saved as a tricky window that needs the 
 * "move technique" to be hidden.
 */
static BOOL checkIfTricky(TCHAR *className, TCHAR *windowName, RECT *pos)
{
    int ii, ret=0 ; 
    vwLogVerbose((_T("checkIfTricky [%s] [%s] %d %d\n"),className,windowName,(int)pos->left,(int)pos->top)) ;
    if(trickyWindows)
    {
        ii = curTricky ;
        while(--ii >= 0)
        {
            if(checkIfWindowMatch(&(trickyList[ii]),className,windowName)) 
            {
                ret = vwTRICKY_WINDOW ;
                break ;
            }
        }
        if((pos->left == -32000) && (pos->top == -32000))
            // some apps hide the window by pushing it to -32000, the windows
            // cannot be moved from here VirtualWin handles these by making them
            // Tricky (not hiding them) so crash recovery will still find them.
            ret |= vwTRICKY_POSITION ;
    }
    return ret ;  
}

/*************************************************
 * Checks if a window is a previous saved sticky window
 */
static BOOL checkIfSticky(TCHAR *className, TCHAR *windowName)
{
    int i=curSticky ;
    
    vwLogVerbose((_T("checkIfSticky [%s] [%s]\n"),className,windowName)) ;
    while(--i >= 0)
    {
        if(checkIfWindowMatch(&(stickyList[i]),className,windowName)) 
            // Typically user windows will loose their stickiness if
            // minimized, therefore we do not remove their name from 
            // the list as done above.
            return TRUE;
    }
    return FALSE;
}

/*************************************************
 * Checks if a window is an predifined desktop to go to
 */
static int checkIfAssigned(TCHAR *className, TCHAR *windowName)
{
    int i;
    vwLogVerbose((_T("checkIfAssigned [%s] [%s]\n"),className,windowName)) ;
    for(i = 0; i < curAssigned; ++i) 
    {
        if((assignedList[i].type & 0x04) == 0)
        {
            vwLogVerbose((_T("Assign comparing [%s] [%s] with %d [%s]\n"),className,windowName,(int) assignedList[i].type,assignedList[i].match)) ;
            
            if(checkIfWindowMatch(&(assignedList[i]),className,windowName)) 
            {
                if(assignOnlyFirst)
                    assignedList[i].type |= 0x04 ;
                if((assignedList[i].desk > (nDesksX * nDesksY)) && (assignedList[i].desk != vwDESK_PRIVATE1))
                    MessageBox(hWnd,_T("Tried to assign an application to an unavaliable desktop.\nIt will not be assigned.\nCheck desktop assignmet configuration."),vwVIRTUAWIN_NAME _T(" Error"), MB_ICONERROR); 
                else
                    return assignedList[i].desk ; // Yes, assign
            }
        }
    }
    return currentDesk; // No assignment, return current
}

/*************************************************
 * Forces a window into the foreground. Must be done in this way to avoid
 * the flashing in the taskbar insted of actually changing active window.
 */
static void setForegroundWin(HWND theWin, int makeTop)
{
    DWORD ThreadID1;
    DWORD ThreadID2;
    HWND cwHwnd ;
    int index, err ;
    
    vwLogBasic((_T("setForegroundWin: %x %d (%x)\n"),(int) theWin,makeTop,(int) hWnd)) ;
    if(theWin == NULL)
    {
        /* releasing the focus, the current foreground window MUST be
         * released otherwise VW can think its a popup. Assigning the focus
         * to a window when no window has the focus is prone to failure so
         * give the focus to VW's hidden window rather than to the Desktop */
        theWin = hWnd ;
        makeTop = 0 ;
    }
    else if(((index = winListFind(theWin)) < 0) || !winList[index].Visible ||
            !windowIsNotHung(theWin,100))
    {
        /* don't make the foreground a hidden or non-managed or hung window */
        vwLogBasic((_T("SetForground: %8x - %d %d or HUNG\n"),(int) theWin,index,winList[index].Visible)) ;
        return ;
    }    
    index = 2 ;
    for(;;)
    {
        /* do not bother making it the foreground if it already is */
        if((cwHwnd = GetForegroundWindow()) == theWin)
            break ;
        if(cwHwnd != hWnd)
        {
            /* To get the required permission to make theWin the active window, VW must become the active window.
             * To get the required permissions to do that VW must attatch to the thread of the current active window.
             * But don't do this if the current process hung */
            if((cwHwnd != NULL) && windowIsNotHung(cwHwnd,100))
            {
                /* Get the thread responsible for VirtuaWin,
                   and the thread for the foreground window */
                ThreadID1 = GetWindowThreadProcessId(cwHwnd, NULL);
                ThreadID2 = GetWindowThreadProcessId(hWnd, NULL);
                /* By sharing input state, threads share their concept of
                   the active window */
                if(ThreadID1 != ThreadID2)
                {
                    AttachThreadInput(ThreadID1, ThreadID2, TRUE);
                    err = SetForegroundWindow(hWnd) ; 
                    // Set VirtuaWin active. Don't no why, but it seems to work
                    AttachThreadInput(ThreadID1, ThreadID2, FALSE);
                    vwLogVerbose((_T("Attached to foreground Window: %x - %x\n"),(int) cwHwnd,(int) GetForegroundWindow())) ;
                }
                else
                {
                    SetForegroundWindow(hWnd) ; 
                    vwLogVerbose((_T("VW owns foreground Window: %x - %x\n"),(int) cwHwnd,(int) GetForegroundWindow())) ;
                }
            }
            else
            {
                SetForegroundWindow(hWnd) ; 
                vwLogVerbose((_T("No foreground Window or hung: %x - %x\n"),(int) cwHwnd,(int) GetForegroundWindow())) ;
            }
        }
        SetForegroundWindow(theWin) ;
        /* SetForegroundWindow can return success (non-zero) but not succeed (GetForegroundWindow != theWin)
         * Getting the foreground window right is really important because if the existing foreground window
         * is left as the foreground window but hidden (common when moving the app or desk) VW will confuse
         * it with a popup */
        cwHwnd = GetForegroundWindow() ;
        vwLogBasic((_T("Set foreground window: %d, %x -> %x\n"),(theWin == cwHwnd),(int) theWin,(int) cwHwnd)) ;
        if((cwHwnd == theWin) || (--index < 0))
            break ;
        /* A short sleep allows the rest of the system to catch up */
        vwLogVerbose((_T("About to FG sleep\n"))) ;
        Sleep(1) ;
    }
    /* bring to the front if requested as swapping desks can muddle the order */
    if(makeTop)
        BringWindowToTop(theWin);
}

/************************************************
 * Show the setup dialog and perform some stuff before and after display
 */
static void showSetup(void)
{
    if(!setupOpen)
    {
        // reload load current config
        readConfig();
        vwLogVerbose((_T("About to call createPropertySheet\n"))) ;
        createPropertySheet(hInst,hWnd);
        vwLogVerbose((_T("createPropertySheet returned\n"))) ;
    }
    else if((setupHWnd != NULL) && (GetForegroundWindow() != setupHWnd))
    {
        // setup dialog has probably been lost under the windows raise it.
        setForegroundWin(NULL,0);
        SetForegroundWindow(setupHWnd) ;
        SetWindowPos(setupHWnd,HWND_NOTOPMOST,0,0,0,0,
                     SWP_DEFERERASE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_NOMOVE) ;
    }
}

/************************************************
 * Moves a window to a given desk
 */
static int windowSetDesk(HWND theWin, int theDesk, int move)
{
    HWND ownerWin, activeHWnd ;
    unsigned char show ;
    BOOL setActive ;
    int index, ret=0 ;
    
    activeHWnd = GetForegroundWindow() ;
    vwLogBasic((_T("Set window desk: %x %d %d (%x)\n"),(int) theWin,theDesk,move,(int) activeHWnd)) ;
    setActive = FALSE ;
    do {
        ownerWin = 0 ;
        index = nWin ;
        while(--index >= 0)
        {
            if((winList[index].Handle == theWin) || (winList[index].Owner == theWin))
            {
                // Even if the window does not have to be moved, it was found so return true
                ret = 1 ;
                if(winList[index].Desk != theDesk)
                    winList[index].ZOrder[theDesk] = winList[index].ZOrder[winList[index].Desk] ;
                else if(winList[index].Visible == vwVISIBLE_YESTEMP)
                    winList[index].ZOrder[theDesk] = winList[index].ZOrder[currentDesk] ;
                if(((winList[index].Desk != theDesk) || (winList[index].Visible == vwVISIBLE_YESTEMP)) &&
                   !winList[index].Sticky)
                {
                    // set the windows zorder on the new desk to be its zorder on the current desk
                    winList[index].ZOrder[theDesk] = winList[index].ZOrder[winList[index].Desk] ;
                    /* if temporarily copying the window to this desk */ 
                    if(move > 1)
                    {
                        if(winList[index].Desk == currentDesk)
                            show = vwVISIBLE_YES ;
                        else if(theDesk == currentDesk)
                            show = vwVISIBLE_YESTEMP ;
                        else
                            show = vwVISIBLE_NO ;
                    }
                    else
                    {
                        winList[index].Desk = theDesk;
                        if(currentDesk == theDesk)
                            show = vwVISIBLE_YES ;
                        else if(move)
                            show = vwVISIBLE_NO ;
                        else
                            show = vwVISIBLE_YESTEMP ;
                    }
                    if(!show && (activeHWnd == winList[index].Handle))
                        setActive = TRUE ;
                    showHideWindow(&winList[index],0,show) ;
                    if((winList[index].Owner != 0) && (winList[index].Owner != theWin))
                        ownerWin = winList[index].Owner ;
                }
            }
        }
    } while((theWin = ownerWin) != 0) ;
    if(setActive)
    {
        // we have just assigned the foreground window to a different
        // desktop, we must find a replacement as leaving this one as the
        // active foreground window can lead to problems 
        unsigned long activeZOrder=0 ;
        activeHWnd = NULL ;
        index = nWin ;
        while(--index >= 0)
        {
            if((winList[index].Desk == currentDesk) &&
               ((winList[index].Style & (WS_MINIMIZE|WS_VISIBLE)) == WS_VISIBLE) && 
               (winList[index].ZOrder[currentDesk] > activeZOrder))
            {
                activeHWnd = winList[index].Handle;
                activeZOrder = winList[index].ZOrder[currentDesk];
            }
        }
        vwLogVerbose((_T("Looking for replacement active: %x\n"),(int) activeHWnd)) ;
        setForegroundWin(activeHWnd,0) ;
    }
    return ret ;
}

/************************************************
 * Sets a windows sticky setting
 */
static int windowSetSticky(HWND theWin, int state)
{
    unsigned long zorder ;
    HWND ownerWin ;
    int index, ii, ret=0 ;
    
    do {
        ownerWin = 0 ;
        index = nWin ;
        while(--index >= 0)
        {
            if((winList[index].Handle == theWin) || (winList[index].Owner == theWin))
            {
                if(state < 0) // toggle sticky state - set state so all owner windows are set correctly.
                    state = winList[index].Sticky ^ TRUE;
                vwLogVerbose((_T("Setting Sticky: %x %x - %d -> %d\n"),(int) winList[index].Handle,
                            (int) theWin,(int) winList[index].Sticky,state)) ;
                winList[index].Sticky = state ;
                if(winList[index].Sticky)
                {
                    // set its zorder of all desks to its zorder on its current desk
                    zorder = winList[index].ZOrder[winList[index].Desk] ;
                    ii = MAXDESK - 1 ;
                    do
                        winList[index].ZOrder[ii] = zorder ;
                    while(--ii >= 0) ;
                    // if not visible (i.e. was on another desktop) then make it visible
                    if(!winList[index].Visible)
                        showHideWindow(&winList[index],0,vwVISIBLE_YES) ;
                }
                winList[index].Desk = currentDesk;
                if((winList[index].Owner != 0) && (winList[index].Owner != theWin))
                    ownerWin = winList[index].Owner ;
                ret = 1 ;
            }
        }
    } while((theWin = ownerWin) != 0) ;
    return ret ;
}


/*************************************************
 * Searches for windows in user list
 */
static void findUserWindows(void) 
{
    HWND tmpHnd;
    int i;
    
    for(i = 0; i < curUser; ++i)
    {
        if(userList[i].type & 1)
            tmpHnd = FindWindow(NULL,userList[i].match);
        else
            tmpHnd = FindWindow(userList[i].match, NULL);
        
        if((tmpHnd != NULL) && (winListFind(tmpHnd) < 0))
        {
            winList[nWin].Handle = tmpHnd;
            winList[nWin].Style = GetWindowLong(tmpHnd, GWL_STYLE);
            winList[nWin].ExStyle = GetWindowLong(tmpHnd, GWL_EXSTYLE);
            nWin++;
        }
    }
}


/*************************************************
 * Callback function. Integrates all enumerated windows
 */
static BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam) 
{
    int idx, style, exstyle ;
    HWND phwnd, gphwnd ;
    RECT pos ;
    
    if((style = GetWindowLong(hwnd, GWL_STYLE)) & WS_CHILD)                // No child windows
            return TRUE;
    if((idx=winListFind(hwnd)) < 0)
    {
        exstyle = GetWindowLong(hwnd, GWL_EXSTYLE) ;
        /* Criterias for a window to be handeled by VirtuaWin
         * Note: some apps like winamp use the WS_EX_TOOLWINDOW flag to remove themselves from
         * the taskbar so VW will manage toolwin windows if they are not popups or have owners
         */
        if(!(style & WS_VISIBLE) ||                                        // Must be visible
           ((exstyle & WS_EX_TOOLWINDOW) &&                                // No toolwindows
            ((style & WS_POPUP) || (GetWindow(hwnd,GW_OWNER) != NULL))) ||
           (hwnd == setupHWnd))                                            // Dont manage VW setup
            // Ignore this window
            return TRUE;
           
        // Only toplevel or owned by desktop
        if(((phwnd=GetParent(hwnd)) != NULL) && (phwnd != desktopHWnd))
        {
            if((idx=winListFind(phwnd)) >= 0)
            {
                // The method used to hide tricky windows requires us to manage sub dialogs as well
                if(!winList[idx].Tricky)
                    return TRUE;
            }                
            else if((GetWindowLong(phwnd, GWL_STYLE) & WS_VISIBLE) || 
                    (((gphwnd=GetParent(phwnd)) != NULL) && (gphwnd != desktopHWnd)))
                // Some apps like Word attach sub dialogs to a hidden parent window rather
                // than to the main window and as the window is hidden it won't be managed
                // so the sub-dialog will not be hidden. 
                return TRUE;
        }
        if(nWin >= MAXWIN)
        {
            static BOOL printedError=FALSE ;
            if(!printedError)
            {
                printedError = TRUE ;
                MessageBox(hWnd,_T("Maximum number of managed windows has been reached,\nnew windows will not be managed.\n\nPlease report this to problem."),vwVIRTUAWIN_NAME _T(" Error"), MB_ICONERROR);
            }
        }
        else
        {
            idx = nWin++;
            winList[idx].Handle = hwnd;
            winList[idx].Style = style;
            winList[idx].ExStyle = exstyle;
        }
        return TRUE;
    }
    winList[idx].State = 1 ;
    if(winList[idx].Visible)
    {
        if(!(style & WS_VISIBLE))
        {
            /* this window has been hidden by some else - stop handling it
             * unless VirtualWin knows its not visible (which means it is
             * probably a hung process so keep it.) */
            winList[idx].State = 0 ;
            return TRUE ;
        }
        /* if visible store the latest style flags */
        winList[idx].Style = style;
        winList[idx].ExStyle = GetWindowLong(hwnd, GWL_EXSTYLE) ;
    }
    else if(winList[idx].Tricky)
    {
        GetWindowRect(hwnd,&pos) ;
        if(pos.top >= -5000)
        {
            /* Something has moved this window back into a visible area (or
             * at least outside VirtuaWins domain) so make it belong to this
             * desktop, update the list entry, also check the location as the
             * app may have only made the window visible */
            vwLogBasic((_T("Got tricky window state change: %x %d (%d) %d -> %d %d\n"),
                            (int) winList[idx].Handle,winList[idx].Desk,currentDesk,
                            winList[idx].Visible,(int)pos.left,(int)pos.top)) ;
            winList[idx].State = 2 ;
        }
    }
    else if(style & WS_VISIBLE)
    {
        /* Something has made this window visible so make it belong to this desktop, update the list entry,
         * also check the location as the app may have only made the window visible */
        vwLogBasic((_T("Got window state change: %x %d (%d) %d -> %d\n"),
                    (int) winList[idx].Handle,winList[idx].Desk,currentDesk,
                    winList[idx].Visible,IsWindowVisible(winList[idx].Handle))) ;
        winList[idx].State = 2 ;
    }
    return TRUE;
}


static int winListUpdate(void)
{
    DWORD iprocId, jprocId ;
    HWND activeHWnd ;
    RECT pos ;
    TCHAR cname[vwCLASSNAME_MAX], wname[vwWINDOWNAME_MAX] ;
    int inWin, newDesk=0, i, j, hungCount=0 ;
    
    vwLogVerbose((_T("Updating winList, nWin %d, fgw %x tpw %x\n"),nWin,
                (int) GetForegroundWindow(),(int) GetTopWindow(NULL))) ;
    /* We now own the mutex. */
    i = inWin = nWin ;
    while(--i >= 0)
        winList[i].State = 0 ;

    // Get all windows
    EnumWindows(enumWindowsProc,0);
    if(curUser)
        findUserWindows();
    
    // finish the initialisation of new windows
    for(i=inWin ; i < nWin ; ++i)
    {
        // Store the owner if there is one, but only if the owner is visible
        GetClassName(winList[i].Handle,cname,vwCLASSNAME_MAX);
        if(!GetWindowText(winList[i].Handle,wname,vwWINDOWNAME_MAX))
            _tcscpy(wname,_T("<None>"));
        GetWindowRect(winList[i].Handle,&pos) ;
        winList[i].Tricky = checkIfTricky(cname,wname,&pos) ;
        winList[i].Desk = currentDesk;
        winList[i].ZOrder[currentDesk] = 1;
        winList[i].Visible = TRUE;
        winList[i].State = 1 ;
        winList[i].menuId = 0 ;
        if(((winList[i].Owner = GetWindow(winList[i].Handle,GW_OWNER)) != NULL) && !IsWindowVisible(winList[i].Owner))
            winList[i].Owner = NULL ;
        // isn't part of an existing app thats opened a new window
        if((winList[i].Sticky=checkIfSticky(cname,wname)))
            windowSetSticky(winList[i].Handle,TRUE) ;
        else if(useDeskAssignment &&
                ((j = checkIfAssigned(cname,wname)) != currentDesk))
        {
            windowSetDesk(winList[i].Handle,j,assignImmediately) ;
            if(assignImmediately && (hiddenWindowAct == 3) && (newDesk == 0))
                newDesk = j ;
        }
    }
    // now finish of initialization of owned windows
    for(i=inWin ; i < nWin ; ++i)
    {
        if((winList[i].Tricky & vwTRICKY_POSITION) &&
           (GetWindowThreadProcessId(winList[i].Handle,&iprocId) != 0))
        {
            /* some apps like Excel and Adobe Reader create one main window
             * and a hidden tricky position windows per file open for the
             * task bar, if this is one make it owned by the main window */
            j = nWin ;
            while(--j >= 0)
            {
                if((j != i) && ((winList[j].Tricky & vwTRICKY_POSITION) == 0) &&
                   (GetWindowThreadProcessId(winList[j].Handle,&jprocId) != 0) &&
                   (jprocId == iprocId))
                {
                    vwLogBasic((_T("Making tricky window %x owned by %x\n"),
                                (int) winList[i].Handle,(int) winList[j].Handle)) ;
                    winList[i].Owner = winList[j].Handle ;
                    break ;
                }
            }
        }
        if(winList[i].Owner != NULL)
        {
            j = nWin ;
            while(--j >= 0)
                if(winList[j].State && (winList[j].Handle == winList[i].Owner))
                {
                    if(winList[i].Handle == winList[j].Owner)
                        /* circular owner loop, make i the parent */
                        winList[i].Owner = NULL ;
                    else
                    {
                        // an existing app has either unhidden an old window or popped up a new one 
                        // use the existing window's settings for this one
                        if(winList[j].Owner != NULL)
                            winList[i].Owner = winList[j].Owner ;
                        if(!winList[i].Sticky)
                            winList[i].Sticky = winList[j].Sticky ;
                        if((winList[i].Desk=winList[j].Desk) != currentDesk)
                            winList[i].State = 2 ;
                        if((winList[i].Tricky & vwTRICKY_WINDOW) && winList[j].Visible)
                        {
                            // if an owned window is flagged as tricky we must make the parent window
                            // tricky otherwise the call to ShowOwnedPopups is likely to break things
                            winList[j].Tricky |= vwTRICKY_WINDOW ;
                            vwLogBasic((_T("Making parent window %x tricky\n"),(int) winList[j].Handle)) ;
                        }
                    }
                    break ;
                }
        }
    }
    if(vwLogEnabled())
    {
        for(i=inWin ; i < nWin ; ++i)
        {
            GetWindowRect(winList[i].Handle,&pos) ;
            GetClassName(winList[i].Handle,cname,vwCLASSNAME_MAX);
            if(!GetWindowText(winList[i].Handle,wname,vwWINDOWNAME_MAX))
                _tcscpy(wname,_T("<None>"));
            vwLogBasic((_T("Got new window %8x %08x %08x Desk %d Stk %d Trk %d Vis %d Own %x Pos %d %d\n  Class \"%s\" Title \"%s\"\n"),
                        (int) winList[i].Handle,(int)winList[i].Style,(int)winList[i].ExStyle,
                        (int)winList[i].Desk,(int)winList[i].Sticky,(int)winList[i].Tricky,
                        winList[i].Visible,(int) winList[i].Owner,(int) pos.left,(int) pos.top,cname,wname)) ;
        }
    }
          
    // remove windows that have gone.
    // Note that when a window is closed it takes a while for the window to
    // disappear, windows will then find another app to make current and the
    // order of events is fairly random. The problem for us is that if
    // windows selects a hidden app (i.e. on another desktop) it is very
    // difficult to differentiate between this and a genuine pop-up event.
    activeHWnd = GetForegroundWindow() ;
    if(lastLostHWnd != NULL)
    {
        if(activeHWnd == lastLostHWnd)
            activeHWnd = NULL ;
        else
            lastLostHWnd = NULL ;
    }
    for(i=0, j=0 ; i < nWin; ++i)
    {
        if(winList[i].State)
        {
            if(i != j)
                winList[j] = winList[i] ;
            j++ ;
        }
        else
        {
            vwLogBasic((_T("Lost window %8x %d %d %d %d %x\n"),(int) winList[i].Handle,
                        winList[i].Desk,winList[i].Sticky,winList[i].Tricky,
                        winList[i].Visible,(int) winList[i].Owner)) ;
            if(winList[i].Handle == lastFGHWnd)
            {
                if(activeHWnd == lastFGHWnd)
                {
                    lastLostHWnd = activeHWnd ;
                    activeHWnd = NULL ;
                }
                lastBOFGHWnd = lastFGHWnd ;
                lastBOFGStyle = lastFGStyle ;
                lastFGHWnd = NULL ;
            }
        }
    }
    nWin = j ;
    
    // Handle the re-assignment of any popped up window, set the zorder and count hung windows
    vwLogVerbose((_T("Active %8x Last %8x %x LBO %8x %x\n"),(int) activeHWnd,
                    (int) lastFGHWnd, lastFGStyle, (int) lastBOFGHWnd, lastBOFGStyle)) ;
    i = nWin ;
    while(--i >= 0)
    {
        if(winList[i].State == 2)
        {
            if((winList[i].Desk != currentDesk) && ((hiddenWindowAct == 0) || (winList[i].Desk >= vwDESK_PRIVATE1)))
            {
                j = winList[i].Desk ;
                windowSetDesk(winList[i].Handle,currentDesk,2) ;
                windowSetDesk(winList[i].Handle,j,1) ;
                if(winList[i].Handle == activeHWnd)
                    activeHWnd = NULL ;
            }
            else
            {
                if((hiddenWindowAct == 3) && (newDesk == 0))
                    newDesk = winList[i].Desk ;
                windowSetDesk(winList[i].Handle,currentDesk,hiddenWindowAct) ;
            }
        }
        if(winList[i].Handle == activeHWnd)
        {
            // one problem with handling tricky windows is if the user closes the last window on a desktop
            // windows will select a Tricky hidden window as the replacement - try to spot this
            if(activeHWnd != lastFGHWnd)
            {
                // one problem with handling tricky windows is if the user
                // closes or minimises the last window on a desktop windows
                // will select a Tricky hidden window as the replacement -
                // try to spot & handle this - a bit messy... 
                if(!winList[i].Visible && winList[i].Tricky &&
                   (((lastBOFGHWnd != NULL) && (lastFGHWnd == NULL)) ||
                    ((lastFGHWnd != NULL) && 
                     (!IsWindow(lastFGHWnd) ||
                      ((((lastFGStyle & WS_MINIMIZE) == 0) || (lastBOFGHWnd != lastFGHWnd) || ((lastBOFGStyle & WS_MINIMIZE) == 0)) &&
                       (GetWindowLong(lastFGHWnd,GWL_STYLE) & WS_MINIMIZE))))))
                    // not a popup, windows selected replacement - ingore
                    setForegroundWin(NULL,0) ;
                else if((winList[i].Desk != currentDesk) && ((hiddenWindowAct == 0) || (winList[i].Desk >= vwDESK_PRIVATE1)))
                    setForegroundWin(NULL,0) ;
                else
                {
                    if(!winList[i].Visible && hiddenWindowAct)
                    {
                        vwLogBasic((_T("Got Popup - Active %8x Last %8x %x LBO %8x %x\n"),(int) activeHWnd,
                                        (int) lastFGHWnd, lastFGStyle, (int) lastBOFGHWnd, lastBOFGStyle)) ;
                        if((hiddenWindowAct == 3) && (newDesk == 0))
                            newDesk = winList[i].Desk ;
                        windowSetDesk(winList[i].Handle,currentDesk,hiddenWindowAct) ;
                    }
                    winList[i].ZOrder[currentDesk] = ++vwZOrder ;
                    // if this is only a temporary display increase its zorder in its main desk
                    if(winList[i].Desk != currentDesk)
                        winList[i].ZOrder[winList[i].Desk] = vwZOrder ;
                }
            }
        }
        if(!winList[i].Visible)
        {
            if(winList[i].Sticky || (winList[i].Desk == currentDesk))
                hungCount++ ;
        }
        else if(((winList[i].Visible & vwVISIBLE_TEMP) == 0) &&
                !winList[i].Sticky && (winList[i].Desk != currentDesk))
            hungCount++ ;
    }
    if(activeHWnd != NULL)
    {
        lastBOFGHWnd = lastFGHWnd ;
        lastBOFGStyle = lastFGStyle ;
        if((lastFGHWnd = activeHWnd) == hWnd)
            lastFGHWnd = NULL ;
        else if(lastFGHWnd != NULL)
            lastFGStyle = GetWindowLong(activeHWnd,GWL_STYLE) ;
    }
    vwLogVerbose((_T("Updated winList, %d windows - %d hung, newDesk %d\n"),nWin,hungCount,newDesk)) ;
    return ((newDesk << 16) | hungCount) ;
}

/*************************************************
 * Makes all windows visible
 */
static void showAll(int shwFlags)
{
    int x ;
    lockMutex();
    winListUpdate() ;
    for (x = 0; x < nWin; ++x) 
    {
        // still ignore windows on a private desktop unless exiting (vwSHWIN_TRYHARD)
        if((winList[x].Desk < vwDESK_PRIVATE1) || (shwFlags & vwSHWIN_TRYHARD))
        {
            winList[x].Desk = currentDesk ;
            showHideWindow(&winList[x],shwFlags,vwVISIBLE_YES);
        }
    }
    releaseMutex();
}

/************************************************
 * Does necessary stuff before shutting down
 */
static void shutDown(void)
{
    KillTimer(hWnd, 0x29a);                // Remove the timer
    if(saveLayoutOnExit || saveSticky)
    {
        lockMutex();
        winListUpdate() ;
        if(saveLayoutOnExit)
            saveAssignedList(nWin,winList); 
        if(saveSticky)
            saveStickyWindows(nWin,winList);
        releaseMutex();
    }
    unloadModules();
    unRegisterAllKeys();
    showAll(0);                            // gather all windows quickly
    Shell_NotifyIcon(NIM_DELETE, &nIconD); // This removes the icon
    showAll(vwSHWIN_TRYHARD);              // try harder to gather remaining ones before exiting
    DeleteObject(checkIcon);               // Delete loaded icon resource
    PostQuitMessage(0);
}

/************************************************
 * Callback for new window assignment and taskbar fix (removes bogus reappearances of tasks on win9x).
 */
static VOID CALLBACK monitorTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    static unsigned long mtCount=0 ;
    int index, hungCount ;
    
    lockMutex();
    timerCounter++ ;
    hungCount = winListUpdate() ;
    if((index = (hungCount >> 16)) > 0)
    {
        releaseMutex();
        changeDesk(index,MOD_CHANGEDESK);
        return ;
    }
    if(hungCount > 0)
    {
        /* there's a hung process, try to handle it - every second to start
         * with, after 32 seconds only try once every 8 seconds */
        mtCount++ ;
        index = (mtCount & ~0x7f) ? 31:3;
        if((mtCount & index) == 0)
        {
            if(displayTaskbarIcon)
            {
                /* flash the icon for half a second each time we try */
                _stprintf(nIconD.szTip,_T("%d window%s not responding"),hungCount,(hungCount==1) ? _T(""):_T("s")) ;
                nIconD.hIcon = NULL; // No icon
                Shell_NotifyIcon(NIM_MODIFY, &nIconD);
            }
            index = nWin ;
            while(--index >= 0)
            {
                if(!winList[index].Visible)
                {
                   if(winList[index].Sticky || (winList[index].Desk == currentDesk))
                       showHideWindow(&winList[index],0,vwVISIBLE_YES) ;
                }
                else if(!(winList[index].Visible & vwVISIBLE_TEMP))
                {
                   if(!winList[index].Sticky && (winList[index].Desk != currentDesk))
                       showHideWindow(&winList[index],0,vwVISIBLE_NO) ;
                }
            }
        }
        else if((((mtCount-1) & index) == 0) && displayTaskbarIcon)
        {
            /* restore the icon */
            setIcon(currentDesk);
        }
    }
    else if(mtCount)
    {
        /* hung process problem has been resolved. */
        mtCount = 0 ;
        _tcscpy(nIconD.szTip,vwVIRTUAWIN_NAME_VERSION);	// Restore Tooltip
        setIcon(currentDesk);
    }

    if(taskbarFixRequired)
    {
        index = nWin ;
        while(--index >= 0)
            if(winList[index].Tricky && !winList[index].Visible)
                PostMessage(taskHWnd, RM_Shellhook, HSHELL_WINDOWDESTROYED, (LPARAM) winList[index].Handle);
    }
    releaseMutex();
}

/*************************************************
 * Helper function for all the step* functions below
 * Does the actual switching work 
 */
static int changeDesk(int newDesk, WPARAM msgWParam)
{
    HWND activeHWnd, zoh ;
    unsigned long activeZOrder=0, zox, zoy, zob ;
    int notHung, activeRefocus=0, x, y, b ;
    
    if(newDesk == currentDesk)
        // Nothing to do
        return 0;
    
    /* don't bother generating an image unless the user has been on the
     * desk for at least a second */
    if((deskImageCount > 0) && (timerCounter >= 4))
        createDeskImage(currentDesk,0) ;
    
    vwLogBasic((_T("Step Desk Start: %d -> %d\n"),currentDesk,newDesk)) ;
    
    timerCounter = 0 ;
    lockMutex();
    winListUpdate() ;
    
    if(isDragging || (checkMouseState(1) == 1))
    {
        /* move the app we are dragging to the new desktop, must handle owner windows */
        HWND ownerWnd;
        
        activeHWnd = GetForegroundWindow() ;
        do {
            ownerWnd = 0 ;
            x = nWin ;
            while(--x >= 0)
            {
                if((winList[x].Handle == activeHWnd) || (winList[x].Owner == activeHWnd))
                {
                    winList[x].Desk = newDesk;
                    winList[x].ZOrder[newDesk] = winList[x].ZOrder[currentDesk] ;
                    if((winList[x].Owner != 0) && (winList[x].Owner != activeHWnd))
                        ownerWnd = winList[x].Owner ;
                }
            }
        } while((activeHWnd = ownerWnd) != 0) ;
    }
    if(setupOpen)
        storeDeskHotkey() ;
    
    currentDesk = newDesk;
    /* Calculations for getting the x and y positions */
    currentDeskY = ((currentDesk - 1)/nDesksX) + 1 ;
    currentDeskX = nDesksX + currentDesk - (currentDeskY * nDesksX);
    
    activeHWnd = NULL;
    if(preserveZOrder == 1)
    {
        for (x = 0; x < nWin ; ++x)
        {
            if(winList[x].Sticky || (!minSwitch && (winList[x].Style & WS_MINIMIZE)))
                winList[x].Desk = currentDesk ;
            if((winList[x].Desk != currentDesk) && winList[x].Visible)
                // Hide these, not iconic or "Switch minimized" enabled
                showHideWindow(&winList[x],0,vwVISIBLE_NO) ;
        }
        y = nWin ;
        zoy = 0xffffffff ;
        zoh = HWND_NOTOPMOST ;
        for(;;)
        {
            b = -1 ;
            zob = 0 ;
            for(x = 0; x < nWin ; ++x)
            {
                if((winList[x].Desk == currentDesk) && ((winList[x].ExStyle & WS_EX_TOPMOST) == 0) &&
                   ((zox=winList[x].ZOrder[currentDesk]) >= zob) && ((zox < zoy) || ((zox == zoy) && (x < y))))
                {
                    zob = zox ;
                    b = x ;
                }
            }
            if(b < 0)
                break ;
            // Show these windows
            if(!winList[b].Visible)
                notHung = showHideWindow(&winList[b],0,vwVISIBLE_YES) ;
            else
                notHung = windowIsNotHung(winList[b].Handle,100) ;
            vwLogVerbose((_T("ZOrder: %d %d %x -> %d %d %x (%d)\n"),y,zoy,zoh,b,zob,winList[b].Handle,notHung)) ;
            if(notHung)
            {
                SetWindowPos(winList[b].Handle,zoh,0,0,0,0,SWP_DEFERERASE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOSENDCHANGING|SWP_NOMOVE) ;
                zoh = winList[b].Handle ;
            }
            else
                vwLogBasic((_T("ZOrder: %8x - HUNG\n"),(int) winList[b].Handle)) ;
            if((activeHWnd == NULL) && ((winList[b].Style & (WS_MINIMIZE|WS_VISIBLE)) == WS_VISIBLE))
            {
                activeHWnd = winList[b].Handle;
                activeZOrder = winList[b].ZOrder[currentDesk];
            }
            y = b ;
            zoy = zob ;
        }
    }
    else
    {
        y = 0 ;
        for (x = 0; x < nWin ; ++x)
        {
            if(winList[x].Sticky || (!minSwitch && (winList[x].Style & WS_MINIMIZE)))
                winList[x].Desk = currentDesk ;
            if(winList[x].Desk == currentDesk)
            {
                // Show these windows
                if(((winList[x].Style & (WS_MINIMIZE|WS_VISIBLE)) == WS_VISIBLE) && 
                   (winList[x].ZOrder[currentDesk] > activeZOrder))
                {
                    activeHWnd = winList[x].Handle;
                    activeZOrder = winList[x].ZOrder[currentDesk];
                    activeRefocus = 0 ;
                }
                if(!winList[x].Visible)
                {
                    showHideWindow(&winList[x],0,vwVISIBLE_YES) ;
                    y = 1 ;
                }
                else if(y && windowIsNotHung(winList[x].Handle,50))
                {
                    /* It is visible but its position in the taskbar is wrong - fix by removing from the taskbar and then adding again */
                    PostMessage(taskHWnd, RM_Shellhook, HSHELL_WINDOWDESTROYED, (LPARAM) winList[x].Handle);
                    Sleep(1);
                    PostMessage(taskHWnd, RM_Shellhook, HSHELL_WINDOWCREATED, (LPARAM) winList[x].Handle );
                    if(activeHWnd == winList[x].Handle)
                        activeRefocus = 1 ;
                }
            }
            else if(winList[x].Visible)
                // Hide these, not iconic or "Switch minimized" enabled
                showHideWindow(&winList[x],0,vwVISIBLE_NO) ;
        }
        if(preserveZOrder)
        {
            // very small sleep to allow system to catch up
            Sleep(1);
            
            y = nWin ;
            zoy = 0xffffffff ;
            zoh = HWND_NOTOPMOST ;
            for(;;)
            {
                b = -1 ;
                zob = 0 ;
                for(x = 0; x < nWin ; ++x)
                {
                    if((winList[x].Desk == currentDesk) && ((winList[x].ExStyle & WS_EX_TOPMOST) == 0) &&
                       ((zox=winList[x].ZOrder[currentDesk]) >= zob) && ((zox < zoy) || ((zox == zoy) && (x < y))))
                    {
                        zob = zox ;
                        b = x ;
                    }
                }
                if(b < 0)
                    break ;
                // Show these windows
                notHung = windowIsNotHung(winList[b].Handle,100) ;
                vwLogVerbose((_T("TBZOrder: %d %d %x -> %d %d %x (%d)\n"),y,zoy,zoh,b,zob,winList[b].Handle,notHung)) ;
                if(notHung)
                {
                    SetWindowPos(winList[b].Handle,zoh,0,0,0,0,SWP_DEFERERASE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOSENDCHANGING|SWP_NOMOVE) ;
                    zoh = winList[b].Handle ;
                }
                else
                    vwLogBasic((_T("TBZOrder: %8x - HUNG\n"),(int) winList[b].Handle)) ;
                y = b ;
                zoy = zob ;
            }
        }
    }
    if(releaseFocus)     // Release focus maybe?
        activeHWnd = NULL ;
    vwLogBasic((_T("Active found: %x (%d,%d,%d,%d)\n"),(int) activeHWnd,(int)activeZOrder,releaseFocus,isDragging,activeRefocus)) ;
    if(activeRefocus && (GetForegroundWindow() == activeHWnd))
        /* we have had to fix the taskbar order by destrying and recreating,
         * this loses the current focus so we must refocus the taskbar */
        setForegroundWin(NULL,TRUE) ;
    setForegroundWin(activeHWnd,TRUE) ;
    // reset the monitor timer to give the system a chance to catch up first
    SetTimer(hWnd, 0x29a, 250, monitorTimerProc);
    releaseMutex();
    
    if(setupOpen)
    {
        initDeskHotkey() ;
        showSetup() ;
    }
    setIcon(currentDesk) ;
    if(refreshOnWarp) // Refresh the desktop 
        RedrawWindow( NULL, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN );
    
    postModuleMessage(MOD_CHANGEDESK,msgWParam,currentDesk);
    
    vwLogBasic((_T("Step Desk End (%x)\n"),(int)GetForegroundWindow())) ;
    
    return currentDesk ;
}

/*************************************************
 * Goto a specified desktop specifying desk number
 */
int gotoDesk(int theDesk, BOOL force)
{
    if((theDesk >= MAXDESK) || ((theDesk > (nDesksY * nDesksX)) && !force))
        return 0;
    
    return changeDesk(theDesk,MOD_CHANGEDESK);
}

/*************************************************
 * Goto a specified desktop specifying desk number
 */
static int stepDelta(int delta)
{
    int newDesk ;
    if(currentDesk >= vwDESK_PRIVATE1)
        /* on a private desktop - go to first if delta is +ve, last otherwise */
        newDesk = (delta < 0) ? (nDesksX * nDesksY):1 ;
    else if((newDesk=currentDesk+delta) < 1)
    {
        if(!deskWrap)
            return 0 ;
        newDesk = nDesksX * nDesksY ;
    }
    else if(newDesk > (nDesksX * nDesksY))
    {
        if(!deskWrap)
            return 0 ;
        newDesk = 1 ;
    }
    return changeDesk(newDesk,MOD_CHANGEDESK);
}


/*************************************************
 * Step on desk to the right
 */
static int stepRight(void)
{
    int deskX, deskY=currentDeskY ;
    
    if(currentDesk >= vwDESK_PRIVATE1)
    {   /* on a private desktop - go to first */
        deskX = 1;
        deskY = 1;
    }
    else if((deskX=currentDeskX + 1) > nDesksX) {
        if(!deskWrap)
            return 0;
        deskX = 1;
    }
    return changeDesk(calculateDesk(deskX,deskY),MOD_STEPRIGHT);
}

/*************************************************
 * Step one desk to the left
 */
static int stepLeft(void)
{
    int deskX, deskY=currentDeskY ;
    
    if(currentDesk >= vwDESK_PRIVATE1)
    {   /* on a private desktop - go to last */
        deskX = nDesksX;
        deskY = nDesksY;
    }
    else if((deskX=currentDeskX - 1) < 1) {
        if(!deskWrap)
            return 0;
        deskX = nDesksX;
    }
    return changeDesk(calculateDesk(deskX,deskY),MOD_STEPLEFT);
}

/*************************************************
 * Step one desk down
 */
static int stepDown(void)
{
    int deskX=currentDeskX, deskY ;
    
    if(currentDesk >= vwDESK_PRIVATE1)
    {   /* on a private desktop - go to first */
        deskX = 1;
        deskY = 1;
    }
    else if((deskY = currentDeskY + 1) > nDesksY) {
        if(!deskWrap)
            return 0;
        deskY = 1;
    }
    return changeDesk(calculateDesk(deskX,deskY),MOD_STEPDOWN);
}

/*************************************************
 * Step one desk up
 */
static int stepUp(void)
{
    int deskX=currentDeskX, deskY ;
    
    if(currentDesk >= vwDESK_PRIVATE1)
    {   /* on a private desktop - go to last */
        deskX = nDesksX;
        deskY = nDesksY;
    }
    else if((deskY = currentDeskY - 1) < 1) {
        if(!deskWrap)
            return 0;
        deskY = nDesksY;
    }
    return changeDesk(calculateDesk(deskX,deskY),MOD_STEPUP);
}

/*************************************************
 * createSortedWinList_cos creates a popup menu for the window-hotkey
 * which displays all windows in one list vertically seperated by a line.
 * first column is sticky, second is direct access and third is assign.
 * so you don't have to step through submenus.
 * 
 * Author: Christian Storm aka cosmic (Christian.Storm@Informatik.Uni-Oldenburg.de)
 */
#define vwPMENU_TITLES   0x001
#define vwPMENU_COMPACT  0x002
#define vwPMENU_STICKY   0x100
#define vwPMENU_ACCESS   0x200
#define vwPMENU_ASSIGN   0x400
#define vwPMENU_ID_MASK  0x0ff
#define vwPMENU_COL_MASK 0x700

static int
winListCreateItemList(MenuItem **items,int *numitems)
{
    MenuItem *item;
    TCHAR title[48];
    TCHAR buff[MAX_PATH];
    int i,x,y,c,doAssignMenu=0 ;
    
    // create the window list
    lockMutex();
    winListUpdate() ;
    i = 0 ;
    for(c = 0; c < nWin; ++c)
    {
        // ignore owned windows if we are managing the owner and ones on a private desktop
        if(winList[c].Desk >= vwDESK_PRIVATE1)
            x = 0 ;
        else if(winList[c].Owner == NULL)
            x = -1 ;
        else
        {
            x = nWin ;
            while(--x >= 0)
                if(winList[x].Handle == winList[c].Owner)
                    break ;
        }
        if(x < 0)
        {
            HICON hSmallIcon ;
            
            GetWindowText(winList[c].Handle, buff, 30);
#ifdef NDEBUG
            _stprintf(title,_T("%d - %s"),winList[c].Desk,buff);
#else
            _stprintf(title,_T("%d - %s (%x)"),winList[c].Desk,buff,(int) winList[c].Handle);
#endif
            if(((item = malloc(sizeof(MenuItem))) == NULL) ||
               ((item->name = _tcsdup(title)) == NULL))
            {
                while(--i >= 0)
                {
                    free(items[i]->name) ;
                    free(items[i]) ;
                }
                return 0 ;
            }
            items[i++] = item;
            
            if((hSmallIcon = (HICON)GetClassLong(winList[c].Handle, GCL_HICON)) == NULL)
            {
                // Fallback plan, maybe this works better for this type of application
                // Otherwise there is not much we can do (could try looking for an owned window)
                DWORD theIcon;
                SendMessageTimeout(winList[c].Handle, WM_GETICON, ICON_SMALL, 0L, 
                                   SMTO_ABORTIFHUNG | SMTO_BLOCK, 100, &theIcon);
                if(theIcon == 0)
                    // some apps (e.g. Opera) only have big icons
                    SendMessageTimeout(winList[c].Handle, WM_GETICON, ICON_BIG, 0L, 
                                       SMTO_ABORTIFHUNG | SMTO_BLOCK, 100, &theIcon);
                hSmallIcon = (HICON)theIcon;
            }
            item->icon=hSmallIcon;
            if(((item->desk = winList[c].Desk) != currentDesk) && !winList[c].Sticky)
                doAssignMenu = assignMenu ;
            item->sticky = winList[c].Sticky;
            item->id = i ;
            winList[c].menuId = i ;
        }
        else
            winList[c].menuId = 0 ;
    }
    releaseMutex();
    if((i == 0) || ((stickyMenu + directMenu + doAssignMenu) == 0))
    {
        // Either user has no apps, disabled all 3 menus or only enable assign and all are on the current desk
        for (x=0; x<i; x++)
        {
            free(items[x]->name) ;
            free(items[x]) ;
        }
        return 0 ;
    }
    
    // sorting using bubble sort
    for(x = 0; x < i; x++ )
    {
        for(y = 0; y<i; y++)
        {
            if(_tcscmp(items[x]->name, items[y]->name) < 0 )
            {
                item = items [x];
                items[x] = items[y];
                items[y] = item;
            }
        }
    }
    *numitems = i ;
    
    i = ((stickyMenu + directMenu + assignMenu) == 1) ? 0:vwPMENU_TITLES ;
    if(compactMenu)
        i |= vwPMENU_COMPACT ;
    if(stickyMenu)
        i |= vwPMENU_STICKY ;
    if(directMenu)
        i |= vwPMENU_ACCESS ;
    if(doAssignMenu)
        i |= vwPMENU_ASSIGN ;
    return i ;
}

static HMENU
winListCreateMenu(int flags, int itemCount, MenuItem **items)
{
    HMENU hMenu;
    int c, x ;
    
    if((hMenu = CreatePopupMenu()) == NULL)
        return NULL ;
    
    if(flags & vwPMENU_STICKY)
    {
        if(flags & vwPMENU_TITLES)
        {
            AppendMenu(hMenu, (flags & vwPMENU_COMPACT) ? MF_STRING:(MF_STRING | MF_DISABLED),
                       (flags & vwPMENU_COMPACT) ? vwPMENU_STICKY:0,_T("Sticky"));
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
        }
        for(x=0,c=0 ; x < itemCount ; x++)
        {
            if((c != 0) && (c != items[x]->desk))
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
            c = items[x]->desk;
            AppendMenu( hMenu, MF_OWNERDRAW | (items[x]->sticky ? MF_CHECKED: 0),
                        vwPMENU_STICKY | (items[x]->id), (const TCHAR *) items[x] );
        }
    }
    
    if(flags & vwPMENU_ACCESS)
    {
        if(flags & vwPMENU_TITLES)
        {
            AppendMenu(hMenu, (flags & vwPMENU_COMPACT) ? MF_STRING:(flags & vwPMENU_STICKY) ? (MF_STRING | MF_MENUBARBREAK | MF_DISABLED):(MF_STRING | MF_DISABLED),
                       (flags & vwPMENU_COMPACT) ? vwPMENU_ACCESS:0,_T("Access"));
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
        }
        for(x=0,c=0 ; x < itemCount ; x++)
        {
            if((c != 0) && (c != items[x]->desk))
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
            c = items[x]->desk;
            AppendMenu( hMenu, MF_OWNERDRAW, vwPMENU_ACCESS | (items[x]->id), (const TCHAR *) items[x] );
        }
    }
    
    if(flags & vwPMENU_ASSIGN)
    {
        if(flags & vwPMENU_TITLES)
        {
            AppendMenu(hMenu, (flags & vwPMENU_COMPACT) ? MF_STRING:(MF_STRING | MF_MENUBARBREAK | MF_DISABLED),
                       (flags & vwPMENU_COMPACT) ? vwPMENU_ASSIGN:0,_T("Assign"));
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
        }
        for(x=0,c=0 ; x < itemCount ; x++)
        {
            //cannot assign windows already on the current Desktop
            if((items[x]->desk != currentDesk) || (flags & vwPMENU_TITLES))
            {
                if((c != 0) && (c != items[x]->desk))
                    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                c = items[x]->desk;
                if(items[x]->desk != currentDesk)
                    AppendMenu( hMenu, MF_OWNERDRAW, (vwPMENU_ASSIGN | (items[x]->id)), (const TCHAR *) items[x] );
                else
                    /* Make it a separator so cursor movement skips this line */
                    AppendMenu( hMenu, (MF_OWNERDRAW | MF_SEPARATOR), 0, 0) ;
            }
        }
    }
    
    return hMenu ;
}



/************************************************
 * This function decides what switching technique that should be used
 * and calls the appropriate switching function
 */
static BOOL showHideWindow(windowType* aWindow, int shwFlags, unsigned char show)
{
    // Do nothing if we are dragging a window or the show hide state is right
    if((show != 0) ^ (aWindow->Visible != 0))
    {
        unsigned char Tricky;
        RECT pos;
        UINT swpFlags ;
        if(!windowIsNotHung(aWindow->Handle,50) &&
           (Sleep(1),!windowIsNotHung(aWindow->Handle,(shwFlags & vwSHWIN_TRYHARD) ? 5000:100)))
        {
            vwLogBasic((_T("showHideWindow %8x %d %d %x %x %d %d %d - HUNG\n"),(int) aWindow->Handle,shwFlags,show,(int)aWindow->Style,(int)aWindow->ExStyle,aWindow->Visible,aWindow->Desk,currentDesk)) ;
            return FALSE ;
        }
        GetWindowRect( aWindow->Handle, &pos );
        vwLogVerbose((_T("showHideWindow %8x %d %d %x %x %d %d %d - %d %d\n"),(int) aWindow->Handle,shwFlags,show,(int)aWindow->Style,(int)aWindow->ExStyle,aWindow->Visible,aWindow->Desk,currentDesk,(int) pos.left,(int) pos.top)) ;
        Tricky = aWindow->Tricky ;
        if(trickyWindows)
        {
            if((pos.left == -32000) && (pos.top == -32000))
                aWindow->Tricky |= vwTRICKY_POSITION ;
            else
                aWindow->Tricky &= ~vwTRICKY_POSITION ;
            // must still make the window visible in the same way it was hidden
            if(!show)
                Tricky = aWindow->Tricky ;
        }
        if(!Tricky)
        {
            SetWindowPos(aWindow->Handle,0,0,0,0,0,(show) ? 
                         (SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE) : 
                         (SWP_HIDEWINDOW | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE) ) ;
            /* if minimized dont show popups */
            if(!(aWindow->Style & WS_MINIMIZE))
                ShowOwnedPopups(aWindow->Handle,(show) ? TRUE:FALSE) ;
        }
        else
        {
            // show/hide the window in the toolbar and move off screen
            swpFlags = SWP_FRAMECHANGED | SWP_DEFERERASE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_NOMOVE ;
            if(show)
            {
                // Restore the window mode
                SetWindowLong( aWindow->Handle, GWL_EXSTYLE, aWindow->ExStyle );  
                // Notify taskbar of the change
                PostMessage(taskHWnd, RM_Shellhook, HSHELL_WINDOWCREATED, (LPARAM) aWindow->Handle );
                if((pos.top < -5000) && (pos.top > -30000))
                {   // Move the window back onto visible area
                    pos.top += 25000 ;
                    swpFlags ^= SWP_NOMOVE ;
                }
            }
            else
            {
                // This removes window from taskbar and alt+tab list
                SetWindowLong( aWindow->Handle, GWL_EXSTYLE, 
                               (aWindow->ExStyle & (~WS_EX_APPWINDOW)) | WS_EX_TOOLWINDOW);
                // Notify taskbar of the change
                PostMessage(taskHWnd, RM_Shellhook, HSHELL_WINDOWDESTROYED, (LPARAM) aWindow->Handle);
                if((pos.top > -5000) && (pos.top < 20000))
                {   // Move the window off visible area
                    pos.top -= 25000 ;
                    swpFlags ^= SWP_NOMOVE ;
                }
            }
            SetWindowPos(aWindow->Handle, 0, pos.left, pos.top, 0, 0, swpFlags) ; 
        }
    }
    aWindow->Visible = show;
    return TRUE ;
}


/************************************************
 * Toggles the disabled state of VirtuaWin
 */
static void disableAll(HWND aHWnd)
{
    if(enabled)
    {   // disable VirtuaWin
        _tcscpy(nIconD.szTip,vwVIRTUAWIN_NAME _T(" - Disabled")); //Tooltip
        setIcon(0);
        KillTimer(hWnd, 0x29a);
        enableMouse(FALSE);
        unRegisterAllKeys();
        enabled = FALSE;
    }
    else
    {   // Enable VirtuaWin
        registerAllKeys();
        enableMouse(mouseEnable);
        SetTimer(hWnd, 0x29a, 250, monitorTimerProc);
        _tcscpy(nIconD.szTip,vwVIRTUAWIN_NAME_VERSION);	// Tooltip
        setIcon(currentDesk);
        enabled = TRUE;
    }
}

/************************************************
 * Assigns a window to the specified desktop
 * Used by the module message VW_ASSIGNWIN
 */
int assignWindow(HWND theWin, int theDesk, BOOL force)
{
    int ret, change, idx, nDesks ;
    unsigned char sticky=0 ;
    
    vwLogBasic((_T("Assign window: %x %d %d\n"),(int) theWin,theDesk,force)) ;
    
    if(((theWin == NULL) && ((theWin = GetForegroundWindow()) == NULL)) || (theWin == hWnd))
        return 0 ;
    
    change = (theDesk < 0) ;
    if(change)
        theDesk = 0 - theDesk ;
    nDesks = nDesksY * nDesksX ;
    switch(theDesk)
    {
    case VW_STEPPREV:
        if((currentDesk > nDesks) ||
           ((theDesk = currentDesk-1) <= 0))
            theDesk = nDesks ;
        break;
    case VW_STEPNEXT:
        if((currentDesk > nDesks) ||
           ((theDesk = currentDesk+1) > nDesks))
            theDesk = 1 ;
        break;
    case VW_STEPLEFT:
        if(currentDesk > nDesks)
            theDesk = nDesks ;
        else if(currentDeskX <= 1)
            theDesk = calculateDesk(nDesksX,currentDeskY) ;
        else
            theDesk = calculateDesk(currentDeskX-1,currentDeskY) ;
        break;
    case VW_STEPRIGHT:
        if(currentDesk > nDesks)
            theDesk = 1 ;
        else if(currentDeskX >= nDesksX)
            theDesk = calculateDesk(1,currentDeskY) ;
        else
            theDesk = calculateDesk(currentDeskX+1,currentDeskY) ;
        break;
    case VW_STEPUP:
        if(currentDesk > nDesks)
            theDesk = nDesks ;
        else if(currentDeskY <= 1)
            theDesk = calculateDesk(currentDeskX,nDesksY) ;
        else
            theDesk = calculateDesk(currentDeskX,currentDeskY-1) ;
        break;
    case VW_STEPDOWN:
        if(currentDesk > nDesks)
            theDesk = 1 ;
        else if(currentDeskY >= nDesksY)
            theDesk = calculateDesk(currentDeskX,1) ;
        else
            theDesk = calculateDesk(currentDeskX,currentDeskY+1) ;
        break;
    }
    if(((theDesk > nDesks) || (theDesk < 1)) && !force)
        return 0 ; // Invalid desk
    
    lockMutex();
    winListUpdate() ;
    idx = winListFind(theWin) ;
    ret = (idx >= 0) ;
    if(ret)
        sticky = winList[idx].Sticky ;
    releaseMutex();
    if(ret)
    {
        if(change)
        {
            windowSetSticky(theWin,1) ;
            ret = (changeDesk(theDesk,MOD_CHANGEDESK) > 0) ;
            windowSetSticky(theWin,sticky) ;
        }
        else
            ret = windowSetDesk(theWin,theDesk,1) ;
    }
    return ret ;
}

/************************************************
 * Access a window where method:
 *   0 = use config setting, 1 = copy, 2 = move, 3 = change desk
 * Used by the module message VW_ACCESSWIN
 */
static int accessWindow(HWND theWin, int method, BOOL force)
{
    int ret, idx ;
    
    vwLogBasic((_T("Access window: %x %d\n"),(int) theWin,method)) ;
    
    if((theWin == NULL) || (theWin == hWnd))
        return 0 ;
    if((method == 0) && ((method=hiddenWindowAct) == 0))
        method = 1 ;
        
    lockMutex();
    winListUpdate() ;
    idx = winListFind(theWin) ;
    ret = (idx >= 0) ;
    releaseMutex();
    
    if(ret)
    {
        if((winList[idx].Desk >= vwDESK_PRIVATE1) && !force)
            ret = 0 ;
        else if(method != 3)
            ret = windowSetDesk(winList[idx].Handle,currentDesk,method) ;
        else if((ret = (changeDesk(winList[idx].Desk,MOD_CHANGEDESK) > 0)) &&
                ((idx = winListFind(theWin)) > 0) && winList[idx].Visible)
            setForegroundWin(theWin,0);
    }
    return ret ;
}

/************************************************
 * Changes sticky state of a window
 * Used by the module message VW_SETSTICKY
 */
static int setSticky(HWND theWin, int state)
{
    int ret ;
    vwLogBasic((_T("Set sticky window: %x %d\n"),(int) theWin,state)) ;
    
    if((theWin == NULL) && ((theWin = GetForegroundWindow()) == NULL))
        return 0 ;
    
    lockMutex();
    winListUpdate() ;
    ret = windowSetSticky(theWin,state) ;
    releaseMutex();
    return ret ;
}

/************************************************
 * Dismisses the current window by either moving it
 * back to its assigned desk or minimizing.
 */
static int windowDismiss(HWND theWin)
{
    unsigned long activeZOrder ;
    int ret, idx ;
    HWND hwnd ;
    
    vwLogBasic((_T("Dismissing window: %x\n"),(int) theWin)) ;
    if(theWin == NULL)
        return 0 ;
    
    while((GetWindowLong(theWin, GWL_STYLE) & WS_CHILD) && 
          ((hwnd = GetParent(theWin)) != NULL) && (hwnd != desktopHWnd))
        theWin = hwnd ;

    lockMutex();
    winListUpdate();
    if((idx = winListFind(theWin)) < 0)
        ret = 0 ;
    else if(winList[idx].Visible == vwVISIBLE_YESTEMP)
        ret = windowSetDesk(theWin,winList[idx].Desk,1) ;
    else
    {
        ret = CloseWindow(theWin) ;
        /* need to change the focus to another window */
        hwnd = NULL;
        activeZOrder = 0;
        for (idx = 0; idx < nWin ; ++idx)
        {
            if((winList[idx].Desk == currentDesk) &&
               (winList[idx].Handle != theWin) && (winList[idx].Owner != theWin) && 
               ((winList[idx].Style & (WS_MINIMIZE|WS_VISIBLE)) == WS_VISIBLE) && 
               (winList[idx].ZOrder[currentDesk] > activeZOrder))
            {
                hwnd = winList[idx].Handle;
                activeZOrder = winList[idx].ZOrder[currentDesk];
            }
        }
        setForegroundWin(hwnd,TRUE) ;
    }
    releaseMutex();
    return ret ;
}

static BOOL CALLBACK WindowInfoDialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            TCHAR buff[vwCLASSNAME_MAX + vwWINDOWNAME_MAX + 256], *ss ;
            HWND theWin ;
            RECT pos ;
            int idx, tabstops[2] ;
            
            theWin = GetParent(hwndDlg) ;
            if(theWin == NULL)
            {
                SetDlgItemText(hwndDlg,IDC_WID_INFO,_T("Error: failed to get window handle")) ;
                return TRUE;
            }
            tabstops[0] = 10 ;
            tabstops[1] = 52 ;
            SendDlgItemMessage(hwndDlg,IDC_WID_INFO,EM_SETTABSTOPS,(WPARAM)2,(LPARAM)tabstops);
            lockMutex();
            winListUpdate() ;
            idx = winListFind(theWin) ;
            ss = buff ;
            
            _tcscpy(ss,_T("Class Name:\t")) ;
            ss += _tcslen(ss) ;
            GetClassName(theWin,ss,vwCLASSNAME_MAX);
            ss += _tcslen(ss) ;
            _tcscpy(ss,_T("\r\nWindow Name:\t")) ;
            ss += _tcslen(ss) ;
            if(!GetWindowText(theWin,ss,vwWINDOWNAME_MAX))
                _tcscpy(ss,_T("<None>"));
            ss += _tcslen(ss) ;
            GetWindowRect(theWin,&pos) ;
            ss += _stprintf(ss,_T("\r\n\tHandle:\t%x\r\n\tParent:\t%x\r\n\tOwner:\t%x\r\n\tStyles:\t%08x %08x\r\n\tPosition:\t%d %d %d %d\r\n\r\nThis window is "),
                            (int)theWin,(int)GetParent(theWin),(int)GetWindow(theWin,GW_OWNER),
                            (int)GetWindowLong(theWin,GWL_STYLE),(int)GetWindowLong(theWin,GWL_EXSTYLE),
                            (int)pos.top,(int)pos.bottom,(int)pos.left,(int)pos.right) ;
            
            if(idx >= 0)
                _stprintf(ss,_T("being managed\r\n\tOwner:\t%x\r\n\tStyles:\t%08x %08x\r\n\tDesk:\t%d\r\n\tSticky:\t%d\r\n\tTricky:\t%d\r\n\tVisible:\t%d\r\n"),
                          (int)winList[idx].Owner,(int)winList[idx].Style,(int)winList[idx].ExStyle,
                          (int)winList[idx].Desk,(int)winList[idx].Sticky,(int)winList[idx].Tricky,
                          (int)winList[idx].Visible) ;
            else
                _tcscpy(ss,_T("not managed\r\n")) ;
            releaseMutex();
            SetDlgItemText(hwndDlg,IDC_WID_INFO,buff) ;
            return TRUE;
        }
        
    case WM_COMMAND:
        if(LOWORD(wParam) != IDCANCEL)
            break;
        /* no break */
    case WM_CLOSE:
        EndDialog(hwndDlg,0);
        return TRUE;
	
    }
    return FALSE;
}

/************************************************
 * Dismisses the current window by either moving it
 * back to its assigned desk or minimizing.
 */
static void windowMenu(HWND theWin)
{
    unsigned char Sticky=0;
    unsigned char Tricky=0;
    HMENU hpopup ;
    TCHAR buff[20];
    POINT pt ;
    HWND pWin ;
    int ii, jj, idx ;
    
    vwLogBasic((_T("Window Menu: %x\n"),(int) theWin)) ;
    
    if((theWin == NULL) || (theWin == hWnd) || (theWin == desktopHWnd) || (theWin == taskHWnd))
        return ;
    while(((ii=GetWindowLong(theWin,GWL_STYLE)) & WS_CHILD) && 
          ((pWin = GetParent(theWin)) != NULL) && (pWin != desktopHWnd))
        theWin = pWin ;
    // Must be a visible non-child window
    if((ii & WS_CHILD) || ((ii & WS_VISIBLE) == 0))
        return ;
    
    lockMutex();
    winListUpdate() ;
    if((idx = winListFind(theWin)) >= 0)
    {
        Sticky = winList[idx].Sticky ;
        Tricky = winList[idx].Tricky ;
    }
    releaseMutex();

    if((hpopup = CreatePopupMenu()) == NULL)
        return ;
    
    AppendMenu(hpopup,MF_STRING,ID_WM_DISMISS,_T("&Dismiss Window"));
    if(idx >= 0)
    {
        /* currently managed window */
        if(Sticky)
            AppendMenu(hpopup,MF_STRING,ID_WM_STICKY,_T("Remove &Sticky"));
        else
        {
            ii = nDesksX * nDesksY ;
            for(jj = 1 ; jj <= ii ; jj++)
                if(jj != currentDesk)
                {
                    _stprintf(buff,_T("Move to Desk &%d"),jj) ;
                    AppendMenu(hpopup,MF_STRING,ID_WM_DESK+jj,buff) ;
                }
            AppendMenu(hpopup,MF_STRING,ID_WM_STICKY,_T("Make &Sticky"));
        }
    }
    else
        AppendMenu(hpopup,MF_STRING,ID_WM_MANAGE,_T("&Manage Window"));
    AppendMenu(hpopup,MF_SEPARATOR,0,NULL) ;
    AppendMenu(hpopup,MF_STRING,ID_WM_INFO,_T("&Info"));
    
    GetCursorPos(&pt);
    /* Call setForegroundWin to remove the window focus otherwise the menu does
     * not automatically close if the user changes focus */
    setForegroundWin(NULL,0);
    SetForegroundWindow(hWnd);
    idx = TrackPopupMenu(hpopup,TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,pt.x-2,pt.y-2,0,hWnd,NULL) ;
    PostMessage(hWnd, 0, 0, 0);	
    DestroyMenu(hpopup);		
    vwLogBasic((_T("Window Menu returned %d\n"),(int) idx)) ;
    switch(idx)
    {
    case ID_WM_DISMISS:
        windowDismiss(theWin) ;
        return ;
    case ID_WM_STICKY:
        setSticky(theWin,-1) ;
        break;
    case ID_WM_INFO:
        DialogBox(hInst,MAKEINTRESOURCE(IDD_WINDOWINFODIALOG),theWin,(DLGPROC) WindowInfoDialogFunc);
        return ;
    case ID_WM_MANAGE:
        lockMutex();
        winListUpdate() ;
        if((winListFind(theWin) < 0) && (nWin < MAXWIN))
        {
            idx = nWin++;
            winList[idx].Handle = theWin ;
            winList[idx].Owner = NULL ;
            winList[idx].Style = GetWindowLong(theWin, GWL_STYLE) ;
            winList[idx].ExStyle = GetWindowLong(theWin, GWL_EXSTYLE) ;
            winList[idx].ZOrder[currentDesk] = 1 ;
            winList[idx].Desk = currentDesk ;
            winList[idx].menuId = 0 ;
            winList[idx].Sticky = 0 ;
            winList[idx].Tricky = 0 ;
            winList[idx].Visible = TRUE ;
            winList[idx].State = 1 ;
            vwLogBasic((_T("Got new managed window %d\n"),(int) idx)) ;
        }
        releaseMutex();
        break;
    default:
        if((idx > ID_WM_DESK) && (idx <= (ID_WM_DESK + (nDesksX * nDesksY))))
        {
            assignWindow(theWin,idx - ID_WM_DESK,FALSE) ;
            return ;
        }
    }
    SetForegroundWindow(theWin);
}

/*************************************************
 * Retrieves the system menu font
 */

static HFONT getMenuFont()
{
    NONCLIENTMETRICS metrics;
    HFONT menufont;

    memset(&metrics,0,sizeof(NONCLIENTMETRICS));
    metrics.cbSize=sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(NONCLIENTMETRICS),&metrics,0);

    menufont=CreateFontIndirect(&metrics.lfMenuFont);

    return menufont;
}

/*************************************************
 * Returns the bounding rect for each menu item
 * response to WM_MEASUREITEM
 */

#define MENU_X_PADDING 1
#define MENU_Y_PADDING 1
#define ICON_PADDING 2
#define ICON_SIZE 16

static void measureMenuItem(HWND hwnd,MEASUREITEMSTRUCT* mitem)
{
    HDC testdc;
    HFONT menufont,oldfont;
    MenuItem* item;
    SIZE size;
    TCHAR *measuretext;

    item = (MenuItem*) mitem->itemData;

    measuretext = (item != NULL) ? item->name : _T("D") ;

    menufont=getMenuFont();
    testdc=GetDC(hwnd);
    oldfont=SelectObject(testdc,menufont);

    GetTextExtentPoint32(testdc,measuretext,_tcslen(measuretext),&size);

    // width + end padding + icon + icon padding
    mitem->itemWidth = size.cx + ICON_SIZE + ICON_PADDING + (2*MENU_X_PADDING) ;

    // height + padding (larger of text or icon)
    mitem->itemHeight = (size.cy > ICON_SIZE) ? size.cy + (2*MENU_Y_PADDING) : ICON_SIZE + (2*MENU_Y_PADDING) ;

    SelectObject(testdc,oldfont);
    DeleteObject(menufont);
    ReleaseDC(hwnd,testdc);
}

/*************************************************
 * Renders menu item
 * response to WM_DRAWIEM
 */

static void renderMenuItem(DRAWITEMSTRUCT* ditem)
{
    MenuItem* item;
    HFONT menufont,oldfont;
    HICON icon;
    UINT oldalign;
    HBRUSH focusbrush,oldbrush;
    HPEN oldpen;
    SIZE size;
    int backgroundcolor,textcolor, ll;

    item = (MenuItem*) ditem->itemData;

    if((item != NULL) && (ditem->itemState & ODS_SELECTED))
    {
        backgroundcolor = COLOR_HIGHLIGHT ;
        textcolor = COLOR_HIGHLIGHTTEXT ;
    }
    else
    {
        backgroundcolor = COLOR_MENU ;
        textcolor = COLOR_MENUTEXT ;
    }
    focusbrush=GetSysColorBrush(backgroundcolor);
    SetBkColor(ditem->hDC,GetSysColor(backgroundcolor));

    // menu highlight rectangle
    oldpen=SelectObject(ditem->hDC,GetStockObject(NULL_PEN));
    oldbrush=SelectObject(ditem->hDC,focusbrush);
    Rectangle(ditem->hDC,ditem->rcItem.left,ditem->rcItem.top,ditem->rcItem.right+1,ditem->rcItem.bottom+1);

    menufont=getMenuFont();
    oldfont=SelectObject(ditem->hDC,menufont);
    oldalign=SetTextAlign(ditem->hDC,TA_BOTTOM);
    SetTextColor(ditem->hDC,GetSysColor(textcolor));

    if(item != NULL)
    {
        icon=(ditem->itemState & ODS_CHECKED) ? checkIcon : item->icon;
        DrawIconEx(ditem->hDC,ditem->rcItem.left+MENU_X_PADDING,ditem->rcItem.top+MENU_Y_PADDING,icon,ICON_SIZE,ICON_SIZE,0,0,DI_NORMAL);
        if((ll = _tcslen(item->name)) > 0)
        {
            GetTextExtentPoint32(ditem->hDC,item->name,ll,&size);
            ExtTextOut(ditem->hDC,ditem->rcItem.left+MENU_X_PADDING+ICON_SIZE+ICON_PADDING,
                       ditem->rcItem.bottom-((ditem->rcItem.bottom-ditem->rcItem.top-size.cy+1) >> 1),ETO_OPAQUE,0,item->name,_tcslen(item->name),0);
        }
    }
    SetTextAlign(ditem->hDC,oldalign);
    SelectObject(ditem->hDC,oldfont);
    SelectObject(ditem->hDC,oldpen);
    SelectObject(ditem->hDC,oldbrush);

    DeleteObject(menufont);
}

/*************************************************
 * Pops up and handles the window list menu
 */
static void winListPopupMenu(HWND aHWnd, int forceFocusChange)
{
    static int singleColumn=0;
    HMENU hpopup;
    POINT pt;
    HWND hwnd ;
    int flags, retItem, id, ii ;
    
    // storage for list of MenuItem structs
    MenuItem *items[MAXWIN];
    int itemCount;
    
    if((flags = winListCreateItemList(items,&itemCount)) == 0)
        return ;
    GetCursorPos(&pt);
    pt.x -= 2 ;
    pt.y -= 2 ;
    
    /* Call setForegroundWin to remove the window focus otherwise the menu does
     * not automatically close if the user changes focus, unfortunately this breaks
     * double clicking on the systray icon so not done in this case */
    if(forceFocusChange)
        setForegroundWin(NULL,0) ;
    SetForegroundWindow(aHWnd);
    
    for(;;)
    {
        retItem = 0 ;
        if(flags & vwPMENU_COMPACT)
        {
            while((singleColumn & flags) == 0)
            {
                singleColumn = singleColumn << 1 ;
                if((singleColumn & vwPMENU_COL_MASK) == 0)
                    singleColumn = vwPMENU_STICKY ;
            }
            ii = vwPMENU_TITLES | vwPMENU_COMPACT | singleColumn ;
        }
        else
            ii = flags ;
        if((hpopup = winListCreateMenu(ii,itemCount,items)) == NULL)
            break ;
        retItem = TrackPopupMenu(hpopup, TPM_RETURNCMD | TPM_LEFTBUTTON, // Return menu code
                                 pt.x, pt.y, 0, aHWnd, NULL);
        vwLogBasic((_T("Window menu returned: %x\n"),(int) retItem)) ;
        PostMessage(aHWnd, 0, 0, 0) ;
        DestroyMenu(hpopup) ;
        if(((retItem & vwPMENU_COL_MASK) == 0) || ((retItem & vwPMENU_ID_MASK) != 0))
            break ;
        singleColumn = singleColumn << 1 ;
    }
            
    if(retItem & vwPMENU_ID_MASK)
    {
        id = retItem & vwPMENU_ID_MASK ;
        ii = nWin ;
        while(--ii >= 0)
            if(winList[ii].menuId == id)
                break ;
        if(ii >= 0)
        {
            hwnd = winList[ii].Handle ;
            vwLogVerbose((_T("Menu select %x %d %x\n"),retItem,ii,(int) hwnd)) ;
            if(retItem & vwPMENU_STICKY)
                // Sticky toggle
                setSticky(hwnd,-1) ;
            else if(retItem & vwPMENU_ACCESS)
            {   // window access - restore if minimized
                gotoDesk(winList[ii].Desk,FALSE);
                if(winList[ii].Style & WS_MINIMIZE)
                    ShowWindow(hwnd,SW_SHOWNORMAL) ;
                setForegroundWin(hwnd,0);
            } 
            else
            {   // Assign to this desktop
                assignWindow(hwnd,currentDesk,TRUE) ;
                if(winList[ii].Style & WS_MINIMIZE)
                    ShowWindow(hwnd,SW_SHOWNORMAL) ;
                setForegroundWin(hwnd,0) ;
            }
        }
    }
    
    // delete MenuItem list
    for (ii=0 ; ii<itemCount ; ii++)
    {
        free(items[ii]->name);
        free(items[ii]) ;
    }
}

/*************************************************
 * Main window callback, this is where all main window messages are taken care of
 */
static LRESULT CALLBACK
wndProc(HWND aHWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static UINT taskbarRestart; 
    POINT pt;
    
    switch (message)
    {
    case VW_MOUSEWARP:
        // Is virtuawin enabled
        if(enabled)
        {
            GetCursorPos(&pt);
            
            isDragging = (LOWORD(lParam) & 0x1) ;
            switch(HIWORD(lParam))
            {
            case 0:
                /* left */
                if((stepLeft() != 0) && (LOWORD(lParam) & 0x2))
                {
                    if(noMouseWrap)
                        SetCursorPos(pt.x + warpLength, pt.y);
                    else
                        SetCursorPos(desktopWorkArea[0][2] - warpLength, pt.y);
                }
                break;
                
            case 1:
                /* up */
                if(invertY)
                    wParam = stepDown();
                else
                    wParam = stepUp();
                if((wParam != 0) && (LOWORD(lParam) & 0x2))
                {
                    if(noMouseWrap)
                        SetCursorPos(pt.x, pt.y + warpLength);
                    else
                        SetCursorPos(pt.x, desktopWorkArea[0][3] - warpLength);
                }
                break;
                
            case 2:
                /* right */
                if((stepRight() != 0) && (LOWORD(lParam) & 0x2))
                {
                    if(noMouseWrap)
                        SetCursorPos(pt.x - warpLength, pt.y);
                    else
                        SetCursorPos(desktopWorkArea[0][0] + warpLength, pt.y);
                }
                break;
                
            case 3:
                /* down */
                if(invertY)
                    wParam = stepUp();
                else
                    wParam = stepDown();
                if((wParam != 0) && (LOWORD(lParam) & 0x2))
                {
                    if(noMouseWrap)
                        SetCursorPos(pt.x, pt.y - warpLength);
                    else
                        SetCursorPos(pt.x, desktopWorkArea[0][1] + warpLength);
                }
                break;
            
            case 4:
                /* window list */
                winListPopupMenu(aHWnd,1) ;
                break;
            
            case 5:
                /* window menu */
                windowMenu(GetForegroundWindow());
                break;
            }
            isDragging = FALSE;
        }
        return TRUE;
        
    case WM_HOTKEY:				// A hot key was pressed
        if(!enabled)
            return FALSE ;
        // Cycling hot keys
        if(wParam == cyclingKeyUp)
            stepDelta(1) ;
        else if(wParam == cyclingKeyDown)
            stepDelta(-1) ;
        // Desk switch hot keys
        else if(wParam == vwLeft)
            stepLeft();
        else if(wParam == vwRight)
            stepRight();
        else if(wParam == vwUp)
        {
            if(invertY)
                stepDown();
            else
                stepUp();
        }
        else if(wParam == vwDown)
        {
            if(invertY)
                stepUp();
            else
                stepDown();
        }
        else if(wParam == stickyKey)
            setSticky(0,-1);
        else if(wParam == dismissKey)
        {
            windowDismiss(GetForegroundWindow());
        }
        else if(wParam == vwWMenu)
        {
            if(enabled)
                windowMenu(GetForegroundWindow());
        }
        else if(wParam == vwWList)
        {
            if(enabled)
                winListPopupMenu(aHWnd,1) ;
        }
        else
        {
            // Desktop Hot keys
            int ii=MAXDESK-1 ;
            do {
                if(wParam == vwDesk[ii])
                {
                    gotoDesk(ii,TRUE);
                    break ;
                }
            } while(--ii > 0) ;
        }
        return TRUE;
        
        // Plugin messages
    case VW_CHANGEDESK: 
        if(!enabled)
            return FALSE ;
        switch (wParam)
        {
        case VW_STEPPREV:
            stepDelta(-1) ;
            break;
        case VW_STEPNEXT:
            stepDelta(1) ;
            break;
        case VW_STEPLEFT:
            stepLeft();
            break;
        case VW_STEPRIGHT:
            stepRight();
            break;
        case VW_STEPUP:
            stepUp();
            break;
        case VW_STEPDOWN:
            stepDown();
            break;
        default:
            gotoDesk(wParam,FALSE);
            break;
        }
        return TRUE;
        
    case VW_CLOSE: 
        shutDown();
        return TRUE;
        
    case VW_SETUP:
        if(!enabled)
            return FALSE ;
        showSetup();
        return TRUE;
        
    case VW_DELICON:
        Shell_NotifyIcon(NIM_DELETE, &nIconD); // This removes the icon
        return TRUE;
        
    case VW_SHOWICON:
        Shell_NotifyIcon(NIM_ADD, &nIconD);    // This re-adds the icon
        return TRUE;
        
    case VW_HELP:
        showHelp(aHWnd,0);
        return TRUE;
        
    case VW_GATHER:
        showAll(0);
        return TRUE;
        
    case VW_DESKX:
        return nDesksX;
        
    case VW_DESKY:
        return nDesksY;
        
    case VW_CURDESK:
        return currentDesk;
        
    case VW_ASSIGNWIN:
        if(!enabled)
            return FALSE ;
        return assignWindow((HWND) wParam,(int)lParam,FALSE);
        
    case VW_ACCESSWIN:
        if(!enabled)
            return FALSE ;
        return accessWindow((HWND)wParam,(int)lParam,FALSE);
        
    case VW_SETSTICKY:
        if(!enabled)
            return FALSE ;
        return setSticky((HWND)wParam, (int)lParam);
        
    case VW_GETWINDESK:
        {
            int ii ;
            lockMutex();
            if((ii = winListFind((HWND)wParam)) >= 0)
                ii = (int) winList[ii].Desk ;
            else
                ii = 0;
            releaseMutex();
            return ii ;
        }
        
    case VW_FOREGDWIN:
        {
            int ii ;
            
            lockMutex();
            if((ii = winListFind((HWND)wParam)) >= 0)
            {
                /* found the window, if visible bring to the front otherwise set the zorder */
                if(lParam == 0)
                    lParam = currentDesk ;
                if((lParam == currentDesk) && winList[ii].Visible)
                    setForegroundWin((HWND)wParam,0);
                winList[ii].ZOrder[lParam] = vwZOrder ;
                ii = 1 ;
            }
            else
                ii = 0 ;
            releaseMutex();
            return ii ;
        }
    
    case VW_WINLIST:
        {
            // Send over the window list with WM_COPYDATA
            COPYDATASTRUCT cds;         
            lockMutex();
            winListUpdate() ;
            cds.dwData = nWin;
            cds.cbData = sizeof(winList);
            cds.lpData = (void*)winList;
            releaseMutex();
            sendModuleMessage(WM_COPYDATA, (WPARAM) aHWnd, (LPARAM)&cds); 
            return TRUE;
        }
        
    case VW_INSTALLPATH:
    case VW_USERAPPPATH:
        {
            // Send over the VirtuaWin install path with WM_COPYDATA
            //  - always use a byte string so unicode/non-uncode modules can work together
            COPYDATASTRUCT cds;
            char *ss = (message == VW_INSTALLPATH) ? VirtuaWinPathStr:UserAppPathStr ;
            cds.dwData = 0 - message ;
            cds.cbData = strlen(ss) + 1 ;
            cds.lpData = (void*)ss;
            sendModuleMessage(WM_COPYDATA, (WPARAM) aHWnd, (LPARAM)&cds); 
            return TRUE;
        }
    case VW_DESKIMAGE:
        if(wParam == 0)
            return deskImageCount ;
        else if(wParam == 1)
            return enableDeskImage(lParam) ;
        else if(deskImageCount == 0)
            return 0 ;
        else if(wParam == 2)
            return disableDeskImage(1) ;
        else if(wParam == 3)
            return createDeskImage(currentDesk,0) ;
        else if(wParam == 4)
            return deskImageInfo.bmiHeader.biHeight ;
        else if(wParam == 5)
            return deskImageInfo.bmiHeader.biWidth ;
        else
            return 0 ;
        
    case VW_ENABLE_STATE:
        lParam = enabled ;
        if((wParam == 1) || ((wParam == 2) && enabled) ||
           ((wParam == 3) && !enabled))
            disableAll(aHWnd);
        return lParam ;
        
        // End plugin messages
        
    case WM_CREATE:		       // when main window is created
        // register message for explorer/systray crash restart
        // only works with >= IE4.0 
        taskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
        return TRUE;
        
    case WM_ENDSESSION:
        if(wParam)
            shutDown();
        return TRUE;
        
    case WM_DESTROY:	  // when application is closed
        shutDown();            
        return TRUE;
        
    case UWM_SYSTRAY:		   // We are being notified of mouse activity over the icon
        switch (lParam)
        {
        case WM_LBUTTONDOWN:               // Show the window list
            if(enabled)
                winListPopupMenu(aHWnd,0) ;
            break;
        
        case WM_LBUTTONDBLCLK:             // double click on icon
            if(enabled)
                showSetup();
            break;
            
        case WM_MBUTTONUP:		   // Move to the next desktop
            if(enabled)
                stepDelta((HIWORD(GetKeyState(VK_SHIFT))) ? -1:1) ;
            break;
            
        case WM_RBUTTONUP:		   // Let's track a popup menu
            if(setupOpen)
            {
                /* dont allow access to this menu while setup is open as its
                 * behaviour is less predictable (shutting down while setup
                 * is open will hang the virtuawin process) */
                showSetup() ;
            }
            else
            {
                HMENU hpopup;
                
                if((hpopup = CreatePopupMenu()) == NULL)
                    return FALSE ;
    
                if(enabled)
                    AppendMenu(hpopup,MF_STRING,ID_SETUP,_T("&Setup"));
                AppendMenu(hpopup,MF_STRING,ID_GATHER,_T("&Gather"));
                AppendMenu(hpopup,MF_STRING,ID_HELP,_T("&Help"));
                AppendMenu(hpopup,MF_STRING,ID_DISABLE,(enabled) ? _T("&Disable") : _T("&Enable"));
                AppendMenu(hpopup,MF_SEPARATOR,0,NULL) ;
                AppendMenu(hpopup,MF_STRING,ID_EXIT,_T("E&xit"));
                if(enabled)
                {
                    AppendMenu(hpopup,MF_SEPARATOR,0,NULL) ;
                    AppendMenu(hpopup,MF_STRING,ID_FORWARD,_T("&Next"));
                    AppendMenu(hpopup,MF_STRING,ID_BACKWARD,_T("&Back"));
                }
                GetCursorPos(&pt);
                SetForegroundWindow(aHWnd);
                switch (TrackPopupMenu(hpopup, TPM_RETURNCMD |    // Return menu code
                                       TPM_RIGHTBUTTON, (pt.x-2), (pt.y-2), // screen coordinates
                                       0, aHWnd, NULL))
                {
                case ID_SETUP:		// show setup box
                    showSetup();
                    break;
                case ID_EXIT:		// exit application
                    DestroyWindow(aHWnd);
                    break;
                case ID_FORWARD:	// move to the next desktop
                    stepDelta(1) ;
                    break;
                case ID_BACKWARD:	// move to the previous desktop
                    stepDelta(-1) ;
                    break;
                case ID_GATHER:		// gather all windows
                    showAll(0);
                    break;
                case ID_DISABLE:	// Disable VirtuaWin
                    disableAll(aHWnd);
                    break;
                case ID_HELP:		// show help
                    showHelp(aHWnd,0);
                    break;
                }
                PostMessage(aHWnd, 0, 0, 0);	
                DestroyMenu(hpopup);
            }
            break;
        }
        return TRUE;
        
    case WM_DISPLAYCHANGE:
        /* screen size has changed, get the new size and set the mouse work area */
        getScreenSize();
        if(deskImageCount > 0)
        {
            int hh = deskImageInfo.bmiHeader.biHeight ;
            deskImageInfo.bmiHeader.biHeight -= 1 ;
            enableDeskImage(hh) ;
        }
        /* no break */
    case WM_SETTINGCHANGE:
        /* the position and size of the taskbar may have changed */ 
        getWorkArea();
        return TRUE;
    case WM_MEASUREITEM:
        measureMenuItem(aHWnd,(MEASUREITEMSTRUCT*)lParam);
        break;
    case WM_DRAWITEM:
        renderMenuItem((DRAWITEMSTRUCT*)lParam);        
        break;
    default:
        // If taskbar restarted
        if((message == taskbarRestart) && displayTaskbarIcon )
        {
            Shell_NotifyIcon(NIM_ADD, &nIconD);	// This re-adds the icon
            goGetTheTaskbarHandle();
        }
        break;
    }
    return DefWindowProc(aHWnd, message, wParam, lParam);
}

static int
VirtuaWinInit(HINSTANCE hInstance)
{
    OSVERSIONINFO os;
    WNDCLASSEX wc;
    DWORD threadID;
    BOOL awStore;
    TCHAR *classname = vwVIRTUAWIN_NAME _T("MainClass") ;
    hInst = hInstance;
    
#ifdef _WIN32_MEMORY_DEBUG
    /* Enable heap checking on each allocate and free */
    _CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF|_CRTDBG_DELAY_FREE_MEM_DF|
                    _CRTDBG_LEAK_CHECK_DF|_CRTDBG_DELAY_FREE_MEM_DF);
#endif
    
    /* Only one instance may be started */
    hMutex = CreateMutex(NULL, FALSE, vwVIRTUAWIN_NAME _T("PreventSecond"));
    if(GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // Display configuration window...
        PostMessage(FindWindow(classname, NULL), VW_SETUP, 0, 0);
        return 0; // ...and quit 
    }
    
    os.dwOSVersionInfoSize = sizeof(os);
    GetVersionEx(&os);
    if(os.dwPlatformId == VER_PLATFORM_WIN32s)
        osVersion = OSVERSION_31 ;
    else if(os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        osVersion = OSVERSION_9X ;
    else if(os.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        if(os.dwMajorVersion < 5)
            osVersion = OSVERSION_NT ;
        else if(os.dwMinorVersion == 0)
            osVersion = OSVERSION_2000 ;
        else
            osVersion = OSVERSION_XP ;
    }
    
    /* Create a window class for the window that receives systray notifications.
       The window will never be displayed */
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = wndProc;
    wc.cbClsExtra = wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIRTUAWIN));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = classname;
    wc.hIconSm = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_VIRTUAWIN), IMAGE_ICON,
                                   GetSystemMetrics(SM_CXSMICON),
                                   GetSystemMetrics(SM_CYSMICON), 0);
    if(RegisterClassEx(&wc) == 0)
    {
        MessageBox(hWnd,_T("Failed to register class!"),vwVIRTUAWIN_NAME _T(" Error"), MB_ICONWARNING);
        return 0 ;
    }
    
    readConfig();	// Read the config file
    if(vwLogFlag)
    {
        TCHAR logFname[MAX_PATH] ;
        GetFilename(vwCONFIG,1,logFname) ;
        _tcscpy(logFname+_tcslen(logFname)-3,_T("log")) ;
        vwLogFile = _tfopen(logFname,_T("w+")) ;
        vwLogBasic((vwVIRTUAWIN_NAME_VERSION _T("\n"))) ;
    }
    
    
    /* set the window to give focus to when releasing focus on switch also used to refresh */
    desktopHWnd = GetDesktopWindow();
    deskIconHWnd = FindWindow(_T("Progman"), _T("Program Manager")) ;
    deskIconHWnd = FindWindowEx(deskIconHWnd, NULL, _T("SHELLDLL_DefView"),NULL) ;
    deskIconHWnd = FindWindowEx(deskIconHWnd, NULL, _T("SysListView32"), _T("FolderView")) ;
    getScreenSize();
    getWorkArea(); // This is dependent on the config
    
    // Fix some things for the alternate hide method
    RM_Shellhook = RegisterWindowMessage(_T("SHELLHOOK"));
    goGetTheTaskbarHandle();
    
    loadIcons();
    
    // Load tricky windows, must be done before the crashRecovery checks
    if(trickyWindows)
        curTricky = loadTrickyList(trickyList) ;
    
    /* Create window. Note that WS_VISIBLE is not used, and window is never shown. */
    if((hWnd = CreateWindowEx(0, classname, classname, WS_POPUP, CW_USEDEFAULT, 0,
                              CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL)) == NULL)
    {
        MessageBox(hWnd,_T("Failed to create window!"),vwVIRTUAWIN_NAME _T(" Error"), MB_ICONWARNING);
        return 0 ;
    }
    
    nIconD.cbSize = sizeof(NOTIFYICONDATA); // size
    nIconD.hWnd = hWnd;		    // window to receive notifications
    nIconD.uID = 1;		    // application-defined ID for icon (can be any UINT value)
    nIconD.uFlags = NIF_MESSAGE |   // nIconD.uCallbackMessage is valid, use it
          NIF_ICON |		    // nIconD.hIcon is valid, use it
          NIF_TIP;		    // nIconD.szTip is valid, use it
    nIconD.uCallbackMessage = UWM_SYSTRAY;  // message sent to nIconD.hWnd
    nIconD.hIcon = icons[1];
    
    _tcscpy(nIconD.szTip,vwVIRTUAWIN_NAME_VERSION);		// Tooltip
    if( displayTaskbarIcon )
    {
        // This adds the icon
        if(Shell_NotifyIcon(NIM_ADD, &nIconD) == 0)
        {
            BOOL retry = 1;
            do
            {
                Sleep(2000);
                // Maybe Systray process hasn't started yet, try again
                retry = Shell_NotifyIcon(NIM_ADD, &nIconD);  // This adds the icon
            }
            while(retry != 0);
        }
    }
    /* Register the keys */
    registerAllKeys();
    setMouseKey();
    
    /* Load some stuff */
    curSticky   = loadStickyList(stickyList);
    curAssigned = loadAssignedList(assignedList);
    curUser     = loadUserList(userList);  // load any user defined windows
    
    /* always move windows immediately on startup */
    awStore = assignImmediately ;
    assignImmediately = TRUE ;
    lockMutex();
    winListUpdate() ;
    releaseMutex();
    assignImmediately = awStore ;
    
    /* Load user modules */
    curDisabledMod = loadDisabledModules(disabledModules);
    loadModules();
    
    /* Create the thread responsible for mouse monitoring */   
    mouseThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MouseProc, NULL, 0, &threadID); 	
    if(!mouseEnable)
        // Suspend the thread if mouse support not required
        enableMouse(FALSE);
    
    // if on win9x the tricky windows need to be continually hidden
    taskbarFixRequired = (osVersion <= OSVERSION_9X) ;
    SetTimer(hWnd, 0x29a, 250, monitorTimerProc); 
    
    return 1 ;
}

/*************************************************
 * VirtuaWin start point
 */
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    
    if(!VirtuaWinInit(hInstance))
        return 0 ;
    
    /* Main message loop */
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    CloseHandle(hMutex);
    return msg.wParam;
}
