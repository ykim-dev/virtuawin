/*
 *  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
 *  Module.c - Example user module for VirtuaWin.
 * 
 *  Copyright (c) 1999-2005 Johan Piculell
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
#include "Messages.h"

int initialised=0 ;
HWND vwHandle;                 /* Handle to VirtuaWin */
TCHAR installPath[MAX_PATH] ;  /* VW install path */
TCHAR userAppPath[MAX_PATH] ;  /* User's config path */


static VOID CALLBACK
startupFailureTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    if(!initialised)
    {
        MessageBox(hwnd,_T("VirtuaWin failed to send the UserApp path."),_T("Module Error"), MB_ICONWARNING);
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
        if(!initialised)
        {
            /* Get the VW Install path and then the user's path - give VirtuaWin 10 seconds to do this */
            SendMessage(vwHandle, VW_INSTALLPATH, 0, 0);
            SetTimer(hwnd, 0x29a, 10000, startupFailureTimerProc);
        }
        break;
    
    case WM_COPYDATA:
        if(!initialised)
        {
            COPYDATASTRUCT *cds;         
            cds = (COPYDATASTRUCT *) lParam ;         
            if(cds->dwData == (0-VW_INSTALLPATH))
            {
                if((cds->cbData < 2) || (cds->lpData == NULL))
                {
                    MessageBox(hwnd,_T("VirtuaWin returned a bad Install path."),_T("Module Error"), MB_ICONWARNING);
                    exit(1) ;
                }
                /* the paths are always returned in a multibyte string so we do not need to know
                 * whether we are talking to a unicode VW */
#ifdef _UNICODE
                MultiByteToWideChar(CP_ACP,0,(char *) cds->lpData,-1,installPath,MAX_PATH) ;
#else
                strcpy(installPath,(char *) cds->lpData) ;
#endif
                /* Now get the VW user's path */
                SendMessage(vwHandle, VW_USERAPPPATH, 0, 0);
            }
            else if(cds->dwData == (0-VW_USERAPPPATH))
            {
                TCHAR buff[MAX_PATH+MAX_PATH];
                if((cds->cbData < 2) || (cds->lpData == NULL))
                {
                    MessageBox(hwnd,_T("VirtuaWin returned a bad UserApp path."),_T("Module Error"), MB_ICONWARNING);
                    exit(1) ;
                }
#ifdef _UNICODE
                MultiByteToWideChar(CP_ACP,0,(char *) cds->lpData,-1,userAppPath,MAX_PATH) ;
#else
                strcpy(userAppPath,(char *) cds->lpData) ;
#endif
                initialised = 1 ;
                _stprintf(buff,_T("VirtuaWin Module initialized, install path:\n\t%s\nUser path:\n\t%s"),installPath,userAppPath) ;
                MessageBox(hwnd,buff,_T("Module Plugin"),0);
            }
        }
        return TRUE ;
        
    case MOD_QUIT:
        /* This must be handled, otherwise VirtuaWin can't shut down the module */
        PostQuitMessage(0);
        break;
    case MOD_SETUP:
        /* Optional */
        MessageBox(hwnd,_T("Add setup here!"),_T("Module Plugin"), 0);
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
    
    memset(&wc, 0, sizeof(WNDCLASS));
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)MainWndProc;
    wc.hInstance = hInstance ;
    /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
       this for locating the window */
    wc.lpszClassName = _T("Module.exe");
  
    if (!RegisterClass(&wc))
        return 0;
  
    /* In this example, the window is never shown */
    if (CreateWindow(_T("Module.exe"), 
                     _T("Module"), 
                     WS_POPUP,
                     CW_USEDEFAULT, 
                     0, 
                     CW_USEDEFAULT, 
                     0,
                     NULL,
                     NULL,
                     hInstance,
                     NULL) == (HWND) 0)
        return 0;
    
    /* main messge loop */
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
