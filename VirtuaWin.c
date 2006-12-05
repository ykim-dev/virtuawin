//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  VirtuaWin.c - Core VirtuaWin routines.
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
#include <commctrl.h>

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

int screenLeft;
int screenRight;
int screenTop;
int screenBottom;

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
ATOM vwMenu;
ATOM cyclingKeyUp;
ATOM cyclingKeyDown;
ATOM stickyKey;
ATOM dismissKey;

BOOL enabled = TRUE;		// if VirtuaWin enabled or not
BOOL isDragging = FALSE;	// if we are currently dragging a window

int taskBarLeftWarp   = 0;      // the warp size for Left taskbar
int taskBarRightWarp  = 0;      // the warp size for Right taskbar
int taskBarTopWarp    = 0;      // the warp size for Top taskbar
int taskBarBottomWarp = 0;      // the warp size for Bottom taskbar
int taskbarOffset     = 3;      // Default 3, 0 if XP skinned taskbar is used.

HINSTANCE hInst;		// current instance
HWND taskHWnd;                  // handle to taskbar
HWND desktopHWnd;		// handle to the desktop window
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

HBITMAP iconReferenceVector[MAXWIN];
int vectorPosition = 0;

// Config parameters, see ConfigParameters.h for descriptions
int saveInterval = 0;
int nOfModules = 0;  
int nWin = 0;        
int currentDeskX = 1;
int currentDeskY = 1;
int currentDesk = 1; 
int nDesksX = 2;     
int nDesksY = 2;     
int warpLength = 20; 
int warpMultiplier = 0;
int configMultiplier = 1;
int preserveZOrder = 0;      
int hiddenWindowAct = 1;
int vwLogFlag = 0 ;
FILE *vwLogFile ;
BOOL noMouseWrap = FALSE;
BOOL mouseEnable = FALSE; 
BOOL useMouseKey = FALSE;
BOOL keyEnable = TRUE;		
BOOL hotKeyEnable = FALSE;      
BOOL releaseFocus = FALSE;	
BOOL minSwitch = TRUE;		
UINT modAlt = 0;		
UINT modShift = 0;		
UINT modCtrl = 0;		
UINT modWin = MOD_WIN;		
UINT mouseModAlt = 0;	
UINT mouseModShift = 0;	
UINT mouseModCtrl = 0;	
BOOL taskBarWarp = FALSE;       
BOOL saveSticky = FALSE;        
BOOL refreshOnWarp = FALSE;     
BOOL stickyKeyRegistered = FALSE;
BOOL dismissKeyRegistered = FALSE;
BOOL deskWrap = FALSE;          
BOOL invertY = FALSE;           
short stickyMenu = 1;         
short assignMenu = 1;
short directMenu = 1;
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
UINT hotkeyMenuEn = 0;
UINT hotkeyMenu = 0;
UINT hotkeyMenuMod = 0;
UINT hotkeyMenuWin = 0;
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
 * Checks if mouse key modifier is pressed
 */
static BOOL checkMouseState(void)
{
    if(!GetSystemMetrics(SM_SWAPBUTTON)) {  // Check the state of mouse button(s)
        if(HIWORD(GetAsyncKeyState(VK_LBUTTON)))
            return TRUE;
        else
            return FALSE;
    } else if(HIWORD(GetAsyncKeyState(VK_RBUTTON)))
        return TRUE;
    else
        return FALSE;
}

/*************************************************
 * The mouse thread. This function runs in a thread and checks the mouse
 * position every 50ms. If mouse support is disabled, the thread will be in 
 * suspended state.
 */
