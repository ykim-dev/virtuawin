//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K/XP
// 
//  This is a module for VirtuaWin that adds hot key support
//  for moving the current active window to next or previous desktop
//  
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

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include <commctrl.h>

#include "assignerres.h"
#include "../../Messages.h"

int initialised=0 ;
HINSTANCE hInst;   // Instance handle
HWND hwndMain;	   // Main window handle
HWND vwHandle;     // Handle to VirtuaWin
ATOM hotKeyNext;
ATOM hotKeyPrev;
UINT HOT_NEXT;
UINT HOT_NEXT_MOD;
UINT HOT_NEXT_WIN;
UINT HOT_PREV;
UINT HOT_PREV_MOD;
UINT HOT_PREV_WIN;
WORD CHANGE_DESKTOP=BST_UNCHECKED;
TCHAR *configFile;
UINT numberOfDesktops;


/*************************************************
 * Translates virtual key codes to "hotkey codes"
 */
UINT hotKey2ModKey(UINT vModifiers)
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
 * Register the assignment hotkey
 */
static void registerAssignment(void)
{
    if(HOT_NEXT)
    {
        hotKeyNext = GlobalAddAtom(_T("AssignmentNext"));
        if(RegisterHotKey(hwndMain, hotKeyNext, hotKey2ModKey(HOT_NEXT_MOD) | HOT_NEXT_WIN, HOT_NEXT) == 0)
            MessageBox(hwndMain,_T("Invalid key modifier combination, check hot keys!"),
                       _T("VWAssigner Error"), MB_ICONWARNING);
    }
    if(HOT_PREV)
    {
        hotKeyPrev = GlobalAddAtom(_T("AssignmentPrev"));
        if(RegisterHotKey(hwndMain, hotKeyPrev, hotKey2ModKey(HOT_PREV_MOD) | HOT_PREV_WIN, HOT_PREV) == 0)
            MessageBox(hwndMain,_T("Invalid key modifier combination, check hot keys!"), 
                       _T("VWAssigner Error"), MB_ICONWARNING);
    }
}

/*************************************************
 * Un-register the assignment hotkey
 */
static void unregisterAssignment(void)
{
    UnregisterHotKey(hwndMain, hotKeyNext);
    UnregisterHotKey(hwndMain, hotKeyPrev);
}

//*************************************************

static void loadSettings(void)
{
    char dummy[80] ;
    FILE* fp;
    
    if((fp = _tfopen(configFile,_T("r"))))
    {
        fscanf(fp, "%s%i", dummy, &HOT_NEXT_MOD);
        fscanf(fp, "%s%i", dummy, &HOT_NEXT);
        fscanf(fp, "%s%i", dummy, &HOT_PREV_MOD);
        fscanf(fp, "%s%i", dummy, &HOT_PREV);
        if(fscanf(fp, "%s%i", dummy, &HOT_NEXT_WIN) == 2)
        {
            fscanf(fp, "%s%i", dummy, &HOT_PREV_WIN);
            fscanf(fp, "%s%hd", dummy, &CHANGE_DESKTOP);
        }
        fclose(fp);
    }
}

//*************************************************

static void saveSettings(void)
{
    FILE* fp;
    if(!(fp = _tfopen(configFile,_T("w")))) 
    {
        MessageBox(hwndMain,_T("Error writing config file"),_T("VWAssigner Error"), MB_ICONWARNING);
    } 
    else 
    {
        fprintf(fp, "next_mod# %i\n", HOT_NEXT_MOD);
        fprintf(fp, "next# %i\n", HOT_NEXT);
        fprintf(fp, "prev_mod# %i\n", HOT_PREV_MOD);
        fprintf(fp, "prev# %i\n", HOT_PREV);
        fprintf(fp, "next_win# %i\n", HOT_NEXT_WIN);
        fprintf(fp, "prev_win# %i\n", HOT_PREV_WIN);
        fprintf(fp, "change_desktop# %d\n", CHANGE_DESKTOP);
        fclose(fp);
    }
}

//*************************************************
// This is the main function for the dialog. 

static BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WORD wRawHotKey;
    
    switch (msg) {
    case WM_INITDIALOG:
        unregisterAssignment();
        SendDlgItemMessage(hwndDlg, IDC_HOTNEXT, HKM_SETHOTKEY, 
                           MAKEWORD(HOT_NEXT, HOT_NEXT_MOD), 0);
        SendDlgItemMessage(hwndDlg, IDC_HOTNEXTW, BM_SETCHECK, (HOT_NEXT_WIN != 0),0);
        SendDlgItemMessage(hwndDlg, IDC_HOTPREV, HKM_SETHOTKEY, 
                           MAKEWORD(HOT_PREV, HOT_NEXT_MOD), 0);
        SendDlgItemMessage(hwndDlg, IDC_HOTPREVW, BM_SETCHECK, (HOT_PREV_WIN != 0),0);
        SendDlgItemMessage(hwndDlg, IDC_CHNGDESK, BM_SETCHECK, CHANGE_DESKTOP, 0 );
        return TRUE;
        
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            wRawHotKey = (WORD)SendDlgItemMessage(hwndDlg, IDC_HOTNEXT, HKM_GETHOTKEY, 0, 0);
            HOT_NEXT = LOBYTE(wRawHotKey);
            HOT_NEXT_MOD = HIBYTE(wRawHotKey);
            if(SendDlgItemMessage(hwndDlg, IDC_HOTNEXTW, BM_GETCHECK, 0, 0) == BST_CHECKED)
                HOT_NEXT_WIN = MOD_WIN;
            else
                HOT_NEXT_WIN = FALSE;
            
            wRawHotKey = (WORD)SendDlgItemMessage(hwndDlg, IDC_HOTPREV, HKM_GETHOTKEY, 0, 0);
            HOT_PREV = LOBYTE(wRawHotKey);
            HOT_PREV_MOD = HIBYTE(wRawHotKey);
            if(SendDlgItemMessage(hwndDlg, IDC_HOTPREVW, BM_GETCHECK, 0, 0) == BST_CHECKED)
                HOT_PREV_WIN = MOD_WIN;
            else
                HOT_PREV_WIN = FALSE;
            
            CHANGE_DESKTOP = (WORD)SendDlgItemMessage(hwndDlg, IDC_CHNGDESK, BM_GETCHECK, 0, 0);
            
            saveSettings();
            unregisterAssignment();
            registerAssignment();
            EndDialog(hwndDlg,0);
            return 1;
        case IDCANCEL:
            registerAssignment();
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

/* Initializes the app */ 
static BOOL InitApplication(HWND hwnd, char *userAppPath)
{
    TCHAR buff[MAX_PATH];
    
    InitCommonControls();
#ifdef _UNICODE
    MultiByteToWideChar(CP_ACP,0,(char *) userAppPath,-1,buff,MAX_PATH) ;
#else
    strcpy(buff,userAppPath) ;
#endif
    _tcscat(buff,_T("vwassigner.cfg")) ;
    if((configFile = _tcsdup(buff)) == NULL)
    {
        MessageBox(hwnd,_T("Malloc failure"),_T("VWAssigner Error"), MB_ICONWARNING);
        exit(1) ;
    }
    loadSettings();
    registerAssignment();
    
    return 1;
}

static VOID CALLBACK startupFailureTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    if(!initialised)
    {
        MessageBox(hwnd,_T("VirtuaWin failed to send the UserApp path."),_T("VWAssigner Error"), MB_ICONWARNING);
        exit(1) ;
    }
}

//*************************************************

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND theActive = NULL;
    COPYDATASTRUCT *cds;         
    int theCurDesk, theNewDesk;
    int deskX, deskY;
    
    switch (msg)
    {
    case WM_HOTKEY:
        // Get the current desktop
        theCurDesk = SendMessage(vwHandle, VW_CURDESK, 0, 0);
        // Get the active window
        theActive = GetForegroundWindow();
        if(theActive)
        {
            if(wParam == hotKeyNext)
                theNewDesk = theCurDesk+1 ;
            else if(wParam == hotKeyPrev)
                theNewDesk = theCurDesk-1 ;
            else
                break ;
            if(theNewDesk <= 0)
                theNewDesk = numberOfDesktops ;
            else if(theNewDesk > numberOfDesktops)
                theNewDesk = 1 ;
            if(CHANGE_DESKTOP == BST_CHECKED)
                theNewDesk = 0 - theNewDesk ;
            SendMessage(vwHandle, VW_ASSIGNWIN, (WPARAM)theActive, theNewDesk) ;
        }
        break;  
    
    case WM_COPYDATA:
        cds = (COPYDATASTRUCT *) lParam ;         
        if((cds->dwData == (0-VW_USERAPPPATH)) && !initialised)
        {
            initialised = 1 ;
            KillTimer(hwnd,0x29a) ;
            if((cds->cbData < 2) || (cds->lpData == NULL))
            {
                MessageBox(hwnd,_T("VirtuaWin returned a bad UserApp path."),_T("VWAssigner Error"), MB_ICONWARNING);
                exit(1) ;
            }
            InitApplication(hwnd,(char *) cds->lpData) ;
        }
        return TRUE ;
        
    case MOD_INIT: // This must be taken care of in order to get the handle to VirtuaWin. 
        // The handle to VirtuaWin comes in the wParam 
        vwHandle = (HWND) wParam; // Should be some error handling here if NULL 
        deskY = SendMessage(vwHandle, VW_DESKY, 0, 0);
        deskX = SendMessage(vwHandle, VW_DESKX, 0, 0);
        numberOfDesktops = deskX * deskY;
        if(!initialised)
        {
            // Get the user path - give VirtuaWin 10 seconds to do this
            SendMessage(vwHandle, VW_USERAPPPATH, 0, 0);
            SetTimer(hwnd, 0x29a, 10000, startupFailureTimerProc);
        }
        break;
    
    case MOD_CFGCHANGE:
        deskY = SendMessage(vwHandle, VW_DESKY, 0, 0);
        deskX = SendMessage(vwHandle, VW_DESKX, 0, 0);
        numberOfDesktops = deskX * deskY;
        break;
    
    case MOD_QUIT: // This must be handeled, otherwise VirtuaWin can't shut down the module 
        PostQuitMessage(0);
        break;
    
    case MOD_SETUP:
        DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), vwHandle, (DLGPROC) DialogFunc);
        break;
    
    case WM_DESTROY:
        unregisterAssignment();
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
    WNDCLASS wc;
    
    hInst = hInstance;
    memset(&wc, 0, sizeof(WNDCLASS));
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)MainWndProc;
    wc.hInstance = hInst;
    /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
       this for locating the window */
    wc.lpszClassName = _T("VWAssigner.exe") ;
    
    if (!RegisterClass(&wc))
        return 0;
    
    // the window is never shown
    if ((hwndMain = CreateWindow(_T("VWAssigner.exe"), 
                                 _T("VWAssigner"), 
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
