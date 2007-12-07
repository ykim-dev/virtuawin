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

/* Get the list of hotkey commands */
#define VW_COMMAND(a, b, c, d) a = b ,
enum {
#include "vwCommands.def"
} ;
#undef  VW_COMMAND

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

/* define some constants that are often missing in early compilers */
#ifndef WS_EX_NOACTIVATE
#define WS_EX_NOACTIVATE 0x8000000
#endif
/* The virtual screen size system matrix values were only added for WINVER >= 0x0500 (Win2k) */
#ifndef SM_CMONITORS
#define SM_XVIRTUALSCREEN       76
#define SM_YVIRTUALSCREEN       77
#define SM_CXVIRTUALSCREEN      78
#define SM_CYVIRTUALSCREEN      79
#define SM_CMONITORS            80
#endif

#define calculateDesk(x,y) (((y) * nDesksX) - (nDesksX - (x)))
#define windowIsNotHung(inhHWnd,waitTime) (SendMessageTimeout(inhHWnd,(int)NULL,0,0,SMTO_ABORTIFHUNG|SMTO_BLOCK,waitTime,NULL))

// Variables
HWND hWnd;                                   // handle to VirtuaWin
HANDLE hMutex;
FILE *vwLogFile ;
vwUByte vwEnabled=TRUE;	                     // if VirtuaWin enabled or not

int taskbarEdge;
int desktopWorkArea[2][4] ;

windowType winList[vwWINDOW_MAX];            // list for holding windows
vwHotkey   hotkeyList[vwHOTKEY_MAX];         // list for holding hotkeys
moduleType moduleList[MAXMODULES];           // list that holds modules
disModules disabledModules[MAXMODULES*2];    // list with disabled modules

vwWindowMatch *userList;                     // list for holding user added applications
vwWindowMatch *stickyList;                   // list with saved sticky windows
vwWindowMatch *trickyList;                   // list with saved tricky windows
vwWindowMatch *assignedList;                 // list with all windows that have a predefined desktop

UINT RM_Shellhook;

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
vwUByte setupOpen;         

// vector holding icon handles for the systray
NOTIFYICONDATA nIconD;
HICON icons[vwDESKTOP_SIZE];    // 0=disabled, 1,2..=normal desks
TCHAR *desktopName[vwDESKTOP_SIZE];
unsigned char desktopUsed[vwDESKTOP_SIZE];

#define MENU_X_PADDING 1
#define MENU_Y_PADDING 1
#define ICON_PADDING 2
#define ICON_SIZE 16
HICON checkIcon;

// Config parameters, see ConfigParameters.h for descriptions
int hotkeyCount = 0;
int hotkeyRegistered = 0;
int moduleCount = 0;  
int windowCount = 0;        
int currentDeskX = 1;
int currentDeskY = 1;
int currentDesk = 1; 
int nDesks = 4;     
int nDesksX = 2;     
int nDesksY = 2;     
vwUByte mouseKnock = 2;
vwUByte preserveZOrder = 0;      
vwUByte hiddenWindowAct = 1;
vwUByte vwLogFlag = 0 ;
vwUByte releaseFocus = FALSE;	
vwUByte minSwitch = TRUE;		
vwUByte refreshOnWarp = FALSE;     
vwUByte deskWrap = FALSE;          
vwUByte invertY = FALSE;           
vwUByte winListContent = (vwWINLIST_ACCESS | vwWINLIST_ASSIGN | vwWINLIST_SHOW | vwWINLIST_STICKY) ;
vwUByte winListCompact = 0;
;
vwUByte useDeskAssignment = FALSE;
vwUByte assignImmediately = 1;
vwUByte displayTaskbarIcon = TRUE;
vwUByte taskbarIconShown = 0;
vwUByte noTaskbarCheck = FALSE;
vwUByte trickyWindows = TRUE;
vwUByte taskbarFixRequired = FALSE;

HANDLE mouseThread;                          // Handle to the mouse thread
vwUByte mouseEnabled=TRUE;                   // Status of the mouse thread, always running at startup 
vwUByte mouseEnable = 0;                     // Required mouse support
vwUByte isDragging;	                     // if we are currently dragging a window
vwUByte mouseWarp = FALSE;
vwUByte mouseModifierUsed = FALSE;
vwUByte mouseModifier ;
int mouseDelay = 20;
int mouseJumpLength = 60;

int curDisabledMod = 0; 
vwUInt vwZOrder=0 ;
vwUInt timerCounter=0 ;

/* desk image generation variables */
int         deskImageCount=-1 ;
HBITMAP     deskImageBitmap=NULL ;
BITMAPINFO  deskImageInfo ;
void       *deskImageData=NULL ;

enum {
    OSVERSION_UNKNOWN=0,
    OSVERSION_31,
    OSVERSION_9X,
    OSVERSION_NT,
    OSVERSION_2000,
    OSVERSION_XP
} ;
int osVersion ;
    
#define vwSHWIN_TRYHARD   0x01
static int showHideWindow(windowType *aWindow, int shwFlags, unsigned char show);
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
static void
vwMutexLock(void)
{
    if ((hMutex != (HANDLE) 0) && (WaitForSingleObject(hMutex,0) == WAIT_TIMEOUT))
        WaitForSingleObject(hMutex,INFINITE);
}

/************************************************
 * Releases the window list protection
 */
static void
vwMutexRelease(void)
{
    if (hMutex != (HANDLE) 0)
        ReleaseMutex(hMutex);
}

/************************************************
 * Returns a bit mask of currently pressed modifier keys
 */
static int
vwKeyboardTestModifier(vwUByte modif)
{
    if((modif & vwHOTKEY_ALT) && !HIWORD(GetAsyncKeyState(VK_MENU)))
        return FALSE ;
    if((modif & vwHOTKEY_CONTROL) && !HIWORD(GetAsyncKeyState(VK_CONTROL)))
        return FALSE ;
    if((modif & vwHOTKEY_SHIFT) && !HIWORD(GetAsyncKeyState(VK_SHIFT)))
        return FALSE ;
    return TRUE ;
}

/*************************************************
 * Checks if mouse button pressed on title bar (i.e. dragging window)
 */
static unsigned char
checkMouseState(int force)
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
                lParam = (((int)(short) pt.y) << 16) | (0x0ffff & ((int)(short) pt.x)) ;
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
DWORD WINAPI
vwMouseProc(LPVOID lpParameter)
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
                    if((ii=(mouseJumpLength >> 1)) < 10)
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
           (!mouseModifierUsed || vwKeyboardTestModifier(mouseModifier)))
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
                    state[ii] = (pos[ii] >= mouseJumpLength) ;
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
                    else if(pos[ii] >= mouseJumpLength)
                    {
                        if(newState == 0)
                            newState = 1 ;
                    }
                    else if(pos[ii] <= 0)
                    {
                        if((mouseKnock & 1) && ((newState == 0) || ((newState == 1) && (mouseKnock & 2))))
                            newState = 2 ;
                        else if((newState == 3) || (newState <= 1))
                            newState = 4 ;
                    }
                    else if((newState == 2) && (pos[ii] >= (mouseJumpLength >> 2)))
                        newState = 3 ;
                    if(newState != state[ii])
                    {
                        newPos = (ii & 0x01) ? pt.x:pt.y ;
                        if((state[ii] > 1) && (abs(statePos[ii]-newPos) > mouseJumpLength))
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
void enableMouse(int turnOn)
{
    // Try to turn on thread if not already running
    if(turnOn && !mouseEnabled)
    {
        ResumeThread(mouseThread);
        mouseEnabled = TRUE;
    }
    // Try to turn of thread if already not stopped
    else if(!turnOn && mouseEnabled)
    {
        SuspendThread(mouseThread);
        mouseEnabled = FALSE;
    }
}

/*************************************************
 * Sets the icon in the systray and updates the currentDesk variable
 */
static void
vwIconSet(int deskNumber, int hungCount)
{
    if(displayTaskbarIcon && ((taskbarIconShown & 0x02) == 0))
    {
        int ll ;
        if(hungCount < 0)
        {
            nIconD.hIcon = NULL ;
            hungCount = 0 - hungCount ;
        }
        else
            nIconD.hIcon = icons[deskNumber];
        ll = 0 ;
        if(hungCount)
            ll = _stprintf(nIconD.szTip,_T("%d window%s not responding\n"),hungCount,(hungCount==1) ? _T(""):_T("s")) ;
        if(desktopName[deskNumber] != NULL)
        {
            _tcsncpy(nIconD.szTip+ll,desktopName[deskNumber],64-ll) ;
            nIconD.szTip[63] = '\0' ;
        }
        else
            _stprintf(nIconD.szTip+ll,_T("Desktop %d"),deskNumber) ;
        if(taskbarIconShown & 0x01)
            Shell_NotifyIcon(NIM_MODIFY, &nIconD);
        else
        {
            // This adds the icon, try up to 3 times as systray process may not have started
            for(ll = 3 ;;)
            {
                if(Shell_NotifyIcon(NIM_ADD, &nIconD))
                {
                    taskbarIconShown |= 0x01 ;
                    break ;
                }
                if(--ll <= 0)
                    break ;
                Sleep(2000) ;
            }
        }
    }
    else if(taskbarIconShown & 0x01)
    {
        Shell_NotifyIcon(NIM_DELETE,&nIconD) ;
        taskbarIconShown &= ~0x01 ;
    }
}

/************************ *************************
 * Loads the icons for the systray according to the current setup
 */
void
vwIconLoad(void)
{
    int xIcon = GetSystemMetrics(SM_CXSMICON);
    int yIcon = GetSystemMetrics(SM_CYSMICON);
    int ii, iconId, iconCount ;
    TCHAR buff[16], *ss ;
    
    /* must setup desktopUsed array first, this is used elsewhere */
    memset(desktopUsed+1,1,nDesks) ;
    memset(desktopUsed+nDesks+1,0,vwDESKTOP_SIZE-nDesks-1) ;
    ii = hotkeyCount ;
    while(--ii >= 0)
        if((hotkeyList[ii].command == vwCMD_NAV_MOVE_DESKTOP) &&
           (hotkeyList[ii].desk < vwDESKTOP_SIZE))
            desktopUsed[hotkeyList[ii].desk] = 1 ;
    
    if(nDesksY != 2 || nDesksX != 2) // if 2 by 2 mode
    {
        iconId = IDI_ST_0 ;
        iconCount = 9 ;
    }
    else
    {
        if(osVersion > OSVERSION_2000)
            iconId = IDI_ST_DIS_2 ;
        else
            iconId = IDI_ST_DIS_1 ;
        iconCount = 4 ;
    }
    _tcscpy(buff,_T("icons/")) ;
    for(ii = 0 ; ii<vwDESKTOP_SIZE ; ii++)
    {
        icons[ii] = NULL ;
        if((desktopUsed[ii] != 0) || (ii == 0))
        {
            /* Try to load user defined icons, otherwise use built in icon or disable icon */
            ss = buff+6 ;
            if(ii > 9)
            {
                *ss++ = (ii/10)+'0' ;
                *ss++ = (ii%10)+'0' ;
            }
            else
                *ss++ = ii+'0' ;
            _tcscpy(ss,_T(".ico")) ;
            if(((icons[ii] = (HICON) LoadImage(hInst, buff, IMAGE_ICON, xIcon, yIcon, LR_LOADFROMFILE)) == NULL) &&
               ((ii > iconCount) ||
                ((icons[ii] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(iconId+ii), IMAGE_ICON, xIcon, yIcon, 0)) == NULL)) &&
               ((icons[ii] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_VIRTUAWIN), IMAGE_ICON, xIcon, yIcon, 0)) == NULL))
                icons[ii] = icons[0] ;
        }
    }
    // Load checkmark icon for sticky
    checkIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_CHECK));
    vwIconSet(currentDesk,0) ;
    // if on win9x the tricky windows need to be continually hidden
    taskbarFixRequired = ((osVersion <= OSVERSION_9X) && (taskHWnd != NULL)) ;
}