DWORD WINAPI MouseProc(LPVOID lpParameter)
{
    // in this function I'm using int for bool & 0,1 rather than
    // false,true since the file extension is .c and it's being compiled
    // as C rather than C++ ... this should change when we port to C++
    
    int mousekeyPressed = 0, xDelta, yDelta ;
    int movingLeft, movingRight, movingUp, movingDown ; 
    POINT firstPoint;
    POINT pt;
    
    // infinite loop
    while(1)
    {
        // If the useMouseKey function is turned on (the function
        // that requires a modifier key to be pressed to change
        // desktops using the mouse) and that key is pressed, then
        // we need to get the position at which the modifier key was
        // pressed.  Later we'll use that first position to see if
        // there's a motion tendency towards the edge of the screen that
        // we're switching to.  If they're not moving the mouse in that
        // general direction then we don't want to make the switch
        // because that sometimes causes the screen to switch when the
        // user presses the modifier key for some other purpose if the
        // mouse is near the edge of the screen.  Checking for their
        // general motion tendency feels natural and prevents accidental
        // switching
        
        if(useMouseKey)
        {   // Are we using a mouse key
            if(HIWORD(GetAsyncKeyState(MOUSEKEY)))
            {
                if(!mousekeyPressed)
                {
                    mousekeyPressed = 1;
                    GetCursorPos(&firstPoint);
                }
            }
            else
                mousekeyPressed = 0;
        }

        // sleep between iterations of this function.  If we're using
        // modifier keys it loops more often to be sure to watch the
        // tendency.
        if(useMouseKey) {
            Sleep(4);
        }
        else {
            Sleep(50); 
        }
        
        // Now get the second point
        GetCursorPos(&pt);
        
        // Now figure out the motion tendency
        xDelta = pt.x - firstPoint.x;
        yDelta = pt.y - firstPoint.y;
        
        // If they're not using the modifier keys we'll just set all of
        // these to true to simplify the logic later on
        movingLeft  = !useMouseKey || (mousekeyPressed && (xDelta < -25)); // && (yDelta > -30 && yDelta < 30);
        movingRight = !useMouseKey || (mousekeyPressed && (xDelta >  25)); // && (yDelta > -30 && yDelta < 30);
        movingUp    = !useMouseKey || (mousekeyPressed && (yDelta < -25)); // && (xDelta > -30 && xDelta < 30);
        movingDown  = !useMouseKey || (mousekeyPressed && (yDelta >  25)); // && (xDelta > -30 && xDelta < 30);
        
        // ...and if we're moving in the right direction and close
        // enough to the side of the screen, send the message to switch
        // desktops
        if( movingLeft  &&  pt.x < (screenLeft   + 3 + (taskBarWarp * taskBarLeftWarp   * checkMouseState())))
            // switch left
            SendNotifyMessage(hWnd, VW_MOUSEWARP, 0, MAKELPARAM(checkMouseState(), VW_MOUSELEFT));
        else if( movingRight && pt.x > (screenRight  - 3 - (taskBarWarp * taskBarRightWarp  * checkMouseState())))
            // switch right
            SendNotifyMessage(hWnd, VW_MOUSEWARP, 0, MAKELPARAM(checkMouseState(), VW_MOUSERIGHT));
        else if( movingUp && pt.y < (screenTop    + 3 + (taskBarWarp * taskBarTopWarp    * checkMouseState())))
            // switch up
            SendNotifyMessage(hWnd, VW_MOUSEWARP, 0, MAKELPARAM(checkMouseState(), VW_MOUSEUP));
        else if( movingDown && pt.y > (screenBottom - 3 - (taskBarWarp * taskBarBottomWarp * checkMouseState())))
            // switch down
            SendNotifyMessage(hWnd, VW_MOUSEWARP, 0, MAKELPARAM(checkMouseState(), VW_MOUSEDOWN));
        else
            SendNotifyMessage(hWnd, VW_MOUSEWARP, 0, MAKELPARAM(0, VW_MOUSERESET));
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
    char buff[16] ;
    
    if(nDesksY != 2 || nDesksX != 2) // if 2 by 2 mode
        iconId = IDI_ST_0 ;
    else if(osVersion > OSVERSION_2000)
        iconId = IDI_ST_DIS_2 ;
    else
        iconId = IDI_ST_DIS_1 ;
    
    strcpy(buff,"icons/X.ico") ;
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
        vwLeft = GlobalAddAtom("atomKeyLeft");
        if((RegisterHotKey(hWnd, vwLeft, MODKEY, VK_LEFT) == FALSE))
            return FALSE;
        vwRight = GlobalAddAtom("atomKeyRight");
        if((RegisterHotKey(hWnd, vwRight, MODKEY, VK_RIGHT) == FALSE))
            return FALSE;
        vwUp = GlobalAddAtom("atomKeyUp");
        if((RegisterHotKey(hWnd, vwUp, MODKEY, VK_UP) == FALSE))
            return FALSE;
        vwDown = GlobalAddAtom("atomKeyDown");
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
    char buff[16];
    int ii ;
    if(!hotKeysRegistred)
    {
        hotKeysRegistred = TRUE;
        strcpy(buff,"atomKeyP") ;
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
    if(!cyclingKeysRegistered && cyclingKeysEnabled) {
        cyclingKeysRegistered = TRUE;
        if(hotCycleUp)
        {
            cyclingKeyUp = GlobalAddAtom("VWCyclingKeyUp");
            if((RegisterHotKey(hWnd, cyclingKeyUp, hotKey2ModKey(hotCycleUpMod) | hotCycleUpWin, hotCycleUp) == FALSE))
                return FALSE;
        }
        if(hotCycleDown)
        {
            cyclingKeyDown = GlobalAddAtom("VWCyclingKeyDown");
            if((RegisterHotKey(hWnd, cyclingKeyDown, hotKey2ModKey(hotCycleDownMod) | hotCycleDownWin, hotCycleDown) == FALSE))
                return FALSE;
        }
    }
    return TRUE;
}

/*************************************************
 * Unregister the cycling  hot keys, if previosly registred
 */
static void unRegisterCyclingKeys(void)
{
    if(cyclingKeysRegistered) {
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
    if(!menuHotKeyRegistered && hotkeyMenuEn && hotkeyMenu) {
        menuHotKeyRegistered = TRUE;
        vwMenu = GlobalAddAtom("atomKeyMenu");
        if(RegisterHotKey(hWnd, vwMenu, hotKey2ModKey(hotkeyMenuMod) | hotkeyMenuWin, hotkeyMenu) == FALSE)
            return FALSE;
    }
    return TRUE;
}

/*************************************************
 * Unregister the window menu hot key, if previosly registred
 */
static void unRegisterMenuHotKey(void)
{
    if(menuHotKeyRegistered) {
        menuHotKeyRegistered = FALSE;
        UnregisterHotKey(hWnd, vwMenu);
    }
}

/*************************************************
 * Register the sticky hot key, if defined
 */
static BOOL registerStickyKey(void)
{
    if(!stickyKeyRegistered && hotkeyStickyEn && hotkeySticky) {
        stickyKeyRegistered = TRUE;
        stickyKey = GlobalAddAtom("VWStickyKey");
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
        dismissKey = GlobalAddAtom("VWDismissKey");
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
        MessageBox(hWnd, "Invalid key modifier combination, check cursor hotkeys!", 
                   vwVIRTUAWIN_NAME " Error", MB_ICONWARNING);
    if(!registerCyclingKeys())
        MessageBox(hWnd, "Invalid key modifier combination, check cycling hotkeys!", 
                   vwVIRTUAWIN_NAME " Error", MB_ICONWARNING);
    if(!registerHotKeys())
        MessageBox(hWnd, "Invalid key modifier combination, check hotkeys!", 
                   vwVIRTUAWIN_NAME " Error", MB_ICONWARNING);
    if(!registerMenuHotKey())
        MessageBox(hWnd, "Invalid key modifier combination, check menu hotkey!", 
                   vwVIRTUAWIN_NAME " Error", MB_ICONWARNING);
    if(!registerStickyKey())
        MessageBox(hWnd, "Invalid key modifier combination, check sticky hotkey!", 
                   vwVIRTUAWIN_NAME " Error", MB_ICONWARNING);
    if(!registerDismissKey())
        MessageBox(hWnd, "Invalid key modifier combination, check dismiss window hotkey!", 
                   vwVIRTUAWIN_NAME " Error", MB_ICONWARNING);
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

/************************************************
 * Get screen width and height and store values in
 * global variables
 */
static void getScreenSize(void)
{
    /* The virtual screen size system matrix values were only added for WINVER >= 0x0500 (Win2k) */
#ifndef SM_XVIRTUALSCREEN
#define SM_XVIRTUALSCREEN       76
#define SM_YVIRTUALSCREEN       77
#define SM_CXVIRTUALSCREEN      78
#define SM_CYVIRTUALSCREEN      79
#endif

#if 0
    // TODO:  make a user configured flag to specify whether to only act on the primary display or not
    if(0)
    {
        // TODO:  Figure out how to get the size of JUST the primary
        // monitor... I have the width, but dont' know what to do to get
        // the origin w/o using the GetDeviceCaps functions that don't
        // seem to work in MingW
        screenLeft   = GetSystemMetrics(SM_XVIRTUALSCREEN);
        screenRight  = GetSystemMetrics(SM_CXVIRTUALSCREEN) + screenLeft;
        screenTop    = GetSystemMetrics(SM_YVIRTUALSCREEN);
        screenBottom = GetSystemMetrics(SM_CYVIRTUALSCREEN) + screenTop;
    }
#endif
    if((screenRight  = GetSystemMetrics(SM_CXVIRTUALSCREEN)) <= 0)
    {
        /* The virtual screen size system matrix values are not supported on
         * this OS (Win95 & NT), use the desktop window size */
        RECT r;
        GetClientRect(desktopHWnd, &r);
        screenLeft   = r.left;
        screenRight  = r.right;
        screenTop    = r.top;
        screenBottom = r.bottom;
    }
    else
    {
        screenLeft   = GetSystemMetrics(SM_XVIRTUALSCREEN);
        screenRight += screenLeft;
        screenTop    = GetSystemMetrics(SM_YVIRTUALSCREEN);
        screenBottom = GetSystemMetrics(SM_CYVIRTUALSCREEN)+screenTop;
    }
    vwLogBasic((vwLogFile,"Got screen size: %d %d -> %d %d\n",screenLeft,screenRight,screenTop,screenBottom)) ;
}

/************************************************
 * Tries to locate the handle to the taskbar
 */
static void goGetTheTaskbarHandle(void)
{
    if(!noTaskbarCheck)
    {
        HWND hwndTray = FindWindowEx(NULL, NULL, "Shell_TrayWnd", NULL);
        HWND hwndBar = FindWindowEx(hwndTray, NULL, "ReBarWindow32", NULL );
        
        // Maybe "RebarWindow32" is not a child to "Shell_TrayWnd", then try this
        if( hwndBar == NULL )
            hwndBar = hwndTray;
        
        taskHWnd = FindWindowEx(hwndBar, NULL, "MSTaskSwWClass", NULL);
        
        if( taskHWnd == NULL )
            MessageBox(hWnd, "Could not locate handle to the taskbar.\n This will disable the ability to hide troublesome windows correctly.",vwVIRTUAWIN_NAME " Error", 0); 
    }
}

/************************************************
 * Grabs and stores the taskbar coordinates
 */
void getTaskbarLocation(void)
{
    if(!noTaskbarCheck)
    {
        RECT r;
        taskBarLeftWarp   = 0;
        taskBarRightWarp  = 0;
        taskBarTopWarp    = 0;
        taskBarBottomWarp = 0;
        
        /* Get the height of the task bar */
        GetWindowRect(FindWindow("Shell_traywnd", ""), &r);
        /* Determine position of task bar */
        if ((r.bottom + r.top) == (screenBottom - screenTop)) // task bar is on side
        {
            if (r.left <= screenLeft)                          // task bar is on left
                taskBarLeftWarp   = r.right - r.left - taskbarOffset;
            else                                               // task bar is on right
                taskBarRightWarp  = r.right - r.left - taskbarOffset;
        }
        else                                                  // task bar is on top/bottom
        {
            if (r.top <= screenTop)                            // task bar is on top
                taskBarTopWarp    = r.bottom - r.top - taskbarOffset;
            else                                               // task bar is on bottom
                taskBarBottomWarp = r.bottom - r.top - taskbarOffset;
        }
    }
    vwLogBasic((vwLogFile,"Got taskbar location: %d %d -> %d %d\n",taskBarLeftWarp,taskBarRightWarp,taskBarTopWarp,taskBarBottomWarp)) ;
}

/************************************************
 * Show the VirtuaWin help pages
 */
void showHelp(HWND aHWnd, UINT context)
{
    char buff[MAX_PATH];
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

static BOOL checkIfWindowMatch(vwWindowMatch *wm, char *className, char *windowName)
{
    if(wm->type & 0x02)
    {
        static meRegex regex ;
        int ll ;
        if(meRegexComp(&regex,wm->match,0) != meREGEX_OKAY)
            return 0 ;
        if(wm->type & 0x01)
        {
            ll = strlen(windowName) ;
            return meRegexMatch(&regex,windowName,ll,0,ll,(meREGEX_BEGBUFF|meREGEX_ENDBUFF)) ;
        }
        ll = strlen(className) ;
        return meRegexMatch(&regex,className,ll,0,ll,(meREGEX_BEGBUFF|meREGEX_ENDBUFF)) ;
    }
    else if(wm->type & 0x01)
        return (!strcmp(wm->match,windowName)) ;
    return (!strcmp(wm->match,className)) ;
}

/************************************************
 * Checks if this window is saved as a tricky window that needs the 
 * "move technique" to be hidden.
 */
static BOOL checkIfTricky(char *className, char *windowName, RECT *pos)
{
    int ii, ret=0 ; 
    vwLogVerbose((vwLogFile, "checkIfTricky [%s] [%s] %d %d\n",className,windowName,(int)pos->left,(int)pos->top)) ;
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
static BOOL checkIfSticky(char *className, char *windowName)
{
    int i=curSticky ;
    
    vwLogVerbose((vwLogFile,"checkIfSticky [%s] [%s]\n",className,windowName)) ;
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
static int checkIfAssigned(char *className, char *windowName)
{
    int i;
    vwLogVerbose((vwLogFile,"checkIfAssigned [%s] [%s]\n",className,windowName)) ;
    for(i = 0; i < curAssigned; ++i) 
    {
        if((assignedList[i].type & 0x04) == 0)
        {
            vwLogVerbose((vwLogFile, "Assign comparing [%s] [%s] with %d [%s]\n",className,windowName,(int) assignedList[i].type,assignedList[i].match)) ;
            
            if(checkIfWindowMatch(&(assignedList[i]),className,windowName)) 
            {
                if(assignOnlyFirst)
                    assignedList[i].type |= 0x04 ;
                if((assignedList[i].desk > (nDesksX * nDesksY)) && (assignedList[i].desk != vwDESK_PRIVATE1))
                    MessageBox(hWnd, "Tried to assign an application to an unavaliable desktop.\nIt will not be assigned.\nCheck desktop assignmet configuration.",vwVIRTUAWIN_NAME " Error", MB_ICONERROR); 
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
    
    vwLogBasic((vwLogFile,"setForegroundWin: %x %d (%x)\n",(int) theWin,makeTop,(int) hWnd)) ;
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
        /* don't make the foreground a hidden or non-managed or hung window */
        return ;
    
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
                    vwLogVerbose((vwLogFile, "Attached to foreground Window: %x - %x\n",(int) cwHwnd,(int) GetForegroundWindow())) ;
                }
                else
                {
                    SetForegroundWindow(hWnd) ; 
                    vwLogVerbose((vwLogFile, "VW owns foreground Window: %x - %x\n",(int) cwHwnd,(int) GetForegroundWindow())) ;
                }
            }
            else
            {
                SetForegroundWindow(hWnd) ; 
                vwLogVerbose((vwLogFile, "No foreground Window or hung: %x - %x\n",(int) cwHwnd,(int) GetForegroundWindow())) ;
            }
        }
        SetForegroundWindow(theWin) ;
        /* SetForegroundWindow can return success (non-zero) but not succeed (GetForegroundWindow != theWin)
         * Getting the foreground window right is really important because if the existing foreground window
         * is left as the foreground window but hidden (common when moving the app or desk) VW will confuse
         * it with a popup */
        cwHwnd = GetForegroundWindow() ;
        vwLogBasic((vwLogFile,"Set foreground window: %d, %x -> %x\n",(theWin == cwHwnd),(int) theWin,(int) cwHwnd)) ;
        if((cwHwnd == theWin) || (--index < 0))
            break ;
        /* A short sleep allows the rest of the system to catch up */
        vwLogVerbose((vwLogFile,"About to FG sleep\n")) ;
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
        vwLogVerbose((vwLogFile,"About to call createPropertySheet\n")) ;
        createPropertySheet(hInst,hWnd);
        vwLogVerbose((vwLogFile,"createPropertySheet returned\n")) ;
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
    vwLogBasic((vwLogFile,"Set window desk: %x %d %d (%x)\n",(int) theWin,theDesk,move,(int) activeHWnd)) ;
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
        vwLogVerbose((vwLogFile,"Looking for replacement active: %x\n",(int) activeHWnd)) ;
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
                vwLogVerbose((vwLogFile,"Setting Sticky: %x %x - %d -> %d\n",(int) winList[index].Handle,
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
            tmpHnd = FindWindow(NULL, userList[i].match);
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
                MessageBox(hWnd, "Maximum number of managed windows has been reached,\nnew windows will not be managed.\n\nPlease report this to problem.",vwVIRTUAWIN_NAME " Error", MB_ICONERROR);
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
            vwLogBasic((vwLogFile,"Got tricky window state change: %x %d (%d) %d -> %d %d\n",
                            (int) winList[idx].Handle,winList[idx].Desk,currentDesk,
                            winList[idx].Visible,(int)pos.left,(int)pos.top)) ;
            winList[idx].State = 2 ;
        }
    }
    else if(style & WS_VISIBLE)
    {
        /* Something has made this window visible so make it belong to this desktop, update the list entry,
         * also check the location as the app may have only made the window visible */
        vwLogBasic((vwLogFile,"Got window state change: %x %d (%d) %d -> %d\n",
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
    char cname[vwCLASSNAME_MAX], wname[vwWINDOWNAME_MAX] ;
    int inWin, newDesk=0, i, j, hungCount=0 ;
    
    vwLogVerbose((vwLogFile,"Updating winList, nWin %d, fgw %x tpw %x\n",nWin,
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
            strcpy(wname,"<None>");
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
            windowSetDesk(winList[i].Handle,j,assignImmediately) ;
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
                    vwLogBasic((vwLogFile,"Making tricky window %x owned by %x\n",
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
                    // an existing app has either unhidden an old window or popped up a new one 
                    // use the existing window's settings for this one
                    if(!winList[i].Sticky)
                        winList[i].Sticky = winList[j].Sticky ;
                    if((winList[i].Desk=winList[j].Desk) != currentDesk)
                        winList[i].State = 2 ;
                    break ;
                }
        }
    }
    if(vwLogEnabled())
    {
        for(i=inWin ; i < nWin ; ++i)
        {
            GetClassName(winList[i].Handle,cname,vwCLASSNAME_MAX);
            if(!GetWindowText(winList[i].Handle,wname,vwWINDOWNAME_MAX))
                strcpy(wname,"<None>");
            vwLogBasic((vwLogFile,"Got new window %8x %08x %08x Desk %d Stk %d Trk %d Vis %d Own %x Pos %d %d\n  Class \"%s\" Title \"%s\"\n",
                        (int) winList[i].Handle,(int)winList[i].Style,(int)winList[i].ExStyle,winList[i].Desk,winList[i].Sticky,winList[i].Tricky,
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
            vwLogBasic((vwLogFile,"Lost window %8x %d %d %d %d %x\n",(int) winList[i].Handle,
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
    vwLogVerbose((vwLogFile,"Active %8x Last %8x %x LBO %8x %x\n",(int) activeHWnd,
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
                        vwLogBasic((vwLogFile,"Got Popup - Active %8x Last %8x %x LBO %8x %x\n",(int) activeHWnd,
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
    vwLogVerbose((vwLogFile,"Updated winList, %d windows - %d hung, newDesk %d\n",nWin,hungCount,newDesk)) ;
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
    saveDisabledList(nOfModules,moduleList);
    unloadModules();
    unRegisterAllKeys();
    showAll(0);                            // gather all windows quickly
    Shell_NotifyIcon(NIM_DELETE, &nIconD); // This removes the icon
    showAll(vwSHWIN_TRYHARD);            // try harder to gather remaining ones before exiting
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
                sprintf(nIconD.szTip,"%d window%s not responding",hungCount,(hungCount==1) ? "":"s") ;
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
        strcpy(nIconD.szTip,vwVIRTUAWIN_NAME_VERSION);	// Restore Tooltip
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
    int notHung, x, y, b ;
    
    if(newDesk == currentDesk)
        // Nothing to do
        return 0;
    
    if(vwLogEnabled())
    {
#ifdef vwLOG_TIMING
        SYSTEMTIME stime;
    
        GetLocalTime (&stime);
        vwLogPrint((vwLogFile, "[%04d-%02d-%02d %02d:%02d:%02d] Step Desk Start: %d -> %d\n",
                    stime.wYear, stime.wMonth, stime.wDay, stime.wHour,
                    stime.wMinute, stime.wSecond, currentDesk, newDesk)) ;
#else
        vwLogPrint((vwLogFile,"Step Desk Start: %d -> %d\n",currentDesk,newDesk)) ;
#endif
    }
    
    lockMutex();
    winListUpdate() ;
    
    if(isDragging)
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
                if((winList[x].Desk == currentDesk) && ((zox=winList[x].ZOrder[currentDesk]) >= zob) &&
                   ((zox < zoy) || ((zox == zoy) && (x < y))))
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
            vwLogVerbose((vwLogFile, "ZOrder: %d %d %x -> %d %d %x (%d)\n",y,zoy,zoh,b,zob,winList[b].Handle,notHung)) ;
            if(notHung)
            {
                SetWindowPos(winList[b].Handle,zoh,0,0,0,0,SWP_DEFERERASE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOSENDCHANGING|SWP_NOMOVE) ;
                zoh = winList[b].Handle ;
            }
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
        for (x = 0; x < nWin ; ++x)
        {
            if(winList[x].Sticky || (!minSwitch && (winList[x].Style & WS_MINIMIZE)))
                winList[x].Desk = currentDesk ;
            if(winList[x].Desk == currentDesk)
            {
                // Show these windows
                if(!winList[x].Visible)
                    showHideWindow(&winList[x],0,vwVISIBLE_YES) ;
                if(((winList[x].Style & (WS_MINIMIZE|WS_VISIBLE)) == WS_VISIBLE) && 
                   (winList[x].ZOrder[currentDesk] > activeZOrder))
                {
                    activeHWnd = winList[x].Handle;
                    activeZOrder = winList[x].ZOrder[currentDesk];
                }
            }
            else if(winList[x].Visible)
                // Hide these, not iconic or "Switch minimized" enabled
                showHideWindow(&winList[x],0,vwVISIBLE_NO) ;
        }
        if(preserveZOrder)
        {
            y = nWin ;
            zoy = 0xffffffff ;
            zoh = HWND_NOTOPMOST ;
            for(;;)
            {
                b = -1 ;
                zob = 0 ;
                for(x = 0; x < nWin ; ++x)
                {
                    if((winList[x].Desk == currentDesk) && ((zox=winList[x].ZOrder[currentDesk]) >= zob) &&
                       ((zox < zoy) || ((zox == zoy) && (x < y))))
                    {
                        zob = zox ;
                        b = x ;
                    }
                }
                if(b < 0)
                    break ;
                // Show these windows
                notHung = windowIsNotHung(winList[b].Handle,100) ;
                vwLogVerbose((vwLogFile, "TBZOrder: %d %d %x -> %d %d %x (%d)\n",y,zoy,zoh,b,zob,winList[b].Handle,notHung)) ;
                if(notHung)
                {
                    SetWindowPos(winList[b].Handle,zoh,0,0,0,0,SWP_DEFERERASE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOSENDCHANGING|SWP_NOMOVE) ;
                    zoh = winList[b].Handle ;
                }
                y = b ;
                zoy = zob ;
            }
        }
    }
    if(releaseFocus)     // Release focus maybe?
        activeHWnd = NULL ;
    vwLogBasic((vwLogFile, "Active found: %x (%d,%d,%d)\n",(int) activeHWnd,(int)activeZOrder,releaseFocus,isDragging)) ;
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
    if(vwLogEnabled())
    {
#ifdef vwLOG_TIMING
        SYSTEMTIME stime;
    
        GetLocalTime (&stime);
        vwLogPrint((vwLogFile, "[%04d-%02d-%02d %02d:%02d:%02d] Step Desk End (%x)\n",
                    stime.wYear, stime.wMonth, stime.wDay, stime.wHour,
                    stime.wMinute, stime.wSecond,(int)GetForegroundWindow())) ;
#else
        vwLogPrint((vwLogFile,"Step Desk End (%x)\n",(int)GetForegroundWindow())) ;
#endif
    }
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
 * Converts an icon to a bitmap representation
 */
static HBITMAP createBitmapIcon(HICON anIcon)
{
    HDC aHDC = GetDC(hWnd);
    HDC aCHDC = CreateCompatibleDC(aHDC);
    HBITMAP hBMP = CreateCompatibleBitmap(aHDC, 13, 13);
    RECT rect ;
    hBMP = SelectObject(aCHDC, hBMP);    
    rect.top = rect.left = 0 ;
    rect.bottom = rect.right = 13 ;
    FillRect(aCHDC,&rect,GetSysColorBrush(COLOR_WINDOW));
    if(DrawIconEx(aCHDC, -1, -1, anIcon, 15, 15, 0, NULL, DI_NORMAL) == 0)
    {
        DeleteObject(hBMP);
        hBMP = 0;
    }
    else
    {
        hBMP = SelectObject(aCHDC, hBMP);
        iconReferenceVector[vectorPosition] = hBMP;
        vectorPosition++;
    }
    DeleteDC(aCHDC);
    ReleaseDC(hWnd, aHDC);
    
    return hBMP;
}

/*************************************************
 * Deletes bitmaps allocated when displaying window menu
 */
static void clearBitmapVector(void)
{
    while(vectorPosition > 0)
    {
        vectorPosition--;
        DeleteObject(iconReferenceVector[vectorPosition]);
    }
}

/*************************************************
 * createSortedWinList_cos creates a popup menu for the window-hotkey
 * which displays all windows in one list vertically seperated by a line.
 * first column is sticky, second is direct access and third is assign.
 * so you don't have to step through submenus.
 * 
 * Author: Christian Storm aka cosmic (Christian.Storm@Informatik.Uni-Oldenburg.de)
 */
#define vwPMENU_STICKY 0x100
#define vwPMENU_ACCESS 0x200
#define vwPMENU_ASSIGN 0x400
#define vwPMENU_MASK   0xf00

static HMENU createSortedWinList_cos(void)
{
    HMENU hMenu;         // menu bar handle
    char title[48];
    MenuItem *items[MAXWIN], *item;
    char buff[MAX_PATH];
    int i,x,y,c,doAssignMenu=0;
    BOOL useTitle;    // Only use title if we have more than one menu
    
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
            sprintf(title, "%d - %s", winList[c].Desk, buff);
#else
            sprintf(title, "%d - %s (%x)", winList[c].Desk, buff,(int) winList[c].Handle);
#endif
            if(((item = malloc(sizeof(MenuItem))) == NULL) ||
               ((item->name = strdup(title)) == NULL))
            {
                while(--i >= 0)
                {
                    free(items[i]->name) ;
                    free(items[i]) ;
                }
                return NULL ;
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
            if(hSmallIcon != NULL)
                item->icon = createBitmapIcon(hSmallIcon);
            else
                item->icon = 0 ;
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
        return NULL ;
    }
    
    if((stickyMenu + directMenu + assignMenu) == 1)
        // Don't show titles if only one menu is enabled
        useTitle = FALSE;
    else
        useTitle = TRUE;
    
    hMenu = CreatePopupMenu();
    
    // sorting using bubble sort
    for(x = 0; x < i; x++ )
    {
        for(y = 0; y<i; y++)
        {
            if(strcmp(items[x]->name, items[y]->name) < 0 )
            {
                item = items [x];
                items[x] = items[y];
                items[y] = item;
            }
        }
    }
    
    if(stickyMenu)
    {
        if(useTitle)
        {
            AppendMenu(hMenu, MF_STRING | MF_DISABLED, 0, "Sticky" );
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
        }
        for(x=0,c=0 ; x < i ; x++)
        {
            if((c != 0) && (c != items[x]->desk))
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
            c = items[x]->desk;
            AppendMenu( hMenu, MF_STRING | (items[x]->sticky ? MF_CHECKED: 0),
                        vwPMENU_STICKY | (items[x]->id), items[x]->name );
            if(items[x]->icon != 0)
                SetMenuItemBitmaps(hMenu, vwPMENU_STICKY | (items[x]->id), MF_BYCOMMAND, items[x]->icon, 0);
        }
    }
    
    if(directMenu)
    {
        if(useTitle)
        {
            AppendMenu(hMenu, (stickyMenu) ? (MF_STRING | MF_DISABLED | MF_MENUBARBREAK):(MF_STRING | MF_DISABLED), 0, "Access" );
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
        }
        for(x=0,c=0 ; x < i ; x++)
        {
            if((c != 0) && (c != items[x]->desk))
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
            c = items[x]->desk;
            AppendMenu( hMenu, MF_STRING, vwPMENU_ACCESS | (items[x]->id), items[x]->name );
            if(items[x]->icon != 0)
                SetMenuItemBitmaps(hMenu, vwPMENU_ACCESS | (items[x]->id), MF_BYCOMMAND, items[x]->icon, 0);
        }
    }
    
    if(doAssignMenu)
    {
        if(useTitle)
        {
            AppendMenu(hMenu, (stickyMenu || directMenu) ? (MF_STRING | MF_DISABLED | MF_MENUBARBREAK):(MF_STRING | MF_DISABLED), 0, "Assign" );
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
        }
        for(x=0,c=0 ; x < i ; x++)
        {
            //sticky windows can't be assigned cause they're sticky :-) so leave them.out..
            //cannot assign to current Desktop
            y = (!items[x]->sticky) && (items[x]->desk != currentDesk) ;
            if(y || useTitle)
            {
                if((c != 0) && (c != items[x]->desk))
                    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                c = items [x]->desk;
                if(y)
                {
                    AppendMenu( hMenu, MF_STRING, (vwPMENU_ASSIGN | (items[x]->id)), items[x]->name );
                    if(items[x]->icon != 0)
                        SetMenuItemBitmaps(hMenu, (vwPMENU_ASSIGN | (items[x]->id)), MF_BYCOMMAND, items[x]->icon, 0);
                }
                else
                    AppendMenu( hMenu, MF_DISABLED, 0, "") ;
            }
        }
    }
    
    // destroy the generated window-list
    for (x=0; x<i; x++)
    {
        free(items[x]->name) ;
        free(items[x]) ;
    }
    
    return hMenu;
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
        if(!windowIsNotHung(aWindow->Handle,(shwFlags & vwSHWIN_TRYHARD) ? 10000:100))
        {
            vwLogVerbose((vwLogFile,"showHideWindow %8x %d %d %x %x %d %d %d - HUNG\n",(int) aWindow->Handle,shwFlags,show,(int)aWindow->Style,(int)aWindow->ExStyle,aWindow->Visible,aWindow->Desk,currentDesk)) ;
            return FALSE ;
        }
        GetWindowRect( aWindow->Handle, &pos );
        vwLogVerbose((vwLogFile,"showHideWindow %8x %d %d %x %x %d %d %d - %d %d\n",(int) aWindow->Handle,shwFlags,show,(int)aWindow->Style,(int)aWindow->ExStyle,aWindow->Visible,aWindow->Desk,currentDesk,(int) pos.left,(int) pos.top)) ;
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
            ShowWindow(aWindow->Handle,(show) ? SW_SHOWNA:SW_HIDE) ;
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
        strcpy(nIconD.szTip,vwVIRTUAWIN_NAME " - Disabled"); //Tooltip
        setIcon(0);
        unRegisterAllKeys();
        KillTimer(hWnd, 0x29a);
        enabled = FALSE;
    }
    else
    {   // Enable VirtuaWin
        strcpy(nIconD.szTip,vwVIRTUAWIN_NAME_VERSION);	// Tooltip
        setIcon(currentDesk);
        registerAllKeys();
        SetTimer(hWnd, 0x29a, 250, monitorTimerProc);
        enabled = TRUE;
    }
}

/************************************************
 * Assigns a window to the specified desktop
 * Used by the module message VW_ASSIGNWIN
 */
int assignWindow(HWND theWin, int theDesk, BOOL force)
{
    int ret, change, idx ;
    unsigned char sticky=0 ;
    
    vwLogBasic((vwLogFile,"Assign window: %x %d %d\n",(int) theWin,theDesk,force)) ;
    change = (theDesk < 0) ;
    if(change)
        theDesk = 0 - theDesk ;
        
    if((theWin == NULL) || (theWin == hWnd) ||
       (((theDesk > (nDesksY * nDesksX)) || (theDesk < 1)) && !force))
        return 0 ; // Invalid window or desk
    
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
    
    vwLogBasic((vwLogFile,"Access window: %x %d\n",(int) theWin,method)) ;
    
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
    vwLogBasic((vwLogFile,"Set sticky window: %x %d\n",(int) theWin,state)) ;
    if(theWin == NULL)
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
    
    vwLogBasic((vwLogFile,"Dismissing window: %x\n",(int) theWin)) ;
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

/*************************************************
 * Pops up and handles the window list menu
 */
static void winListPopupMenu(HWND aHWnd)
{
    HMENU hpopup;
    POINT pt;
    HWND hwnd ;
    int retItem, id, ii;
    
    if((hpopup = createSortedWinList_cos()) == NULL)
        return ;
    GetCursorPos(&pt);
    SetForegroundWindow(aHWnd);
    
    retItem = TrackPopupMenu(hpopup, TPM_RETURNCMD |  // Return menu code
                             TPM_LEFTBUTTON, (pt.x-2), (pt.y-2), // screen coordinates
                             0, aHWnd, NULL);
    
    if(retItem)
    {
        id = retItem & ~vwPMENU_MASK ;
        ii = nWin ;
        while(--ii >= 0)
            if(winList[ii].menuId == id)
                break ;
        if(ii >= 0)
        {
            hwnd = winList[ii].Handle ;
            vwLogVerbose((vwLogFile,"Menu select %x %d %x\n",retItem,ii,(int) hwnd)) ;
            if(retItem & vwPMENU_STICKY)
                // Sticky toggle
                setSticky(hwnd,-1) ;
            else if(retItem & vwPMENU_ACCESS)
            {   // window access
                gotoDesk(winList[ii].Desk,FALSE);
                setForegroundWin(hwnd,0);
            } 
            else
            {   // Assign to this desktop
                assignWindow(hwnd,currentDesk,TRUE) ;
                setForegroundWin(hwnd,0) ;
            }
        }
    }
    
    PostMessage(aHWnd, 0, 0, 0);  // see above
    DestroyMenu(hpopup);       // Delete loaded menu and reclaim its resources
    clearBitmapVector();
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
        // Try to avoid switching if we press on the taskbar
        if((LOWORD(lParam) && (GetForegroundWindow() == FindWindow("Shell_traywnd", ""))))
            goto skipMouseWarp; // if so, skip whole sequence
        // Is virtuawin enabled
        if(enabled)
        {
            // Are we using a mouse key
            if(useMouseKey && !HIWORD(GetAsyncKeyState(MOUSEKEY)))
                // If key not pressed skip whole sequence
                goto skipMouseWarp;
            
            // Suspend mouse thread during message processing, 
            // otherwise we might step over several desktops
            SuspendThread(mouseThread); 
            GetCursorPos(&pt);
            
            switch HIWORD(lParam)
            {
            case VW_MOUSELEFT:
                warpMultiplier++;
                if((warpMultiplier >= configMultiplier))
                {
                    isDragging = LOWORD(lParam);
                    if(stepLeft() != 0)
                    {
                        if(noMouseWrap)
                            SetCursorPos(pt.x + warpLength, pt.y);
                        else
                            SetCursorPos(screenRight-warpLength, pt.y);
                    }
                    isDragging = FALSE;
                    warpMultiplier = 0;
                }
                ResumeThread(mouseThread);
                break;
                
            case VW_MOUSERIGHT:
                warpMultiplier++;
                if((warpMultiplier >= configMultiplier))
                {
                    isDragging = LOWORD(lParam);
                    if(stepRight() != 0)
                    {
                        if(noMouseWrap)
                            SetCursorPos(pt.x - warpLength, pt.y);
                        else
                            SetCursorPos(screenLeft+warpLength, pt.y);
                    }
                    isDragging = FALSE;
                    warpMultiplier = 0;
                }
                ResumeThread(mouseThread);
                break;
                
            case VW_MOUSEUP:
                warpMultiplier++;
                if((warpMultiplier >= configMultiplier))
                {
                    int switchVal;
                    isDragging = LOWORD(lParam);
                    if(invertY)
                        switchVal = stepDown();
                    else
                        switchVal = stepUp();
                    if(switchVal != 0)
                    {
                        if(noMouseWrap)
                            SetCursorPos(pt.x, pt.y + warpLength);
                        else
                            SetCursorPos(pt.x, screenBottom-warpLength);
                    }
                    isDragging = FALSE;
                    warpMultiplier = 0;
                }
                ResumeThread(mouseThread);
                break;
                
            case VW_MOUSEDOWN:
                warpMultiplier++;
                if((warpMultiplier >= configMultiplier))
                {
                    int switchVal;
                    isDragging = LOWORD(lParam);
                    if(invertY)
                        switchVal = stepUp();
                    else
                        switchVal = stepDown();
                    if(switchVal != 0)
                    {
                        if(noMouseWrap)
                            SetCursorPos(pt.x, pt.y - warpLength);
                        else
                            SetCursorPos(pt.x, screenTop+warpLength);
                    }
                    isDragging = FALSE;
                    warpMultiplier = 0;
                }
                ResumeThread(mouseThread);
                break;
                
            case VW_MOUSERESET:
                warpMultiplier = 0;
            }
            ResumeThread(mouseThread);
        }
skipMouseWarp:  // goto label for skipping mouse stuff
        
        return TRUE;
        
    case WM_HOTKEY:				// A hot key was pressed
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
            setSticky(GetForegroundWindow(),-1);
        else if(wParam == dismissKey)
            windowDismiss(GetForegroundWindow());
        else if(wParam == vwMenu)
        {
            if(enabled)
                winListPopupMenu(aHWnd) ;
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
        switch (wParam) {
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
        return assignWindow((HWND)wParam,(int)lParam,FALSE);
        
    case VW_ACCESSWIN:
        return accessWindow((HWND)wParam,(int)lParam,FALSE);
        
    case VW_SETSTICKY:
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
            COPYDATASTRUCT cds;
            char *ss = (message == VW_INSTALLPATH) ? VirtuaWinPath:UserAppPath ;
            cds.dwData = 0 - message ;
            cds.cbData = strlen(ss) + 1;
            cds.lpData = (void*)ss;
            sendModuleMessage(WM_COPYDATA, (WPARAM) aHWnd, (LPARAM)&cds); 
            return TRUE;
        }

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
                winListPopupMenu(aHWnd) ;
            break;
        
        case WM_LBUTTONDBLCLK:             // double click on icon
            showSetup();
            break;
            
        case WM_MBUTTONUP:		   // Move to the next desktop
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
                HMENU hmenu, hpopup;
                UINT nID ;
                GetCursorPos(&pt);
                hmenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
                
                hpopup = GetSubMenu(hmenu, 0);
                
                nID = GetMenuItemID(hpopup, 3); // Get the Disable item
                if(enabled) // Change the text depending on state
                    ModifyMenu(hpopup, nID, MF_BYCOMMAND, nID, "Disable");
                else
                    ModifyMenu(hpopup, nID, MF_BYCOMMAND, nID, "Enable");
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
                DestroyMenu(hpopup);  // Delete loaded menu and reclaim its resources
                DestroyMenu(hmenu);		
            }
            break;
        }
        return TRUE;
        
    case WM_DISPLAYCHANGE:
        getScreenSize();
        return TRUE;
    case WM_SETTINGCHANGE:
        getTaskbarLocation();
        return TRUE;
        
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
    char *classname = vwVIRTUAWIN_NAME "MainClass";
    hInst = hInstance;

    /* Only one instance may be started */
    hMutex = CreateMutex(NULL, FALSE, vwVIRTUAWIN_NAME "PreventSecond");
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
        MessageBox(hWnd, "Failed to register class!",vwVIRTUAWIN_NAME " Error", MB_ICONWARNING);
        return 0 ;
    }
    
    readConfig();	// Read the config file
    if(vwLogFlag)
    {
        char logFname[MAX_PATH] ;
        GetFilename(vwCONFIG,1,logFname) ;
        strcpy(logFname+strlen(logFname)-3,"log") ;
        vwLogFile = fopen(logFname,"w+") ;
        vwLogBasic((vwLogFile,vwVIRTUAWIN_NAME_VERSION "\n")) ;
    }
    
    
    /* set the window to give focus to when releasing focus on switch also used to refresh */
    desktopHWnd = GetDesktopWindow();
    getScreenSize();
    getTaskbarLocation(); // This is dependent on the config
    
    // Fix some things for the alternate hide method
    RM_Shellhook = RegisterWindowMessage("SHELLHOOK");
    goGetTheTaskbarHandle();
    
    loadIcons();
    
    // Load tricky windows, must be done before the crashRecovery checks
    if(trickyWindows)
        curTricky = loadTrickyList(trickyList) ;
    
    /* Create window. Note that WS_VISIBLE is not used, and window is never shown. */
    if((hWnd = CreateWindowEx(0, classname, classname, WS_POPUP, CW_USEDEFAULT, 0,
                              CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL)) == NULL)
    {
        MessageBox(hWnd, "Failed to create window!",vwVIRTUAWIN_NAME " Error", MB_ICONWARNING);
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
    
    strcpy(nIconD.szTip,vwVIRTUAWIN_NAME_VERSION);		// Tooltip
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
    if(!mouseEnable) // Suspend the thread if no mouse support
        enableMouse(FALSE);
    //SuspendThread(mouseThread);
    
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
