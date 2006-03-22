//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K/XP
// 
//  This is a module for VirtuaWin that are used for 
//  restoring lost windows. 
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


#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "winlistres.h"
#include "../Defines.h"
#include "../Messages.h"

HINSTANCE hInst;   // Instance handle
HWND hwndMain;	   // Main window handle
HWND vwHandle;     // Handle to VirtuaWin

typedef struct {
    HWND handle;
    RECT rect;
    int style;
    int exstyle;
} winType;

winType windowList[999]; // should be enough
int noOfWin;
int curSel;
int screenLeft;
int screenRight;
int screenTop;
int screenBottom;
UINT RM_Shellhook;
HWND hwndTask;          // handle to taskbar

/* prototype for the dialog box function. */
static BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/* Main message handler */
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

/* Initializes the window */ 
static BOOL InitApplication(void)
{
    WNDCLASS wc;
    
    memset(&wc, 0, sizeof(WNDCLASS));
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)MainWndProc;
    wc.hInstance = hInst;
    /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
       this for locating the window */
    wc.lpszClassName = "WinList.exe";
    
    if (!RegisterClass(&wc))
        return 0;
    
    return 1;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        
    case MOD_INIT: // This must be taken care of in order to get the handle to VirtuaWin. 
        // The handle to VirtuaWin comes in the wParam 
        vwHandle = (HWND) wParam; // Should be some error handling here if NULL 
        break;
    case MOD_QUIT: // This must be handeled, otherwise VirtuaWin can't shut down the module 
        PostQuitMessage(0);
        break;
    case MOD_SETUP: // Optional
        DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), vwHandle, (DLGPROC) DialogFunc);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    return 0;
}

/*
 * Main startup function
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    MSG msg;
    hInst = hInstance;
    if (!InitApplication())
        return 0;
    
    // the window is never shown
    if ((hwndMain = CreateWindow("WinList.exe", 
                                 "WinList", 
                                 WS_POPUP,
                                 CW_USEDEFAULT, 
                                 0, 
                                 CW_USEDEFAULT, 
                                 0,
                                 NULL,
                                 NULL,
                                 hInst,
                                 NULL)) == (HWND)0)
        return 0;
    
    
    // main messge loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

/*************************************************
 * Callback function. Integrates all enumerated windows
 */
__inline BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam) 
{
    RECT rect ;
    int style = GetWindowLong(hwnd, GWL_STYLE);
    int exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    
    GetWindowRect(hwnd,&rect);
    
    if(!(style & WS_CHILD) && (!GetParent(hwnd) || GetParent(hwnd) == GetDesktopWindow()) &&
       (!(exstyle & WS_EX_TOOLWINDOW) || (rect.left <= -10000) || (rect.top <= -10000)))
    {
        char *ss, buff[vwCLASSNAME_MAX+vwCLASSNAME_MAX+4];
        ss = buff ;
        
        // Add window to the windowlist
        // If at the hidden position then flag as likely lost
        if((rect.left <= -10000) && (rect.top <= -10000) &&
           (rect.left >= -30000) && (rect.top >= -30000))
            *ss++ = '*' ;
        else if(style & WS_VISIBLE)
            *ss++ = ' ' ;
        else
            *ss++ = 'H' ;
        *ss++ = '\t' ;
        GetClassName(hwnd,ss,vwCLASSNAME_MAX);
        ss[vwCLASSNAME_MAX] = '\0' ;
        ss += strlen(ss) ;
        *ss++ = '\t' ;
        if(GetWindowText(hwnd,ss,vwCLASSNAME_MAX))
            ss[vwCLASSNAME_MAX] = '\0' ;
        else
            strcpy(ss, "<None>");
        SendDlgItemMessage((HWND) lParam, ID_WINLIST, LB_ADDSTRING, 0, (LONG) buff);
        windowList[noOfWin].handle = hwnd;
        windowList[noOfWin].style = style;
        windowList[noOfWin].exstyle = exstyle;
        windowList[noOfWin].rect = rect ;
        noOfWin++;
    }
    
    return TRUE;
}

static void goGetTheTaskbarHandle(HWND hDlg)
{
    HWND hwndTray = FindWindowEx(NULL, NULL, "Shell_TrayWnd", NULL);
    HWND hwndBar = FindWindowEx(hwndTray, NULL, "ReBarWindow32", NULL );
    
    // Maybe "RebarWindow32" is not a child to "Shell_TrayWnd", then try this
    if( hwndBar == NULL )
        hwndBar = hwndTray;
    
    hwndTask = FindWindowEx(hwndBar, NULL, "MSTaskSwWClass", NULL);
    
    if( hwndTask == NULL )
        MessageBox(hDlg, "Could not locate handle to the taskbar.\n This will disable the ability to hide troublesome windows correctly.", "VirtuaWinList", 0); 
}