/************************************************
 * Registering all hotkeys
 */
void
vwHotkeyRegister(void)
{
    if(!hotkeyRegistered)
    {
        TCHAR buff[64] ;
        int ii ;
        hotkeyRegistered = hotkeyCount ;
        
        for(ii=0 ; ii<hotkeyCount ; ii++)
        {
            if(hotkeyList[ii].atom == 0)
            {
                _stprintf(buff,"vwAtom%d",ii) ;
                hotkeyList[ii].atom = GlobalAddAtom(buff);
            }
            if(hotkeyList[ii].atom == 0)
                MessageBox(hWnd,_T("Failed to create global atom"),vwVIRTUAWIN_NAME _T(" Error"), MB_ICONWARNING);
            else if(RegisterHotKey(hWnd,hotkeyList[ii].atom,(hotkeyList[ii].modifier & ~vwHOTKEY_EXT),hotkeyList[ii].key) == FALSE)
            {
                _stprintf(buff,"Failed to register hotkey %d, check hotkeys.",ii+1) ;
                MessageBox(hWnd,buff,vwVIRTUAWIN_NAME _T(" Error"), MB_ICONWARNING);
            }
        }
    }
}

/************************************************
 * Unregistering all hotkeys
 */
void
vwHotkeyUnregister(void)
{
    if(hotkeyRegistered)
    {
        int ii ;
        ii = hotkeyRegistered ;
        hotkeyRegistered = 0 ;
        while(--ii >= 0)
            if(hotkeyList[ii].atom)
                UnregisterHotKey(hWnd,hotkeyList[ii].atom) ;
    }
}

/************************************************
 * Get screen width and height and store values in
 * global variables
 */
static void
getScreenSize(void)
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
 * Grabs and stores the workarea of the screen
 */
void
getWorkArea(void)
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
 * Tries to locate the handle to the taskbar
 */
static void
goGetTheTaskbarHandle(void)
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
    
    if(createDefault || (deskNo > nDesks))
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
        count = nDesks ;
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
    int index = windowCount ;
    while(--index >= 0)
        if(winList[index].Handle == hwnd)
            break ;
    return index ;
}

