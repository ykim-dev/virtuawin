/*
 *  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
 *  VWTimeTracker.c - Module to display the time spent on each desk.
 * 
 *  Copyright (c) 2006-2007 VirtuaWin (VirtuaWin@home.se)
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

#define MAX_DESK 32
int deskCrrnt ;
time_t timeTotal[MAX_DESK+1] ;
time_t timeCrrnt[MAX_DESK+1] ;
time_t timePause=0 ;
time_t timeLast=0 ;

static void
UpdateTime(void)
{
    if(!timePause)
    {
        time_t timec = time(NULL) ;
        if(timeLast > 0)
            timeCrrnt[deskCrrnt] += timec - timeLast ;
        timeLast = timec ;
    }
}

static int GenerateTimerList(HWND hDlg)
{
    TCHAR buff[30];
    int ii, tabstops[2] ;
    time_t tt, totals[2] ;
    
    
    SendDlgItemMessage(hDlg,ID_TIMELIST,LB_RESETCONTENT,0,0);
    SendDlgItemMessage(hDlg,ID_TIMELIST,LB_SETHORIZONTALEXTENT,30,0);
    tabstops[0] = 100 ;
    tabstops[1] = 150 ;
    SendDlgItemMessage(hDlg,ID_TIMELIST,LB_SETTABSTOPS,(WPARAM)2,(LPARAM)tabstops);
    
    SendDlgItemMessage(hDlg,ID_TIMELIST,LB_ADDSTRING,0,(LONG) _T("Desk\tCurrent\tTotal")) ;
    SendDlgItemMessage(hDlg,ID_TIMELIST,LB_ADDSTRING,0,(LONG) _T("-------------\t--------------\t--------------")) ;
    
    totals[0] = totals[1] = 0 ;
    for(ii = 0 ; ii <= MAX_DESK ; ii++)
    {
        if((tt=timeTotal[ii] + timeCrrnt[ii]) > 0)
        {
            _stprintf(buff,_T("Desk %d\t%d:%02d:%02d\t%d:%02d:%02d"),ii,
                      (int) (timeCrrnt[ii]/3600),(int) ((timeCrrnt[ii]/60)%60),(int) (timeCrrnt[ii]%60),
                      (int) (tt/3600),(int) ((tt/60)%60),(int) (tt%60)) ;
            SendDlgItemMessage(hDlg,ID_TIMELIST,LB_ADDSTRING,0,(LONG) buff);
            totals[0] += timeCrrnt[ii] ;
            totals[1] += timeTotal[ii] ;
        }
    }
    SendDlgItemMessage(hDlg,ID_TIMELIST,LB_ADDSTRING,0,(LONG) _T("-------------\t--------------\t--------------")) ;
    tt = totals[0] + totals[1] ;
    _stprintf(buff,_T("Total\t%d:%02d:%02d\t%d:%02d:%02d"),
              (int) (totals[0]/3600),(int) ((totals[0]/60)%60),(int) (totals[0]%60),
              (int) (tt/3600),(int) ((tt/60)%60),(int) (tt%60)) ;
    SendDlgItemMessage(hDlg,ID_TIMELIST,LB_ADDSTRING,0,(LONG) buff);
    return 1;
}


static VOID CALLBACK
monitorScreensaverTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
#ifndef SPI_GETSCREENSAVERRUNNING
#define SPI_GETSCREENSAVERRUNNING 114
#endif
    BOOL active;  
    
    SystemParametersInfo(SPI_GETSCREENSAVERRUNNING,0,(LPVOID) &active,0) ; 
    if(active)
    {
        if(!timePause)
        {
            UpdateTime() ;
            timePause = 2 ;
            timeLast = 0 ;
        }
    }
    else if(timePause == 2)
    {
        timePause = 0 ;
        timeLast = 0 ;
        UpdateTime() ;
    }
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
        SetDlgItemText(hwndDlg, ID_STOP, (timePause) ? _T("Start"):_T("Stop")) ;
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
            if(timePause)
            {
                timePause = 0 ;
                timeLast = 0 ;
                UpdateTime() ;
            }
            else
            {
                UpdateTime() ;
                timePause = 1 ;
                timeLast = 0 ;
                GenerateTimerList(hwndDlg);
            }
            SetDlgItemText(hwndDlg, ID_STOP, (timePause) ? _T("Start"):_T("Stop")) ;
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
        break;
    
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
        if(wParam != 0)
            hwnd = (HWND) wParam ;
        else
            hwnd = (HWND) wHnd ;
        SetForegroundWindow(hwnd) ;
        DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), hwnd, (DLGPROC) DialogFunc);
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
