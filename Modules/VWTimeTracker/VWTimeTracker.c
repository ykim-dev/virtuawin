/*
 *  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
 *  Module.c - Example user module for VirtuaWin.
 * 
 *  Copyright (c) 1999-2005 Johan Piculell
 *  Copyright (c) 2006 VirtuaWin (VirtuaWin@home.se)
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 *  USA.
 * 
 *************************************************************************
 * 
 * Simple module skeleton for VirtuaWin. These are the minimal requirements.
 * It is a simple application with a hidden window that receives messages from virtuawin
 * Look in Messages.h to see what can be sent to and from VirtuaWin
 * 
 * Note that the classname must be the same as the filename including the '.exe'
 */
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>
#include "../../Messages.h"
#include "Resource.h"

int initialised=0 ;
HINSTANCE hInst;               /* Instance handle */
HWND wHnd;                     /* Time tracker Handle */
HWND vwHandle;                 /* Handle to VirtuaWin */
TCHAR userAppPath[MAX_PATH] ;  /* User's config path */

#define MAX_DESK 32
int deskCrrnt ;
time_t timeTotal[MAX_DESK+1] ;
time_t timeCrrnt[MAX_DESK+1] ;
time_t timeLast=1 ;

static void
UpdateTime(void)
{
    if(timeLast)
    {
        time_t timec = time(NULL) ;
        if(timeLast > 1)
            timeCrrnt[deskCrrnt] += timec - timeLast ;
        timeLast = timec ;
    }
}

static int GenerateTimerList(HWND hDlg)
{
    TCHAR buff[30];
    int ii, tabstops[2] ;
    time_t tt ;
    
    
    SendDlgItemMessage(hDlg,ID_TIMELIST,LB_RESETCONTENT,0,0);
    SendDlgItemMessage(hDlg,ID_TIMELIST,LB_SETHORIZONTALEXTENT,30,0);
    tabstops[0] = 100 ;
    tabstops[1] = 150 ;
    SendDlgItemMessage(hDlg,ID_TIMELIST,LB_SETTABSTOPS,(WPARAM)2,(LPARAM)tabstops);
    
    SendDlgItemMessage(hDlg,ID_TIMELIST,LB_ADDSTRING,0,(LONG) _T("Desk\tCurrent\tTotal")) ;
    SendDlgItemMessage(hDlg,ID_TIMELIST,LB_ADDSTRING,0,(LONG) _T("_______\t__________\t__________")) ;
    SendDlgItemMessage(hDlg,ID_TIMELIST,LB_ADDSTRING,0,(LONG) _T("")) ;
    
    for(ii = 0 ; ii <= MAX_DESK ; ii++)
    {
        if((tt=timeTotal[ii] + timeCrrnt[ii]) > 0)
        {
            _stprintf(buff,_T("Desk %d\t%d:%02d:%02d\t%d:%02d:%02d"),ii,
                      timeCrrnt[ii]/3600,(timeCrrnt[ii]/60)%60,timeCrrnt[ii]%60,tt/3600,(tt/60)%60,tt%60) ;
            SendDlgItemMessage(hDlg,ID_TIMELIST,LB_ADDSTRING,0,(LONG) buff);
        }
    }
    return 1;
}

/*
   This is the main function for the dialog. 
 */
static BOOL CALLBACK
DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int ii ;
    switch (msg) {
    case WM_INITDIALOG:
        UpdateTime() ;
        GenerateTimerList(hwndDlg);
        SetDlgItemText(hwndDlg, ID_STOP, (timeLast) ? _T("Stop"):_T("Start")) ;
        return TRUE;
        
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwndDlg,0);
            return 1;
        
        case ID_RESET:
            ii = MAX_DESK ;
            do {
                timeTotal[ii] += timeCrrnt[ii] ;
                timeCrrnt[ii] = 0 ;
            } while(--ii >= 0) ;
            return 1;
        
        case ID_STOP:
            if(timeLast)
                timeLast = 0 ;
            else
            {
                timeLast = 1 ;
                UpdateTime() ;
            }
            SetDlgItemText(hwndDlg, ID_STOP, (timeLast) ? _T("Stop"):_T("Start")) ;
            return 1;
            
        case ID_UPDATE:
            UpdateTime() ;
            GenerateTimerList(hwndDlg);
            return 1;
        }
        break;
        
    case WM_CLOSE:
        EndDialog(hwndDlg,0);
        return TRUE;
	
    }
    return FALSE;
}

static VOID CALLBACK
startupFailureTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    if(!initialised)
    {
        MessageBox(hwnd,_T("VirtuaWin failed to send the UserApp path."),_T("VWTimeTracker Error"), MB_ICONWARNING);
        exit(1) ;
    }
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case MOD_INIT: 
        /* This must be taken care of in order to get the handle to VirtuaWin. */
        /* The handle to VirtuaWin comes in the wParam */
        vwHandle = (HWND) wParam; /* Should be some error handling here if NULL */
        UpdateTime() ;
        if((deskCrrnt = SendMessage(vwHandle,VW_CURDESK,0,0)) > MAX_DESK)
            deskCrrnt = 0 ;
        if(!initialised)
        {
            /* Get the VW Install path and then the user's path - give VirtuaWin 10 seconds to do this */
            SendMessage(vwHandle, VW_USERAPPPATH, 0, 0);
            SetTimer(hwnd, 0x29a, 10000, startupFailureTimerProc);
        }
        break;
    
    case WM_COPYDATA:
        if(!initialised)
        {
            COPYDATASTRUCT *cds;         
            cds = (COPYDATASTRUCT *) lParam ;         
            if(cds->dwData == (0-VW_USERAPPPATH))
            {
                if((cds->cbData < 2) || (cds->lpData == NULL))
                {
                    MessageBox(hwnd,_T("VirtuaWin returned a bad UserApp path."),_T("VWTimeTracker Error"), MB_ICONWARNING);
                    exit(1) ;
                }
#ifdef _UNICODE
                MultiByteToWideChar(CP_ACP,0,(char *) cds->lpData,-1,userAppPath,MAX_PATH) ;
#else
                strcpy(userAppPath,(char *) cds->lpData) ;
#endif
                initialised = 1 ;
            }
        }
        return TRUE ;
        
     case MOD_CHANGEDESK:
        UpdateTime() ;
        if(lParam > MAX_DESK)
            deskCrrnt = 0 ;
        else
            deskCrrnt = lParam ;
        return TRUE ;
   case MOD_QUIT:
        /* This must be handled, otherwise VirtuaWin can't shut down the module */
        PostQuitMessage(0);
        break;
    case MOD_SETUP:
        SetForegroundWindow(wHnd) ;
        DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), wHnd, (DLGPROC) DialogFunc);
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
int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    WNDCLASS wc;
    MSG msg;
    
    hInst = hInstance;
    memset(&wc, 0, sizeof(WNDCLASS));
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)MainWndProc;
    wc.hInstance = hInstance ;
    /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
       this for locating the window */
    wc.lpszClassName = _T("VWTimeTracker.exe");
  
    if(!RegisterClass(&wc))
        return 0;
  
    /* In this example, the window is never shown */
    if((wHnd=CreateWindow(_T("VWTimeTracker.exe"), 
                          _T("VWTimeTracker"), 
                          WS_POPUP,
                          CW_USEDEFAULT, 
                          0, 
                          CW_USEDEFAULT, 
                          0,
                          NULL,
                          NULL,
                          hInstance,
                          NULL)) == (HWND) 0)
        return 0;
    
    /* main messge loop */
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