static int
checkIfWindowMatch(vwWindowMatch *wm, TCHAR *className, TCHAR *windowName)
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
static int
checkIfTricky(TCHAR *className, TCHAR *windowName, RECT *pos)
{
    int ret=0 ; 
    vwLogVerbose((_T("checkIfTricky [%s] [%s] %d %d\n"),className,windowName,(int)pos->left,(int)pos->top)) ;
    if(trickyWindows)
    {
        vwWindowMatch *wm=trickyList ;
        while(wm != NULL)
        {
            if(checkIfWindowMatch(wm,className,windowName)) 
            {
                ret = vwTRICKY_WINDOW ;
                break ;
            }
            wm = wm->next ;
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
static int
checkIfSticky(TCHAR *className, TCHAR *windowName)
{
    vwWindowMatch *wm=stickyList ;
    
    vwLogVerbose((_T("checkIfSticky [%s] [%s]\n"),className,windowName)) ;
    while(wm != NULL)
    {
        if(checkIfWindowMatch(wm,className,windowName)) 
            // Typically user windows will loose their stickiness if
            // minimized, therefore we do not remove their name from 
            // the list as done above.
            return TRUE;
        wm = wm->next ;
    }
    return FALSE;
}

/*************************************************
 * Checks if a window is an predifined desktop to go to
 */
static int checkIfAssigned(TCHAR *className, TCHAR *windowName)
{
    vwWindowMatch *wm=assignedList ;
    
    vwLogVerbose((_T("checkIfAssigned [%s] [%s]\n"),className,windowName)) ;
    while(wm != NULL)
    {
        vwLogVerbose((_T("Assign comparing [%s] [%s] with %d [%s]\n"),className,windowName,(int) wm->type,wm->match)) ;
        
        if(checkIfWindowMatch(wm,className,windowName)) 
        {
            if((wm->desk < vwDESKTOP_SIZE) && desktopUsed[wm->desk])
                return wm->desk ; // Yes, assign
            MessageBox(hWnd,_T("Tried to assign an application to an unavaliable desktop.\nIt will not be assigned.\nCheck desktop assignment configuration."),vwVIRTUAWIN_NAME _T(" Error"), MB_ICONERROR); 
        }
        wm = wm->next ;
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
            (!windowIsNotHung(theWin,50) && (Sleep(1),!windowIsNotHung(theWin,100))))
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
static int
windowSetDesk(HWND theWin, int theDesk, vwUByte move, vwUByte setActive)
{
    HWND ownerWin, activeHWnd ;
    vwUByte show ;
    int index, ret=0 ;
    
    activeHWnd = GetForegroundWindow() ;
    vwLogBasic((_T("Set window desk: %x %d %d (%x)\n"),(int) theWin,theDesk,move,(int) activeHWnd)) ;
    do {
        ownerWin = 0 ;
        index = windowCount ;
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
                    /* if temporarily show the window on this desk */ 
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
        vwUInt activeZOrder=0 ;
        activeHWnd = NULL ;
        index = windowCount ;
        while(--index >= 0)
        {
            if(((winList[index].Desk == currentDesk) || (winList[index].Visible == vwVISIBLE_YESTEMP)) &&
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
    vwUInt zorder ;
    HWND ownerWin ;
    int index, ii, ret=0 ;
    
    do {
        ownerWin = 0 ;
        index = windowCount ;
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
                    ii = vwDESKTOP_SIZE - 1 ;
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
    vwWindowMatch *wm=userList ;
    HWND tmpHnd;
    
    while(wm != NULL)
    {
        if(wm->type & 1)
            tmpHnd = FindWindow(NULL,wm->match);
        else
            tmpHnd = FindWindow(wm->match, NULL);
        
        if((tmpHnd != NULL) && (winListFind(tmpHnd) < 0))
        {
            winList[windowCount].Handle = tmpHnd;
            winList[windowCount].Style = GetWindowLong(tmpHnd, GWL_STYLE);
            winList[windowCount].ExStyle = GetWindowLong(tmpHnd, GWL_EXSTYLE);
            windowCount++;
        }
        wm = wm->next ;
    }
}


/*************************************************
 * Callback function. Integrates all enumerated windows
 */
static int CALLBACK
enumWindowsProc(HWND hwnd, LPARAM lParam) 
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
        if(windowCount >= vwWINDOW_MAX)
        {
            static vwUByte printedError=FALSE ;
            if(!printedError)
            {
                printedError = TRUE ;
                MessageBox(hWnd,_T("Maximum number of managed windows has been reached,\nnew windows will not be managed.\n\nPlease report this to problem."),vwVIRTUAWIN_NAME _T(" Error"), MB_ICONERROR);
            }
        }
        else
        {
            idx = windowCount++;
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
    
    vwLogVerbose((_T("Updating winList, windowCount %d, fgw %x tpw %x\n"),windowCount,
                (int) GetForegroundWindow(),(int) GetTopWindow(NULL))) ;
    /* We now own the mutex. */
    i = inWin = windowCount ;
    while(--i >= 0)
        winList[i].State = 0 ;

    // Get all windows
    EnumWindows(enumWindowsProc,0);
    if(userList != NULL)
        findUserWindows();
    
    // finish the initialisation of new windows
    for(i=inWin ; i < windowCount ; ++i)
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
        winList[i].Flags = 0 ;
        winList[i].menuId = 0 ;
        winList[i].Owner = GetWindow(winList[i].Handle,GW_OWNER) ;
        if((winList[i].ExStyle & (WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE)) || ((winList[i].Owner != NULL) && ((winList[i].ExStyle & WS_EX_APPWINDOW) == 0)))
            winList[i].Flags |= vwWTFLAGS_NO_TASKBAR_BUT ;
        if((winList[i].Owner != NULL) && ((GetWindowLong(winList[i].Owner,GWL_STYLE) & WS_VISIBLE) == 0))
            winList[i].Owner = NULL ;
        // isn't part of an existing app thats opened a new window
        if(((winList[i].Sticky = checkIfSticky(cname,wname)) == 0) && useDeskAssignment)
            winList[i].Desk = checkIfAssigned(cname,wname) ;
    }
    // now finish of initialization of owned windows
    for(i=inWin ; i < windowCount ; ++i)
    {
        if((winList[i].Tricky & vwTRICKY_POSITION) && (winList[i].Owner == NULL) &&
           (GetWindowThreadProcessId(winList[i].Handle,&iprocId) != 0))
        {
            /* some apps like Excel and Adobe Reader create one main window
             * and a hidden tricky position windows per file open for the
             * task bar, if this is one make it owned by the main window */
            j = windowCount ;
            while(--j >= 0)
            {
                if((j != i) && ((winList[j].Tricky & vwTRICKY_POSITION) == 0) &&
                   (GetWindowThreadProcessId(winList[j].Handle,&jprocId) != 0) &&
                   (jprocId == iprocId))
                {
                    vwLogBasic((_T("Making tricky window %x owned by %x\n"),
                                (int) winList[i].Handle,(int) winList[j].Handle)) ;
                    winList[i].Owner = winList[j].Handle ;
                    winList[j].Flags |= vwWTFLAGS_NO_TASKBAR_BUT | vwWTFLAGS_RM_TASKBAR_BUT ;
                    break ;
                }
            }
        }
        if(winList[i].Owner != NULL)
        {
            j = windowCount ;
            while(--j >= 0)
            {
                if(winList[j].Handle == winList[i].Owner)
                {
                    if(winList[j].State == 0)
                        /* owner has gone */
                        winList[i].Owner = NULL ;
                    else if(winList[i].Handle == winList[j].Owner)
                        /* circular owner loop, break the loop */
                        winList[j].Owner = NULL ;
                    if(winList[i].Owner != NULL)
                    {
                        // an existing app has either unhidden an old window or popped up a new one 
                        // use the existing window's settings for this one
                        if(winList[j].Owner != NULL)
                            winList[i].Owner = winList[j].Owner ;
                        
                        if(j >= inWin)
                        {
                            /* two linked windows have started together, treat them as one, but put
                             * the info onto the parent window to avoid dupication and order issues */
                            winList[j].Sticky |= winList[i].Sticky ;
                            winList[i].Sticky = FALSE ;
                            if(winList[i].Desk != currentDesk)
                            {
                                winList[j].Desk = winList[j].Desk ;
                                winList[i].Desk = currentDesk ;
                            }
                        }
                        else
                        {
                            if(winList[j].Sticky)
                                winList[i].Sticky = TRUE ;
                            if((winList[i].Desk=winList[j].Desk) != currentDesk)
                                winList[i].State = 2 ;
                        }
                        if((winList[i].Tricky & vwTRICKY_WINDOW) && 
                           !(winList[j].Tricky & vwTRICKY_WINDOW) && winList[j].Visible)
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
    }
    // finally we can apply any auto stick or assignments
    for(i=inWin ; i < windowCount ; ++i)
    {
        if(winList[i].Sticky)
        {
            /* flagged as sticky in cfg or owner is sticky */ 
            winList[i].Desk = currentDesk ;
            windowSetSticky(winList[i].Handle,TRUE) ;
        }
        else if(winList[i].Desk != currentDesk)
        {
            j = winList[i].Desk ;
            winList[i].Desk = currentDesk ;
            if(j > nDesks)
                windowSetDesk(winList[i].Handle,j,1,FALSE) ;
            else
            {
                windowSetDesk(winList[i].Handle,j,assignImmediately,FALSE) ;
                if(assignImmediately && (hiddenWindowAct == 3) && (newDesk == 0))
                    newDesk = j ;
            }
        }
    }
    if(vwLogEnabled())
    {
        for(i=inWin ; i < windowCount ; ++i)
        {
            GetWindowRect(winList[i].Handle,&pos) ;
            GetClassName(winList[i].Handle,cname,vwCLASSNAME_MAX);
            if(!GetWindowText(winList[i].Handle,wname,vwWINDOWNAME_MAX))
                _tcscpy(wname,_T("<None>"));
            vwLogBasic((_T("Got new window %8x %08x %08x Desk %d Flg %d Stk %d Trk %d Vis %d Own %x Pos %d %d\n  Class \"%s\" Title \"%s\"\n"),
                        (int) winList[i].Handle,(int)winList[i].Style,(int)winList[i].ExStyle,
                        (int)winList[i].Desk,(int)winList[i].Flags,(int)winList[i].Sticky,(int)winList[i].Tricky,
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
    for(i=0, j=0 ; i < windowCount; ++i)
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
    windowCount = j ;
    
    // Handle the re-assignment of any popped up window, set the zorder and count hung windows
    vwLogVerbose((_T("Active %8x Last %8x %x LBO %8x %x\n"),(int) activeHWnd,
                    (int) lastFGHWnd, lastFGStyle, (int) lastBOFGHWnd, lastBOFGStyle)) ;
    i = windowCount ;
    while(--i >= 0)
    {
        if(winList[i].State == 2)
        {
            if((winList[i].Desk != currentDesk) && ((hiddenWindowAct == 0) || (winList[i].Desk > nDesks)))
            {
                j = winList[i].Desk ;
                windowSetDesk(winList[i].Handle,currentDesk,2,FALSE) ;
                windowSetDesk(winList[i].Handle,j,1,FALSE) ;
                if(winList[i].Handle == activeHWnd)
                    activeHWnd = NULL ;
            }
            else
            {
                if((hiddenWindowAct == 3) && (newDesk == 0))
                    newDesk = winList[i].Desk ;
                windowSetDesk(winList[i].Handle,currentDesk,hiddenWindowAct,FALSE) ;
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
                {
                    // not a popup, windows selected replacement - ingore
                    vwLogBasic((_T("Ignore Popup - %8x %d %d, Last %8x %x LBO %8x %x\n"),(int) activeHWnd,
                                winList[i].Visible,winList[i].Tricky,
                                (int) lastFGHWnd, lastFGStyle, (int) lastBOFGHWnd, lastBOFGStyle)) ;
                    setForegroundWin(NULL,0) ;
                }
                else if((winList[i].Desk != currentDesk) && ((hiddenWindowAct == 0) || (winList[i].Desk > nDesks)))
                {
                    vwLogBasic((_T("Ignore Popup2 %d %d\n"),(int) hiddenWindowAct,winList[i].Desk)) ;
                    setForegroundWin(NULL,0) ;
                }
                else
                {
                    if(!winList[i].Visible && hiddenWindowAct)
                    {
                        vwLogBasic((_T("Got Popup - Active %8x Last %8x %x LBO %8x %x\n"),(int) activeHWnd,
                                        (int) lastFGHWnd, lastFGStyle, (int) lastBOFGHWnd, lastBOFGStyle)) ;
                        if((hiddenWindowAct == 3) && (newDesk == 0))
                            newDesk = winList[i].Desk ;
                        windowSetDesk(winList[i].Handle,currentDesk,hiddenWindowAct,FALSE) ;
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
    vwLogVerbose((_T("Updated winList, %d windows - %d hung, newDesk %d\n"),windowCount,hungCount,newDesk)) ;
    return ((newDesk << 16) | hungCount) ;
}

/*************************************************
 * Makes all windows visible
 */
static void showAll(int shwFlags)
{
    int x ;
    vwMutexLock();
    winListUpdate() ;
    for (x = 0; x < windowCount; ++x) 
    {
        // still ignore windows on a private desktop unless exiting (vwSHWIN_TRYHARD)
        if((winList[x].Desk <= nDesks) || (shwFlags & vwSHWIN_TRYHARD))
        {
            winList[x].Desk = currentDesk ;
            showHideWindow(&winList[x],shwFlags,vwVISIBLE_YES);
        }
    }
    vwMutexRelease();
}

/************************************************
 * Does necessary stuff before shutting down
 */
static void shutDown(void)
{
    KillTimer(hWnd, 0x29a);                // Remove the timer
    unloadModules();
    vwHotkeyUnregister();
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
    static vwUInt mtCount=0 ;
    int index, hungCount ;
    
    vwMutexLock();
    timerCounter++ ;
    hungCount = winListUpdate() ;
    if((index = (hungCount >> 16)) > 0)
    {
        vwMutexRelease();
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
            /* flash the icon for half a second each time we try */
            vwIconSet(currentDesk,0-hungCount) ;
            index = windowCount ;
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
        else if(((mtCount-1) & index) == 0)
        {
            /* restore the flashing icon */
            vwIconSet(currentDesk,hungCount) ;
        }
    }
    else if(mtCount)
    {
        /* hung process problem has been resolved. */
        mtCount = 0 ;
        vwIconSet(currentDesk,0);
    }

    if(taskbarFixRequired)
    {
        index = windowCount ;
        while(--index >= 0)
            if(winList[index].Tricky && !winList[index].Visible)
                PostMessage(taskHWnd, RM_Shellhook, HSHELL_WINDOWDESTROYED, (LPARAM) winList[index].Handle);
    }
    vwMutexRelease();
}

/*************************************************
 * Helper function for all the step* functions below
 * Does the actual switching work 
 */
static int changeDesk(int newDesk, WPARAM msgWParam)
{
    HWND activeHWnd, zoh ;
    vwUInt activeZOrder=0, zox, zoy, zob ;
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
    vwMutexLock();
    winListUpdate() ;
    
    if(isDragging || (checkMouseState(1) == 1))
    {
        /* move the app we are dragging to the new desktop, must handle owner windows */
        HWND ownerWnd;
        
        activeHWnd = GetForegroundWindow() ;
        do {
            ownerWnd = 0 ;
            x = windowCount ;
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
        storeDesktopProperties() ;
    
    currentDesk = newDesk;
    /* Calculations for getting the x and y positions */
    currentDeskY = ((currentDesk - 1)/nDesksX) + 1 ;
    currentDeskX = nDesksX + currentDesk - (currentDeskY * nDesksX);
    
    activeHWnd = NULL;
    if(preserveZOrder == 1)
    {
        for (x = 0; x < windowCount ; ++x)
        {
            if(winList[x].Sticky || (!minSwitch && (winList[x].Style & WS_MINIMIZE)))
                winList[x].Desk = currentDesk ;
            if((winList[x].Desk != currentDesk) && winList[x].Visible)
                // Hide these, not iconic or "Switch minimized" enabled
                showHideWindow(&winList[x],0,vwVISIBLE_NO) ;
        }
        y = windowCount ;
        zoy = 0xffffffff ;
        zoh = HWND_NOTOPMOST ;
        for(;;)
        {
            b = -1 ;
            zob = 0 ;
            for(x = 0; x < windowCount ; ++x)
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
        for (x = 0; x < windowCount ; ++x)
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
                else if(y)
                {
                    if(((winList[x].Flags & vwWTFLAGS_NO_TASKBAR_BUT) == 0) && (taskHWnd != NULL))
                    {
                        /* It is visible but its position in the taskbar is wrong - fix by removing from the taskbar and then adding again */
                        PostMessage(taskHWnd,RM_Shellhook,HSHELL_WINDOWDESTROYED,(LPARAM) winList[x].Handle) ;
                        Sleep(1);
                        PostMessage(taskHWnd,RM_Shellhook,HSHELL_WINDOWCREATED,(LPARAM) winList[x].Handle) ;
                    }
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
            
            y = windowCount ;
            zoy = 0xffffffff ;
            zoh = HWND_NOTOPMOST ;
            for(;;)
            {
                b = -1 ;
                zob = 0 ;
                for(x = 0; x < windowCount ; ++x)
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
    vwMutexRelease();
    
    if(setupOpen)
    {
        initDesktopProperties() ;
        showSetup() ;
    }
    vwIconSet(currentDesk,0) ;
    if(refreshOnWarp) // Refresh the desktop 
        RedrawWindow( NULL, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN );
    
    postModuleMessage(MOD_CHANGEDESK,msgWParam,currentDesk);
    
    vwLogBasic((_T("Step Desk End (%x)\n"),(int)GetForegroundWindow())) ;
    
    return currentDesk ;
}

/*************************************************
 * Goto a specified desktop specifying desk number
 */
int
gotoDesk(int theDesk, vwUByte force)
{
    if((theDesk <= 0) || (theDesk >= vwDESKTOP_SIZE) || ((theDesk > nDesks) && !force))
        return 0;
    
    return changeDesk(theDesk,MOD_CHANGEDESK);
}

/*************************************************
 * Goto a specified desktop specifying desk number
 */
static int
stepDelta(int delta)
{
    int newDesk ;
    if(currentDesk > nDesks)
        /* on a private desktop - go to first if delta is +ve, last otherwise */
        newDesk = (delta < 0) ? nDesks:1 ;
    else if((newDesk=currentDesk+delta) < 1)
    {
        if(!deskWrap)
            return 0 ;
        newDesk = nDesks ;
    }
    else if(newDesk > nDesks)
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
static int
stepRight(void)
{
    int deskX, deskY=currentDeskY ;
    
    if(currentDesk > nDesks)
    {   /* on a private desktop - go to first */
        deskX = 1;
        deskY = 1;
    }
    else if((deskX=currentDeskX + 1) > nDesksX)
    {
        if(!deskWrap)
            return 0;
        deskX = 1;
    }
    return changeDesk(calculateDesk(deskX,deskY),MOD_STEPRIGHT);
}

/*************************************************
 * Step one desk to the left
 */
static int
stepLeft(void)
{
    int deskX, deskY=currentDeskY ;
    
    if(currentDesk > nDesks)
    {   /* on a private desktop - go to last */
        deskX = nDesksX;
        deskY = nDesksY;
    }
    else if((deskX=currentDeskX - 1) < 1)
    {
        if(!deskWrap)
            return 0;
        deskX = nDesksX;
    }
    return changeDesk(calculateDesk(deskX,deskY),MOD_STEPLEFT);
}

/*************************************************
 * Step one desk down
 */
static int
stepDown(void)
{
    int deskX=currentDeskX, deskY ;
    
    if(currentDesk > nDesks)
    {   /* on a private desktop - go to first */
        deskX = 1;
        deskY = 1;
    }
    else if((deskY = currentDeskY + 1) > nDesksY)
    {
        if(!deskWrap)
            return 0;
        deskY = 1;
    }
    return changeDesk(calculateDesk(deskX,deskY),MOD_STEPDOWN);
}

/*************************************************
 * Step one desk up
 */
static int
stepUp(void)
{
    int deskX=currentDeskX, deskY ;
    
    if(currentDesk > nDesks)
    {   /* on a private desktop - go to last */
        deskX = nDesksX;
        deskY = nDesksY;
    }
    else if((deskY = currentDeskY - 1) < 1)
    {
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
#define vwPMENU_ALLWIN   0x0001
#define vwPMENU_COMPACT  0x0002
#define vwPMENU_MULTICOL 0x0004
#define vwPMENU_MRU      0x0008
#define vwPMENU_TITLEID  0x0100
#define vwPMENU_ACCESS   0x0100
#define vwPMENU_ASSIGN   0x0200
#define vwPMENU_SHOW     0x0400
#define vwPMENU_STICKY   0x0800
#define vwPMENU_ID_MASK  0x00ff
#define vwPMENU_COL_MASK 0x0f00

static int
winListCreateItemList(int flags, vwMenuItem **items,int *numitems)
{
    vwMenuItem *item;
    TCHAR fmt[16], *ss ;
    TCHAR title[vwWINDOWNAME_MAX+18];
    TCHAR buff[MAX_PATH];
    int i,len,x,y,c,listContent=(winListContent & ~vwWINLIST_ASSIGN) ;
    
    ss = fmt ;
    *ss++ = '%' ;
    if(nDesks > 9)
        *ss++ = '2' ;
#ifdef NDEBUG
    _tcscpy(ss,_T("d - %s")) ;
#else
    _tcscpy(ss,_T("d - %s (%x)")) ;
#endif
    // create the window list
    vwMutexLock();
    winListUpdate() ;
    i = 0 ;
    len = (flags & 0x01) ? vwWINDOWNAME_MAX:30 ;
    for(c = 0; c < windowCount; ++c)
    {
        // ignore owned windows if we are managing the owner and one's on a hidden desktop
        if(winList[c].Desk > nDesks)
            x = 0 ;
        else if(winList[c].Owner == NULL)
            x = -1 ;
        else
        {
            x = windowCount ;
            while(--x >= 0)
                if(winList[x].Handle == winList[c].Owner)
                    break ;
        }
        if(x < 0)
        {
            HICON hSmallIcon ;
            
            GetWindowText(winList[c].Handle, buff, len);
#ifdef NDEBUG
            _stprintf(title,fmt,winList[c].Desk,buff);
#else
            _stprintf(title,fmt,winList[c].Desk,buff,(int) winList[c].Handle);
#endif
            if(((item = malloc(sizeof(vwMenuItem))) == NULL) ||
               ((item->name = _tcsdup(title)) == NULL))
            {
                while(--i >= 0)
                {
                    free(items[i]->name) ;
                    free(items[i]) ;
                }
                return 0 ;
            }
            if(flags & 0x02)
            {
                /* MRU menu - sort based on z-order */
                item->zOrder = winList[c].ZOrder[winList[c].Desk] ;
                if(winList[c].Sticky)
                {
                    y = nDesks ;
                    do {
                        if(winList[c].ZOrder[y] > item->zOrder)
                            item->zOrder = winList[c].ZOrder[y] ;
                    } while(--y > 0) ;
                }
                else if((winList[c].Visible == vwVISIBLE_YESTEMP) &&
                        (winList[c].ZOrder[currentDesk] > item->zOrder))
                    item->zOrder = winList[c].ZOrder[currentDesk] ;
                
                for(y=0 ; y<i ; y++)
                    if(item->zOrder > items[y]->zOrder)
                        break ;
            }
            else
            {
                for(y=0 ; y<i ; y++)
                    if(_tcscmp(item->name,items[y]->name) < 0)
                        break ;
            }
            x = i ;
            while(x > y)
            {
                items[x] = items[x-1] ;
                x-- ;
            }
            items[x] = item ;
            i++ ;
            
            if((hSmallIcon = (HICON)GetClassLong(winList[c].Handle, GCL_HICON)) == NULL)
            {
                // Fallback plan, maybe this works better for this type of application
                // Otherwise there is not much we can do (could try looking for an owned window)
                // Note: some apps (e.g. Opera) only have big icons
                DWORD theIcon;
                if((SendMessageTimeout(winList[c].Handle, WM_GETICON, ICON_SMALL, 0L, 
                                       SMTO_ABORTIFHUNG | SMTO_BLOCK, 100, &theIcon) && (theIcon != 0)) ||
                   (SendMessageTimeout(winList[c].Handle, WM_GETICON, ICON_BIG, 0L, 
                                       SMTO_ABORTIFHUNG | SMTO_BLOCK, 100, &theIcon) && (theIcon != 0)))
                    hSmallIcon = (HICON) theIcon ;
            }
            item->icon = hSmallIcon ;
            if(((item->desk = winList[c].Desk) != currentDesk) && !winList[c].Sticky)
                listContent = winListContent ;
            item->sticky = winList[c].Sticky;
            item->id = i ;
            winList[c].menuId = i ;
        }
        else
            winList[c].menuId = 0 ;
    }
    vwMutexRelease();
    if((i == 0) || ((listContent == 0) && ((flags & 0x02) == 0)))
    {
        // Either user has no apps, disabled all 3 menus or only enable assign and all are on the current desk
        for (x=0; x<i; x++)
        {
            free(items[x]->name) ;
            free(items[x]) ;
        }
        return 0 ;
    }
    
    *numitems = i ;
    
    if(flags & 0x02)
        /* MRU menu */
        i = vwPMENU_MRU|vwPMENU_MULTICOL ;
    else
    {
        i = listContent << 8 ;
        if(listContent != vwWINLIST_ASSIGN)
            i |= vwPMENU_ALLWIN ;
        if(flags & 0x01)
            i |= vwPMENU_COMPACT ;
    }
    return i ;
}

static HMENU
winListCreateMenu(int flags, int itemCount, vwMenuItem **items)
{
    MENUITEMINFO minfo ;
    HMENU hMenu;
    int c, x, divFlags, itemsPerCol, itemsAllowed ;
    
    if((hMenu = CreatePopupMenu()) == NULL)
        return NULL ;
    
    minfo.cbSize = sizeof(MENUITEMINFO) ;
    minfo.fMask = MIIM_STATE ;
    minfo.fState = MFS_DEFAULT ;
    divFlags = (flags & vwPMENU_COMPACT) ? MF_STRING:(MF_STRING | MF_DISABLED) ;
    if(flags & vwPMENU_MULTICOL)
        itemsPerCol = (desktopWorkArea[1][3]-desktopWorkArea[1][1]) / (ICON_SIZE + (2*MENU_Y_PADDING)) ;
    else
        itemsPerCol = -1 ;
    
    if(flags & vwPMENU_MRU)
    {
        itemsAllowed = itemsPerCol - 1 ; 
        for(x=0 ; x < itemCount ; x++)
        {
            AppendMenu(hMenu,((--itemsAllowed == 0) ? ((itemsAllowed = itemsPerCol),(MF_OWNERDRAW|MF_MENUBARBREAK)):MF_OWNERDRAW),
                       vwPMENU_ACCESS | (items[x]->id), (const TCHAR *) items[x]);
        }
#if 0
        /* TODO */
        if(itemCount > 1)
        {
            HiliteMenuItem(NULL,hMenu,1,MF_HILITE|MF_BYPOSITION);
        }
#endif
    }
    if(flags & vwPMENU_ACCESS)
    {
        AppendMenu(hMenu,divFlags,vwPMENU_ACCESS,(flags & vwPMENU_COMPACT) ? _T("Access    (&Next ->)"):_T("Access"));
        SetMenuItemInfo(hMenu,vwPMENU_ACCESS,FALSE,&minfo) ;
        AppendMenu(hMenu,MF_SEPARATOR,0,NULL);
        divFlags |= MF_MENUBARBREAK ;
        itemsAllowed = itemsPerCol - 1 ; 
        for(x=0,c=0 ; x < itemCount ; x++)
        {
            if((c != 0) && (c != items[x]->desk))
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
            c = items[x]->desk;
            AppendMenu(hMenu,((--itemsAllowed == 0) ? ((itemsAllowed = itemsPerCol),(MF_OWNERDRAW|MF_MENUBARBREAK)):MF_OWNERDRAW),
                       vwPMENU_ACCESS | (items[x]->id), (const TCHAR *) items[x] );
        }
    }
    if(flags & vwPMENU_ASSIGN)
    {
        AppendMenu(hMenu,divFlags,vwPMENU_ASSIGN,(flags & vwPMENU_COMPACT) ? _T("Assign    (&Next ->)"):_T("Assign"));
        SetMenuItemInfo(hMenu,vwPMENU_ASSIGN,FALSE,&minfo) ;
        AppendMenu(hMenu,MF_SEPARATOR,0,NULL);
        divFlags |= MF_MENUBARBREAK ;
        itemsAllowed = itemsPerCol - 1 ; 
        for(x=0,c=0 ; x < itemCount ; x++)
        {
            //cannot assign windows already on the current Desktop
            if((items[x]->desk != currentDesk) || (flags & vwPMENU_ALLWIN))
            {
                if((c != 0) && (c != items[x]->desk))
                    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                c = items[x]->desk;
                if(items[x]->desk != currentDesk)
                    AppendMenu(hMenu, ((--itemsAllowed == 0) ? ((itemsAllowed = itemsPerCol),(MF_OWNERDRAW|MF_MENUBARBREAK)):MF_OWNERDRAW),
                               (vwPMENU_ASSIGN | (items[x]->id)), (const TCHAR *) items[x] );
                else
                    /* Make it a separator so cursor movement skips this line */
                    AppendMenu(hMenu,((--itemsAllowed == 0) ? ((itemsAllowed = itemsPerCol),(MF_OWNERDRAW|MF_MENUBARBREAK|MF_SEPARATOR)):(MF_OWNERDRAW|MF_SEPARATOR)), 0, 0) ;
            }
        }
    }
    if(flags & vwPMENU_SHOW)
    {
        AppendMenu(hMenu,divFlags,vwPMENU_SHOW,(flags & vwPMENU_COMPACT) ? _T("Show      (&Next ->)"):_T("Show"));
        SetMenuItemInfo(hMenu,vwPMENU_SHOW,FALSE,&minfo) ;
        AppendMenu(hMenu,MF_SEPARATOR,0,NULL);
        divFlags |= MF_MENUBARBREAK ;
        itemsAllowed = itemsPerCol - 1 ; 
        for(x=0,c=0 ; x < itemCount ; x++)
        {
            if((c != 0) && (c != items[x]->desk))
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
            c = items[x]->desk;
            AppendMenu(hMenu,((--itemsAllowed == 0) ? ((itemsAllowed = itemsPerCol),(MF_OWNERDRAW|MF_MENUBARBREAK)):MF_OWNERDRAW),
                       vwPMENU_SHOW | (items[x]->id), (const TCHAR *) items[x] );
        }
    }
    if(flags & vwPMENU_STICKY)
    {
        AppendMenu(hMenu,divFlags,vwPMENU_STICKY,(flags & vwPMENU_COMPACT) ? _T("Sticky    (&Next ->)"):_T("Sticky"));
        SetMenuItemInfo(hMenu,vwPMENU_STICKY,FALSE,&minfo) ;
        AppendMenu(hMenu,MF_SEPARATOR,0,NULL);
        divFlags |= MF_MENUBARBREAK ;
        itemsAllowed = itemsPerCol - 1 ; 
        for(x=0,c=0 ; x < itemCount ; x++)
        {
            if((c != 0) && (c != items[x]->desk))
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
            c = items[x]->desk;
            AppendMenu(hMenu, MF_OWNERDRAW | (items[x]->sticky ? MF_CHECKED: 0) | ((--itemsAllowed == 0) ? ((itemsAllowed = itemsPerCol),MF_MENUBARBREAK):0),
                       vwPMENU_STICKY | (items[x]->id), (const TCHAR *) items[x] );
        }
    }
    
    return hMenu ;
}



/************************************************
 * This function decides what switching technique that should be used
 * and calls the appropriate switching function
 */
static int
showHideWindow(windowType* aWindow, int shwFlags, unsigned char show)
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
            if((aWindow->Flags & vwWTFLAGS_RM_TASKBAR_BUT) && (taskHWnd != NULL))
                /* Make sure the taskbar does not create a button for this window */
                PostMessage(taskHWnd, RM_Shellhook, HSHELL_WINDOWDESTROYED, (LPARAM) aWindow->Handle);
        }
        else
        {
            // show/hide the window in the toolbar and move off screen
            swpFlags = SWP_FRAMECHANGED | SWP_DEFERERASE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_NOMOVE ;
            if(show)
            {
                // Restore the window mode
                SetWindowLong( aWindow->Handle, GWL_EXSTYLE, aWindow->ExStyle );  
                if(((aWindow->Flags & vwWTFLAGS_NO_TASKBAR_BUT) == 0) && (taskHWnd != NULL))
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
                if(((aWindow->Flags & vwWTFLAGS_NO_TASKBAR_BUT) == 0) && (taskHWnd != NULL))
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
static void
vwToggleEnabled(void)
{
    if(vwEnabled)
    {   // disable VirtuaWin
        KillTimer(hWnd, 0x29a);
        _tcscpy(nIconD.szTip,vwVIRTUAWIN_NAME _T(" - Disabled")); //Tooltip
        vwIconSet(0,0);
        enableMouse(FALSE);
        vwHotkeyUnregister();
        vwEnabled = FALSE;
    }
    else
    {   // Enable VirtuaWin
        vwHotkeyRegister();
        enableMouse(mouseEnable);
        vwIconSet(currentDesk,0);
        vwEnabled = TRUE;
        SetTimer(hWnd, 0x29a, 250, monitorTimerProc);
    }
}

/************************************************
 * Assigns a window to the specified desktop
 * Used by the module message VW_ASSIGNWIN
 */
int
assignWindow(HWND theWin, int theDesk, vwUByte follow, vwUByte force, vwUByte setActive)
{
    int ret, idx ;
    vwUByte sticky=0 ;
    
    vwLogBasic((_T("Assign window: %x %d %d\n"),(int) theWin,theDesk,force)) ;
    
    if(((theWin == NULL) && ((theWin = GetForegroundWindow()) == NULL)) || (theWin == hWnd))
        return 0 ;
    
    switch(theDesk)
    {
    case VW_STEPPREV:
        if((currentDesk > nDesks) ||
           ((theDesk = currentDesk-1) <= 0))
            theDesk = 0 - nDesks ;
        break;
    case VW_STEPNEXT:
        if((currentDesk > nDesks) ||
           ((theDesk = currentDesk+1) > nDesks))
            theDesk = 0 - 1 ;
        break;
    case VW_STEPLEFT:
        if(currentDesk > nDesks)
            theDesk = 0 - nDesks ;
        else if(currentDeskX <= 1)
            theDesk = 0 - calculateDesk(nDesksX,currentDeskY) ;
        else
            theDesk = calculateDesk(currentDeskX-1,currentDeskY) ;
        break;
    case VW_STEPRIGHT:
        if(currentDesk > nDesks)
            theDesk = 0 - 1 ;
        else if(currentDeskX >= nDesksX)
            theDesk = 0 - calculateDesk(1,currentDeskY) ;
        else
            theDesk = calculateDesk(currentDeskX+1,currentDeskY) ;
        break;
    case VW_STEPUP:
        if(currentDesk > nDesks)
            theDesk = 0 - nDesks ;
        else if(currentDeskY <= 1)
            theDesk = 0 - calculateDesk(currentDeskX,nDesksY) ;
        else
            theDesk = calculateDesk(currentDeskX,currentDeskY-1) ;
        break;
    case VW_STEPDOWN:
        if(currentDesk > nDesks)
            theDesk = 0 - 1 ;
        else if(currentDeskY >= nDesksY)
            theDesk = 0 - calculateDesk(currentDeskX,1) ;
        else
            theDesk = calculateDesk(currentDeskX,currentDeskY+1) ;
        break;
    }
    if(theDesk < 0)
    {
        if(!deskWrap)
            // wrapping disabled don't allow this
            return 0 ;
        theDesk = 0 - theDesk ;
    }
    if((theDesk <= 0) || (theDesk >= vwDESKTOP_SIZE) || 
       ((theDesk > nDesks) && (!force || !desktopUsed[theDesk])))
        return 0 ; // Invalid desk
    
    vwMutexLock();
    winListUpdate() ;
    idx = winListFind(theWin) ;
    ret = (idx >= 0) ;
    if(ret)
        sticky = winList[idx].Sticky ;
    vwMutexRelease();
    if(ret)
    {
        if(follow)
        {
            windowSetSticky(theWin,1) ;
            ret = (changeDesk(theDesk,MOD_CHANGEDESK) > 0) ;
            windowSetSticky(theWin,sticky) ;
        }
        else
            ret = windowSetDesk(theWin,theDesk,1,setActive) ;
    }
    return ret ;
}

/************************************************
 * Access a window where method:
 *   0 = use config setting, 1 = move, 2 = show, 3 = change desk
 * Used by the module message VW_ACCESSWIN
 */
static int
accessWindow(HWND theWin, vwUByte method, vwUByte force)
{
    int ret, idx ;
    
    vwLogBasic((_T("Access window: %x %d\n"),(int) theWin,method)) ;
    
    if((theWin == NULL) || (theWin == hWnd))
        return 0 ;
    if((method == 0) && ((method=hiddenWindowAct) == 0))
        method = 1 ;
        
    vwMutexLock();
    winListUpdate() ;
    idx = winListFind(theWin) ;
    ret = (idx >= 0) ;
    vwMutexRelease();
    
    if(ret)
    {
        if((winList[idx].Desk > nDesks) && !force)
            ret = 0 ;
        else if(method != 3)
            ret = windowSetDesk(winList[idx].Handle,currentDesk,method,FALSE) ;
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
static int
setSticky(HWND theWin, int state)
{
    int ret ;
    vwLogBasic((_T("Set sticky window: %x %d\n"),(int) theWin,state)) ;
    
    if((theWin == NULL) && ((theWin = GetForegroundWindow()) == NULL))
        return 0 ;
    
    vwMutexLock();
    winListUpdate() ;
    ret = windowSetSticky(theWin,state) ;
    vwMutexRelease();
    return ret ;
}

/************************************************
 * Dismisses the current window by either moving it
 * back to its assigned desk or minimizing.
 */
static int
windowDismiss(HWND theWin)
{
    vwUInt activeZOrder ;
    int ret, idx ;
    HWND hwnd ;
    
    vwLogBasic((_T("Dismissing window: %x\n"),(int) theWin)) ;
    if(theWin == NULL)
        return 0 ;
    
    while((GetWindowLong(theWin, GWL_STYLE) & WS_CHILD) && 
          ((hwnd = GetParent(theWin)) != NULL) && (hwnd != desktopHWnd))
        theWin = hwnd ;

    vwMutexLock();
    winListUpdate();
    if((idx = winListFind(theWin)) < 0)
        ret = 0 ;
    else if(winList[idx].Visible == vwVISIBLE_YESTEMP)
        ret = windowSetDesk(theWin,winList[idx].Desk,1,FALSE) ;
    else
    {
        ret = CloseWindow(theWin) ;
        /* need to change the focus to another window */
        hwnd = NULL;
        activeZOrder = 0;
        for (idx = 0; idx < windowCount ; ++idx)
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
    vwMutexRelease();
    return ret ;
}

static void
windowSetAlwaysOnTop(HWND theWin)
{
    int ExStyle ;
    
    vwLogBasic((_T("AlwaysOnTop window: %x\n"),(int) theWin)) ;
    if(theWin != NULL)
    {
        ExStyle = GetWindowLong(theWin,GWL_EXSTYLE) ;
        SetWindowPos(theWin,(ExStyle & WS_EX_TOPMOST) ? HWND_NOTOPMOST:HWND_TOPMOST,0,0,0,0,
                     SWP_DEFERERASE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOSENDCHANGING|SWP_NOMOVE) ;
    }
}

static void
windowPushToBottom(HWND theWin)
{
    vwUInt minZOrder=0xffffffff, maxZOrder=0 ;
    HWND pWin=NULL ;
    int ii, idx ;
    pWin = NULL ;
    ii = -1 ;
    idx = windowCount ;
    while(--idx >= 0)
    {
        if(winList[idx].Handle == theWin)
            ii = idx ;
        else if((winList[idx].Desk == currentDesk) || (winList[idx].Visible == vwVISIBLE_YESTEMP))
        {
            if(winList[idx].ZOrder[currentDesk] < minZOrder)
                minZOrder = winList[idx].ZOrder[currentDesk] ;
            if(((winList[idx].Style & (WS_MINIMIZE|WS_VISIBLE)) == WS_VISIBLE) && 
               (winList[idx].ZOrder[currentDesk] >= maxZOrder))
            {
                pWin = winList[idx].Handle;
                maxZOrder = winList[idx].ZOrder[currentDesk];
            }
        }
    }
    if((ii >= 0) && (winList[ii].ZOrder[currentDesk] >= minZOrder))
        winList[ii].ZOrder[currentDesk] = (minZOrder > 0) ? minZOrder-1:0 ;
    if(pWin != NULL)
        setForegroundWin(pWin,0);
    SetWindowPos(theWin,HWND_BOTTOM,0,0,0,0,
                 SWP_DEFERERASE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOSENDCHANGING|SWP_NOMOVE) ;
}

static BOOL CALLBACK
WindowInfoDialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
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
            vwMutexLock();
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
                _stprintf(ss,_T("being managed\r\n\tOwner:\t%x\r\n\tStyles:\t%08x %08x\r\n\tDesk:\t%d\r\n\tFlags:\t%d\r\n\tSticky:\t%d\r\n\tTricky:\t%d\r\n\tVisible:\t%d\r\n"),
                          (int)winList[idx].Owner,(int)winList[idx].Style,(int)winList[idx].ExStyle,
                          (int)winList[idx].Desk,(int)winList[idx].Flags,(int)winList[idx].Sticky,
                          (int)winList[idx].Tricky,(int)winList[idx].Visible) ;
            else
                _tcscpy(ss,_T("not managed\r\n")) ;
            vwMutexRelease();
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
    int ExStyle, ii, idx ;
    
    vwLogBasic((_T("Window Menu: %x\n"),(int) theWin)) ;
    
    if((theWin == NULL) || (theWin == hWnd) || (theWin == desktopHWnd) || (theWin == taskHWnd))
        return ;
    while(((ii=GetWindowLong(theWin,GWL_STYLE)) & WS_CHILD) && 
          ((pWin = GetParent(theWin)) != NULL) && (pWin != desktopHWnd))
        theWin = pWin ;
    // Must be a visible non-child window
    if((ii & WS_CHILD) || ((ii & WS_VISIBLE) == 0))
        return ;
    
    ExStyle = GetWindowLong(theWin,GWL_EXSTYLE) ;
    vwMutexLock();
    winListUpdate() ;
    if((idx = winListFind(theWin)) >= 0)
    {
        Sticky = winList[idx].Sticky ;
        Tricky = winList[idx].Tricky ;
    }
    vwMutexRelease();

    if((hpopup = CreatePopupMenu()) == NULL)
        return ;
    
    AppendMenu(hpopup,(ExStyle & WS_EX_TOPMOST) ? (MF_STRING|MF_GRAYED):MF_STRING,ID_WM_BOTTOM,_T("Push to &Bottom"));
    AppendMenu(hpopup,(ExStyle & WS_EX_TOPMOST) ? (MF_STRING|MF_CHECKED):MF_STRING,ID_WM_ONTOP,_T("Always on &Top"));
    AppendMenu(hpopup,MF_STRING,ID_WM_DISMISS,_T("&Dismiss Window"));
    
    if(idx >= 0)
    {
        /* currently managed window */
        AppendMenu(hpopup,(Sticky) ? (MF_STRING|MF_CHECKED):MF_STRING,ID_WM_STICKY,_T("&Sticky"));
        AppendMenu(hpopup,(deskWrap || (currentDesk < nDesks)) ? MF_STRING:(MF_STRING|MF_GRAYED),ID_WM_NEXT,_T("Move to &Next"));
        AppendMenu(hpopup,(deskWrap || (currentDesk > 1)) ? MF_STRING:(MF_STRING|MF_GRAYED),ID_WM_PREV,_T("Move to &Previous"));
        AppendMenu(hpopup,MF_SEPARATOR,0,NULL) ;
        _tcscpy(buff,_T("Move to Desk & ")) ;
        for(ii = 1 ; ii <= nDesks ; ii++)
        {
            if(ii >= 10)
                buff[13] = (ii/10)+'0' ;
            buff[14] = (ii%10)+'0' ;
            AppendMenu(hpopup,(ii == currentDesk) ? (MF_STRING|MF_GRAYED):MF_STRING,ID_WM_DESK+ii,buff) ;
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
    idx = TrackPopupMenu(hpopup,TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,pt.x,pt.y,0,hWnd,NULL) ;
    PostMessage(hWnd, 0, 0, 0);	
    DestroyMenu(hpopup);		
    vwLogBasic((_T("Window Menu returned %d\n"),(int) idx)) ;
    switch(idx)
    {
    case 0:
        break ;
    case ID_WM_DISMISS:
        windowDismiss(theWin) ;
        return ;
    case ID_WM_ONTOP:
        windowSetAlwaysOnTop(theWin) ;
        break ;
    case ID_WM_BOTTOM:
        windowPushToBottom(theWin) ;
        return ;
    case ID_WM_STICKY:
        setSticky(theWin,-1) ;
        break;
    case ID_WM_INFO:
        DialogBox(hInst,MAKEINTRESOURCE(IDD_WINDOWINFODIALOG),theWin,(DLGPROC) WindowInfoDialogFunc);
        return ;
    case ID_WM_MANAGE:
        vwMutexLock();
        winListUpdate() ;
        if((winListFind(theWin) < 0) && (windowCount < vwWINDOW_MAX))
        {
            idx = windowCount++;
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
            winList[idx].Flags = 0 ;
            vwLogBasic((_T("Got new managed window %d\n"),(int) idx)) ;
        }
        vwMutexRelease();
        break;
    default:
        if(idx == ID_WM_NEXT)
            ii = VW_STEPNEXT ;
        else if(idx == ID_WM_PREV)
            ii = VW_STEPPREV ;
        else if((idx > ID_WM_DESK) && (idx <= (ID_WM_DESK + nDesks)))
            ii = idx - ID_WM_DESK ;
        else
            return ;
        
        if(Sticky)
            setSticky(theWin,0) ;
        assignWindow(theWin,ii,FALSE,FALSE,TRUE) ;
        return ;
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
static void measureMenuItem(HWND hwnd,MEASUREITEMSTRUCT* mitem)
{
    HDC testdc;
    HFONT menufont,oldfont;
    vwMenuItem* item;
    SIZE size;
    TCHAR *measuretext;

    item = (vwMenuItem*) mitem->itemData;

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
    vwMenuItem* item;
    HFONT menufont,oldfont;
    HICON icon;
    UINT oldalign;
    HBRUSH focusbrush,oldbrush;
    HPEN oldpen;
    SIZE size;
    int backgroundcolor,textcolor, ll;

    item = (vwMenuItem *) ditem->itemData;

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
        if((icon = (ditem->itemState & ODS_CHECKED) ? checkIcon : item->icon) != NULL)
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
 * Pops up and handles the window list menu, wlFlags is a bitmask:
 *   0x01 : Create a compact menu
 *   0x02 : Create the most recently used window list
 *   0x04 : force focus change
 */
static void
winListPopupMenu(HWND aHWnd, int wlFlags)
{
    static int singleColumn=0;
    HMENU hpopup;
    POINT pt;
    HWND hwnd ;
    int scDir=1, flags, retItem, id, ii ;
    
    // storage for list of vwMenuItem structs
    vwMenuItem *items[vwWINDOW_MAX] ;
    int itemCount;
    
    if((flags = winListCreateItemList(wlFlags,items,&itemCount)) == 0)
        return ;
    GetCursorPos(&pt);
    
    /* Call setForegroundWin to remove the window focus otherwise the menu does
     * not automatically close if the user changes focus, unfortunately this breaks
     * double clicking on the systray icon so not done in this case */
    if(wlFlags & 0x04)
        setForegroundWin(NULL,0) ;
    SetForegroundWindow(aHWnd);
    singleColumn >>= 1 ;
    for(;;)
    {
        retItem = 0 ;
        if(flags & vwPMENU_COMPACT)
        {
            do {
                singleColumn = (scDir < 0) ? (singleColumn >> 1) : (singleColumn << 1) ;
                if((singleColumn & vwPMENU_COL_MASK) == 0)
                    singleColumn = (scDir < 0) ? vwPMENU_STICKY:vwPMENU_ACCESS ;
            } while((singleColumn & flags) == 0) ;
            ii = vwPMENU_ALLWIN | vwPMENU_COMPACT | vwPMENU_MULTICOL | singleColumn ;
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
        if(flags & vwPMENU_COMPACT)
            scDir = (HIWORD(GetKeyState(VK_SHIFT))) ? -1:1 ;
        else
            flags |= vwPMENU_ALLWIN|vwPMENU_ACCESS|vwPMENU_ASSIGN|vwPMENU_SHOW|vwPMENU_STICKY ;
    }
            
    if(retItem & vwPMENU_ID_MASK)
    {
        id = retItem & vwPMENU_ID_MASK ;
        ii = windowCount ;
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
            else
            {
                flags = winList[ii].Style ;
                if(retItem & vwPMENU_ASSIGN)
                    assignWindow(hwnd,currentDesk,FALSE,TRUE,FALSE) ;
                else if(retItem & vwPMENU_SHOW)
                    accessWindow(hwnd,2,FALSE) ;
                else
                    gotoDesk(winList[ii].Desk,FALSE);
                if(flags & WS_MINIMIZE)
                    ShowWindow(hwnd,SW_SHOWNORMAL) ;
                setForegroundWin(hwnd,0);
            }
        }
    }
    
    // delete vwMenuItem list
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
    int ii ;
    
    switch (message)
    {
    case VW_MOUSEWARP:
        // Is virtuawin enabled
        if(vwEnabled)
        {
            GetCursorPos(&pt);
            
            isDragging = (LOWORD(lParam) & 0x1) ;
            switch(HIWORD(lParam))
            {
            case 0:
                /* left */
                if((stepLeft() != 0) && (LOWORD(lParam) & 0x2))
                {
                    if(mouseWarp)
                        SetCursorPos(desktopWorkArea[0][2] - mouseJumpLength, pt.y);
                    else
                        SetCursorPos(pt.x + mouseJumpLength, pt.y);
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
                    if(mouseWarp)
                        SetCursorPos(pt.x, desktopWorkArea[0][3] - mouseJumpLength);
                    else
                        SetCursorPos(pt.x, pt.y + mouseJumpLength);
                }
                break;
                
            case 2:
                /* right */
                if((stepRight() != 0) && (LOWORD(lParam) & 0x2))
                {
                    if(mouseWarp)
                        SetCursorPos(desktopWorkArea[0][0] + mouseJumpLength, pt.y);
                    else
                        SetCursorPos(pt.x - mouseJumpLength, pt.y);
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
                    if(mouseWarp)
                        SetCursorPos(pt.x, desktopWorkArea[0][1] + mouseJumpLength);
                    else
                        SetCursorPos(pt.x, pt.y - mouseJumpLength);
                }
                break;
            
            case 4:
                /* window list */
                winListPopupMenu(aHWnd,winListCompact | 4) ;
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
        if(!vwEnabled)
            return FALSE ;
        ii = hotkeyCount ;
        while(--ii >= 0)
            if(hotkeyList[ii].atom == wParam)
                break ;
        if(ii < 0)
            return FALSE ;
        switch(hotkeyList[ii].command)
        {
        case vwCMD_NAV_MOVE_LEFT:
            stepLeft();
            break ;
        case vwCMD_NAV_MOVE_RIGHT:
            stepRight();
            break ;
        case vwCMD_NAV_MOVE_UP:
        case vwCMD_NAV_MOVE_DOWN:
            if((hotkeyList[ii].command == vwCMD_NAV_MOVE_UP) ^ (invertY != 0))
                stepUp();
            else
                stepDown();
            break ;
        case vwCMD_NAV_MOVE_PREV:
            stepDelta(-1) ;
            break ;
        case vwCMD_NAV_MOVE_NEXT:
            stepDelta(1) ;
            break ;
        case vwCMD_NAV_MOVE_DESKTOP:
            gotoDesk(hotkeyList[ii].desk,TRUE);
            break ;
        case vwCMD_WIN_STICKY:
            setSticky(0,-1);
            break ;
        case vwCMD_WIN_DISMISS:
            windowDismiss(GetForegroundWindow());
            break ;
        case vwCMD_WIN_MOVE_DESKTOP:
        case vwCMD_WIN_MOVE_DESK_FOL:
            assignWindow(NULL,hotkeyList[ii].desk,(vwUByte) (hotkeyList[ii].command == vwCMD_WIN_MOVE_DESK_FOL),TRUE,FALSE);
            break ;
        case vwCMD_WIN_MOVE_PREV:
        case vwCMD_WIN_MOVE_PREV_FOL:
            assignWindow(NULL,VW_STEPPREV,(vwUByte) (hotkeyList[ii].command == vwCMD_WIN_MOVE_PREV_FOL),TRUE,FALSE);
            break ;
        case vwCMD_WIN_MOVE_NEXT:
        case vwCMD_WIN_MOVE_NEXT_FOL:
            assignWindow(NULL,VW_STEPNEXT,(vwUByte) (hotkeyList[ii].command == vwCMD_WIN_MOVE_NEXT_FOL),TRUE,FALSE);
            break ;
        case vwCMD_UI_WINMENU_STD:
        case vwCMD_UI_WINMENU_CMP:
            windowMenu(GetForegroundWindow());
            break ;
        case vwCMD_UI_WINLIST_STD:
            winListPopupMenu(aHWnd,4) ;
            break ;
        case vwCMD_UI_WINLIST_CMP:
            winListPopupMenu(aHWnd,5) ;
            break ;
        case vwCMD_UI_WINLIST_MRU:
            winListPopupMenu(aHWnd,6) ;
            break ;
        case vwCMD_UI_SETUP:
            showSetup();
            break ;
        case vwCMD_WIN_PUSHTOBOTTOM:
            windowPushToBottom(GetForegroundWindow());
            break ;
        case vwCMD_WIN_ALWAYSONTOP:
            windowSetAlwaysOnTop(GetForegroundWindow());
            break ;
        }
        return TRUE;
        
        // Plugin messages
    case VW_CHANGEDESK: 
        if(!vwEnabled)
            return FALSE ;
        switch (wParam)
        {
        case VW_STEPPREV:
            return stepDelta(-1) ;
        case VW_STEPNEXT:
            return stepDelta(1) ;
        case VW_STEPLEFT:
            return stepLeft();
        case VW_STEPRIGHT:
            return stepRight();
        case VW_STEPUP:
            return stepUp();
        case VW_STEPDOWN:
            return stepDown();
        default:
            return gotoDesk(wParam,FALSE);
        }
        return FALSE ;
        
    case VW_CLOSE: 
        shutDown();
        return TRUE;
        
    case VW_SETUP:
        if(!vwEnabled)
            return FALSE ;
        showSetup();
        return TRUE;
        
    case VW_DELICON:
        taskbarIconShown |= 0x02 ;
        vwIconSet(currentDesk,0) ;
        return TRUE;
        
    case VW_SHOWICON:
        taskbarIconShown &= ~0x02 ;
        vwIconSet(currentDesk,0) ;
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
        if(!vwEnabled)
            return FALSE ;
        if(lParam < 0)
            return assignWindow((HWND) wParam,0-lParam,TRUE,FALSE,FALSE);
        return assignWindow((HWND) wParam,(vwUByte) lParam,FALSE,FALSE,FALSE);
        
    case VW_ACCESSWIN:
        if(!vwEnabled)
            return FALSE ;
        return accessWindow((HWND)wParam,(vwUByte) lParam,FALSE);
        
    case VW_SETSTICKY:
        if(!vwEnabled)
            return FALSE ;
        return setSticky((HWND)wParam,(vwUByte) lParam);
        
    case VW_GETWINDESK:
        {
            vwMutexLock();
            if((ii = winListFind((HWND)wParam)) < 0)
                ii = 0;
            else if((!winList[ii].Visible && (winList[ii].Sticky || (winList[ii].Desk == currentDesk))) ||
                    ((winList[ii].Visible == vwVISIBLE_YES) && !winList[ii].Sticky && (winList[ii].Desk != currentDesk)))
                ii = 0 - (int) winList[ii].Desk ;
            else
                ii = (int) winList[ii].Desk ;
            vwMutexRelease();
            return ii ;
        }
        
    case VW_FOREGDWIN:
        {
            vwMutexLock();
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
            vwMutexRelease();
            return ii ;
        }
    
    case VW_WINLIST:
        {
            // Send over the window list with WM_COPYDATA
            COPYDATASTRUCT cds;         
            DWORD ret ;
            vwMutexLock();
            winListUpdate() ;
            cds.dwData = windowCount;
            cds.cbData = sizeof(winList);
            cds.lpData = (void*)winList;
            if(wParam == 0)
            {
                sendModuleMessage(WM_COPYDATA, (WPARAM) aHWnd, (LPARAM)&cds); 
                ret = TRUE ;
            }
            else if(!SendMessageTimeout((HWND)wParam,WM_COPYDATA,(WPARAM) aHWnd,(LPARAM) &cds,SMTO_ABORTIFHUNG|SMTO_BLOCK,10000,&ret))
                ret = 0 ;
            vwMutexRelease();
            vwLogVerbose((_T("Sent winlist to %d, returning %d\n"),(int) wParam, ret)) ;
            return ret ;
        }
        
    case VW_INSTALLPATH:
    case VW_USERAPPPATH:
        {
            // Send over the VirtuaWin install path with WM_COPYDATA
            //  - always use a byte string so unicode/non-uncode modules can work together
            COPYDATASTRUCT cds;
            DWORD ret ;
            char *ss = (message == VW_INSTALLPATH) ? VirtuaWinPathStr:UserAppPathStr ;
            cds.dwData = 0 - message ;
            cds.cbData = strlen(ss) + 1 ;
            cds.lpData = (void*)ss;
            if(wParam == 0)
            {
                sendModuleMessage(WM_COPYDATA, (WPARAM) aHWnd, (LPARAM)&cds); 
                ret = TRUE ;
            }
            else if(!SendMessageTimeout((HWND)wParam,WM_COPYDATA,(WPARAM) aHWnd,(LPARAM) &cds,SMTO_ABORTIFHUNG|SMTO_BLOCK,10000,&ret))
                ret = 0 ;
            vwLogVerbose((_T("Sent path %d to %d, returning %d\n"),message,(int) wParam, ret)) ;
            return ret ;
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
        lParam = vwEnabled ;
        if((wParam == 1) || ((wParam == 2) && vwEnabled) || ((wParam == 3) && !vwEnabled))
            vwToggleEnabled() ;
        return lParam ;
        
    case VW_DESKNAME:
        {
            // Send over the VirtuaWin install path with WM_COPYDATA
            //  - always use a byte string so unicode/non-uncode modules can work together
            COPYDATASTRUCT cds;
            DWORD ret ;
#ifdef _UNICODE
            char buff[128] ;
#endif
            if(wParam == 0)
                ret = FALSE ;
            else
            {
                ret = TRUE ;
                cds.dwData = 0 - message ;
                if(lParam == 0)
                    lParam = currentDesk ;
                if((lParam >= 0) && (lParam < vwDESKTOP_SIZE) &&
                   (desktopName[lParam] != NULL))
                {
#ifdef _UNICODE
                    if(!WideCharToMultiByte(CP_ACP,0,desktopName[lParam],-1,(char *) buff,128, 0, 0))
                        ret = FALSE ;
                    else
                    {
                        cds.cbData = strlen(buff) + 1 ;
                        cds.lpData = (void *) buff ;
                    }
#else
                    cds.cbData = strlen(desktopName[lParam]) + 1 ;
                    cds.lpData = (void *) desktopName[lParam] ;
#endif
                }
                else
                {
                    cds.cbData = 0 ;
                    cds.lpData = (void*) NULL ;
                }
                if(ret && !SendMessageTimeout((HWND)wParam,WM_COPYDATA,(WPARAM) aHWnd,(LPARAM) &cds,SMTO_ABORTIFHUNG|SMTO_BLOCK,10000,&ret))
                    ret = FALSE ;
            }
            vwLogVerbose((_T("Sent deskname %d to %d, returning %d\n"),(int) lParam,(int) wParam, ret)) ;
            return ret ;
        }
        
    case VW_DESKTOP_SIZE:
        return vwDESKTOP_SIZE;
        
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
        
    case VW_SYSTRAY:		   // We are being notified of mouse activity over the icon
        switch (lParam)
        {
        case WM_LBUTTONDOWN:               // Show the window list
            if(vwEnabled)
                winListPopupMenu(aHWnd,(HIWORD(GetKeyState(VK_CONTROL))) ? 2:winListCompact) ;
            break;
        
        case WM_LBUTTONDBLCLK:             // double click on icon
            if(vwEnabled)
                showSetup();
            break;
            
        case WM_MBUTTONUP:		   // Move to the next desktop
            if(vwEnabled)
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
    
                if(vwEnabled)
                {
                    MENUITEMINFO minfo ;
        
                    AppendMenu(hpopup,MF_STRING,ID_SETUP,_T("&Setup"));
                    minfo.cbSize = sizeof(MENUITEMINFO) ;
                    minfo.fMask = MIIM_STATE ;
                    minfo.fState = MFS_DEFAULT ;
                    SetMenuItemInfo(hpopup,ID_SETUP,FALSE,&minfo) ;
                }
                AppendMenu(hpopup,MF_STRING,ID_GATHER,_T("&Gather"));
                AppendMenu(hpopup,MF_STRING,ID_HELP,_T("&Help"));
                AppendMenu(hpopup,MF_STRING,ID_DISABLE,(vwEnabled) ? _T("&Disable") : _T("&Enable"));
                AppendMenu(hpopup,MF_SEPARATOR,0,NULL) ;
                AppendMenu(hpopup,MF_STRING,ID_EXIT,_T("E&xit"));
                if(vwEnabled)
                {
                    AppendMenu(hpopup,MF_SEPARATOR,0,NULL) ;
                    AppendMenu(hpopup,(deskWrap || (currentDesk < nDesks)) ? MF_STRING:(MF_STRING|MF_GRAYED),ID_FORWARD,_T("&Next"));
                    AppendMenu(hpopup,(deskWrap || (currentDesk > 1)) ? MF_STRING:(MF_STRING|MF_GRAYED),ID_BACKWARD,_T("&Previous"));
                }
                GetCursorPos(&pt);
                SetForegroundWindow(aHWnd);
                switch(TrackPopupMenu(hpopup, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                                      pt.x, pt.y, 0, aHWnd, NULL))
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
                    vwToggleEnabled() ;
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
        if(message == taskbarRestart)
        {
            goGetTheTaskbarHandle();
            vwIconSet(currentDesk,0) ;
        }
        break;
    }
    return DefWindowProc(aHWnd, message, wParam, lParam);
}

static void
VirtuaWinInit(HINSTANCE hInstance, LPSTR cmdLine)
{
    OSVERSIONINFO os;
    WNDCLASSEX wc;
    DWORD threadID;
    vwUByte awStore;
    hInst = hInstance;
    
#ifdef _WIN32_MEMORY_DEBUG
    /* Enable heap checking on each allocate and free */
    _CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF|_CRTDBG_DELAY_FREE_MEM_DF|
                    _CRTDBG_LEAK_CHECK_DF|_CRTDBG_DELAY_FREE_MEM_DF);
#endif
    /* Is this call to VirtuaWin just to send a message to an already ruinning VW? */
    if(strncmp(cmdLine,"-msg",4))
        cmdLine = NULL ;
    else
        cmdLine += 4 ;
    
    /* Only one instance may be started */
    hMutex = CreateMutex(NULL, FALSE, vwVIRTUAWIN_NAME _T("PreventSecond"));
    if(GetLastError() == ERROR_ALREADY_EXISTS)
    {
        UINT message=VW_SETUP ;
        WPARAM wParam=0 ;
        LPARAM lParam=0 ;
        
        if((hWnd = FindWindow(vwVIRTUAWIN_CLASSNAME, NULL)) == NULL)
            exit(-2) ;
        
        /* get the message from the command-line, default is to display configuration window... */
        if(cmdLine != NULL)
            sscanf(cmdLine,"%i %i %li",&message,&wParam,&lParam) ;
        /* post message and quit */
        exit(SendMessage(hWnd,message,wParam,lParam)) ;
    }
    if(cmdLine != NULL)
        /* VW not running, can't post message, return error */
        exit(-1) ;
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
    wc.lpszClassName = vwVIRTUAWIN_CLASSNAME;
    wc.hIconSm = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_VIRTUAWIN), IMAGE_ICON,
                                   GetSystemMetrics(SM_CXSMICON),
                                   GetSystemMetrics(SM_CYSMICON), 0);
    if(RegisterClassEx(&wc) == 0)
    {
        MessageBox(hWnd,_T("Failed to register class!"),vwVIRTUAWIN_NAME _T(" Error"), MB_ICONWARNING);
        exit(1) ;
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
    // Fix some things for the alternate hide method
    RM_Shellhook = RegisterWindowMessage(_T("SHELLHOOK"));
    goGetTheTaskbarHandle();
    getScreenSize();
    getWorkArea();
    
    /* Create window. Note that WS_VISIBLE is not used, and window is never shown. */
    if((hWnd = CreateWindowEx(0, vwVIRTUAWIN_CLASSNAME, vwVIRTUAWIN_CLASSNAME, WS_POPUP, CW_USEDEFAULT, 0,
                              CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL)) == NULL)
    {
        MessageBox(hWnd,_T("Failed to create window!"),vwVIRTUAWIN_NAME _T(" Error"), MB_ICONWARNING);
        exit(2) ;
    }
    /* Create the thread responsible for mouse monitoring */   
    mouseThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) vwMouseProc, NULL, 0, &threadID); 	
    mouseEnabled = TRUE;
    
    nIconD.cbSize = sizeof(NOTIFYICONDATA); // size
    nIconD.hWnd = hWnd;		    // window to receive notifications
    nIconD.uID = 1;		    // application-defined ID for icon (can be any UINT value)
    nIconD.uFlags = NIF_MESSAGE |   // nIconD.uCallbackMessage is valid, use it
          NIF_ICON |		    // nIconD.hIcon is valid, use it
          NIF_TIP;		    // nIconD.szTip is valid, use it
    nIconD.uCallbackMessage = VW_SYSTRAY;  // message sent to nIconD.hWnd
    
    if(trickyWindows)
        loadTrickyList();
    loadStickyList();
    loadAssignedList();
    loadUserList();
    
    vwIconLoad();
    vwHotkeyRegister();
    enableMouse(mouseEnable);
    
    /* always move windows immediately on startup */
    awStore = assignImmediately ;
    assignImmediately = TRUE ;
    vwMutexLock();
    winListUpdate() ;
    vwMutexRelease();
    assignImmediately = awStore ;
    
    /* Load user modules */
    curDisabledMod = loadDisabledModules(disabledModules);
    loadModules();
    
    SetTimer(hWnd, 0x29a, 250, monitorTimerProc); 
}

/*************************************************
 * VirtuaWin start point
 */
int APIENTRY
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg ;
    
    VirtuaWinInit(hInstance,lpCmdLine) ;
    
    /* Main message loop */
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    CloseHandle(hMutex);
    return msg.wParam;
}