/*
 */
static int InitializeApp(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    int tabstops[2] ;
    screenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
    screenRight = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    screenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
    screenBottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    SendDlgItemMessage(hDlg,ID_WINLIST,LB_SETHORIZONTALEXTENT,vwCLASSNAME_MAX+vwCLASSNAME_MAX+4, 0);
    tabstops[0] = 14 ;
    tabstops[1] = 140 ;
    SendDlgItemMessage(hDlg,ID_WINLIST,LB_SETTABSTOPS,(WPARAM)2,(LPARAM)tabstops);
    noOfWin = 0;
    EnumWindows(enumWindowsProc, (LPARAM) hDlg);   // get all windows
    RM_Shellhook = RegisterWindowMessage("SHELLHOOK");
    goGetTheTaskbarHandle(hDlg) ;
    curSel = LB_ERR;
    return 1;
}

/*
   This is the main function for the dialog. 
 */
static BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        InitializeApp(hwndDlg, wParam, lParam);
        return TRUE;
        
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            curSel = SendDlgItemMessage(hwndDlg, ID_WINLIST, LB_GETCURSEL, 0, 0);
            if(curSel != LB_ERR)
            {
                int left, top;
                if((windowList[curSel].style & WS_VISIBLE) == 0)
                {
                    ShowWindow(windowList[curSel].handle, SW_SHOWNA);
                    ShowOwnedPopups(windowList[curSel].handle, SW_SHOWNA);
                }
                if(windowList[curSel].exstyle & WS_EX_TOOLWINDOW)
                {
                    // Restore the window mode
                    SetWindowLong(windowList[curSel].handle, GWL_EXSTYLE, (windowList[curSel].exstyle & (~WS_EX_TOOLWINDOW))) ;  
                    // Notify taskbar of the change
                    PostMessage(hwndTask, RM_Shellhook, 1, (LPARAM) windowList[curSel].handle);
                }
                left = windowList[curSel].rect.left ;
                top = windowList[curSel].rect.top ;
                if((left == -22000) && (top == -22000))
                {
                    /* some apps hide the window by pushing it to -32000,
                     * VirtuaWin handles these by moving to -22000 when it
                     * 'hides' them. Restore it back to -32000 first, if the
                     * user closes WinList, reopens and selects Restore again
                     * on this window it will be moved to the visible area */
                    left = -32000 ;
                    top = -32000 ;
                }
                else
                {
                    if(left < -10000)
                        left += 20000 ;
                    if(top < -10000)
                        top += 20000 ;
                    if(left < screenLeft)
                        left = screenLeft + 10 ;
                    else if(left > screenRight)
                        left = screenLeft + 10 ;
                    if(top < screenTop)
                        top = screenTop + 10 ;
                    else if(top > screenBottom)
                        top = screenTop + 10 ;
                }
                SetWindowPos(windowList[curSel].handle, 0, left, top, 0, 0,
                             SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE ); 
                SetForegroundWindow(windowList[curSel].handle);
            }
            return 1;
        case IDUNDO:
            if(curSel != LB_ERR)
            {
                if((windowList[curSel].style & WS_VISIBLE) == 0)
                {
                    ShowWindow(windowList[curSel].handle, SW_HIDE);
                    ShowOwnedPopups(windowList[curSel].handle, SW_HIDE);
                }
                if(windowList[curSel].exstyle & WS_EX_TOOLWINDOW)
                {
                    // Restore the window mode
                    SetWindowLong(windowList[curSel].handle, GWL_EXSTYLE, windowList[curSel].exstyle) ;  
                    // Notify taskbar of the change
                    PostMessage( hwndTask, RM_Shellhook, 2, (LPARAM) windowList[curSel].handle);
                }
                SetWindowPos(windowList[curSel].handle, 0, windowList[curSel].rect.left, windowList[curSel].rect.top, 0, 0,
                             SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE ); 
                curSel = LB_ERR;
            }
            else
                MessageBox(vwHandle, "Cannot undo", "Undo", 0);
            return 1;
        case IDCANCEL:
            EndDialog(hwndDlg,0);
            return 1;
        }
        break;
        
    case WM_CLOSE:
        EndDialog(hwndDlg,0);
        return TRUE;
	
    }
    return FALSE;
}

/*
 * $Log$
 * Revision 1.4  2003/01/27 20:26:01  jopi
 * Updated copyright header
 *
 * Revision 1.3  2001/02/05 21:24:45  jopi
 * Updated copyright header
 *
 * Revision 1.2  2000/12/11 19:59:26  jopi
 * Added classname for all applications
 *
 *
 */
