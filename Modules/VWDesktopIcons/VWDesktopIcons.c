/*
 *  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
 *  VWDesktopIcons.c - Module to control which icons are displayed on each desktop.
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
#include <commctrl.h>
#include "../../Messages.h"
#include "Resource.h"

typedef struct vwIcon {
    struct vwIcon *next ;
    TCHAR name[1] ;
} vwIcon ;

#define diItemSize (sizeof(LVITEM) + sizeof(POINT) + (sizeof(TCHAR) * MAX_PATH))

int initialised=0 ;
HINSTANCE hInst;               /* Instance handle */
HWND wHnd;                     /* Time tracker Handle */
HWND vwHandle;                 /* Handle to VirtuaWin */
TCHAR userAppPath[MAX_PATH] ;  /* User's config path */
HWND    diHandle;              /* Handle to SystemList containing icons */
HANDLE  diProcess;
LVITEM *diLocalItem ;
char   *diShareItem ;
TCHAR  *diItemName ;
POINT  *diItemPos ;

#define MAX_DESK 32
int     deskCrrnt ;
vwIcon *iconHideHead[MAX_DESK+1] ;

static void
loadConfigFile(void)
{
    vwIcon *ci, *ni, *pi ;
    TCHAR buff[1024], *ss ;
    int desk, len ;
    FILE *fp ;
    
    ss = userAppPath + _tcslen(userAppPath) ;
    _tcscpy(ss,_T("vwdesktopicons.cfg")) ;
    fp = _tfopen(userAppPath,_T("r")) ;
    *ss = '\0' ;
    if(fp != NULL)
    {
        while(_fgetts(buff,1024,fp) != NULL)
        {
            ss = NULL ;
            if((buff[0] != ':') &&
               ((desk = (unsigned short) _ttoi(buff)) > 0) && (desk <= MAX_DESK) &&
               ((ss=_tcschr(buff,' ')) != NULL) &&
               (*++ss == 'H') && (*++ss == ' ') &&
               ((len = _tcslen(++ss)) > 1) &&
               ((ci = malloc(sizeof(vwIcon)+(len*sizeof(TCHAR)))) != NULL))
            {
                if(ss[len-1] == '\n')
                    ss[--len] = '\0' ;
                pi = NULL ;
                ni = iconHideHead[desk] ;
                while((ni != NULL) && (_tcscmp(ni->name,ss) < 0))
                {
                    pi = ni ;
                    ni = ni->next ;
                }
                if(pi == NULL)
                    iconHideHead[desk] = ci ;
                else
                    pi->next = ci ;
                ci->next = ni ;
                _tcscpy(ci->name,ss) ;
            }
        }
        fclose(fp) ;
    }
}

static void
saveConfigFile(void)
{
    TCHAR *ss ;
    DWORD dwNumberOfBytes;
    int ii, jj, rr, itemCount ;
    vwIcon *icon ;
#ifdef _UNICODE
    char name[1024] ;
#else
    char *name ;
#endif
    FILE *fp ;
    
    ss = userAppPath + _tcslen(userAppPath) ;
    _tcscpy(ss,_T("vwdesktopicons.cfg")) ;
    fp = _tfopen(userAppPath,_T("w")) ;
    *ss = '\0' ;
    if(fp != NULL)
    {
        itemCount = ListView_GetItemCount(diHandle) ;
        for(ii=0 ; ii<itemCount ; ii++)
        {
            diLocalItem->iItem = ii;
            WriteProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
            SendMessage(diHandle, LVM_GETITEM, 0, (LPARAM)diShareItem);
            ListView_GetItemPosition(diHandle,ii,(POINT *)(diShareItem+sizeof(LVITEM)));
            ReadProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
            for(jj=0 ; jj<=MAX_DESK ; jj++)
            {
                icon = iconHideHead[jj] ;
                while((icon != NULL) && ((rr=_tcscmp(icon->name,diItemName)) != 0))
                {
                    if(rr > 0)
                    {
                        icon = NULL ;
                        break ;
                    }
                    icon = icon->next ;
                }
                if(icon != NULL)
                {
                    /* icon set to be hidden on this desktop */
#ifdef _UNICODE
                    WideCharToMultiByte(CP_ACP,0,diItemName,-1,name,1024, 0, 0) ;
#else
                    name = diItemName ;
#endif
                    fprintf(fp,"%d H %s\n",jj,name) ;
                }
            }
        }
        fclose(fp) ;
    }
}


static void
UpdateDesktopIcons(void)
{
    DWORD dwNumberOfBytes;
    int ii, rr, itemCount ;
    vwIcon *icon ;
    
    itemCount = ListView_GetItemCount(diHandle) ;
    for(ii=0 ; ii<itemCount ; ii++)
    {
        diLocalItem->iItem = ii;
        WriteProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
        SendMessage(diHandle, LVM_GETITEM, 0, (LPARAM)diShareItem);
        ListView_GetItemPosition(diHandle,ii,(POINT *)(diShareItem+sizeof(LVITEM)));
        ReadProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
        icon = iconHideHead[deskCrrnt] ;
        while((icon != NULL) && ((rr=_tcscmp(icon->name,diItemName)) != 0))
        {
            if(rr > 0)
            {
                icon = NULL ;
                break ;
            }
            icon = icon->next ;
        }
        if(icon != NULL)
        {
            /* hide the icon if not already */
            if(diItemPos->y > -10000)
                ListView_SetItemPosition(diHandle,ii,diItemPos->x,diItemPos->y-20000) ;
        }
        else
        {
            /* show the icon if not already */
            if(diItemPos->y < -10000)
                ListView_SetItemPosition(diHandle,ii,diItemPos->x,diItemPos->y+20000) ;
        }
    }
}

static void
GenerateDesktopIconList(HWND hDlg)
{
    DWORD dwNumberOfBytes;
    int ii, rr, itemCount ;
    vwIcon *icon ;
    
    SendDlgItemMessage(hDlg,ID_HIDE_LIST,LB_RESETCONTENT,0,0);
    SendDlgItemMessage(hDlg,ID_SHOW_LIST,LB_RESETCONTENT,0,0);
    
    itemCount = ListView_GetItemCount(diHandle) ;
    for(ii=0 ; ii<itemCount ; ii++)
    {
        diLocalItem->iItem = ii;
        WriteProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
        SendMessage(diHandle, LVM_GETITEM, 0, (LPARAM)diShareItem);
        ListView_GetItemPosition(diHandle,ii,(POINT *)(diShareItem+sizeof(LVITEM)));
        ReadProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
        icon = iconHideHead[deskCrrnt] ;
        while((icon != NULL) && ((rr=_tcscmp(icon->name,diItemName)) != 0))
        {
            if(rr > 0)
            {
                icon = NULL ;
                break ;
            }
            icon = icon->next ;
        }
        if(icon != NULL)
        {
            /* icon is flagged as hidden */
            SendDlgItemMessage(hDlg,ID_HIDE_LIST,LB_ADDSTRING,0,(LONG) diItemName);
        }
        else
        {
            SendDlgItemMessage(hDlg,ID_SHOW_LIST,LB_ADDSTRING,0,(LONG) diItemName);
        }
    }
}

static void
initDialog(HWND hwndDlg, BOOL enableApply)
{
    HWND hBtn ;
    TCHAR buff[64] ;
    
    _stprintf(buff,_T("Configure icons for desktop %d:\n"),deskCrrnt) ;
    SetDlgItemText(hwndDlg, ID_LABEL,buff) ;
    GenerateDesktopIconList(hwndDlg);
    if((hBtn=GetDlgItem(hwndDlg,ID_APPLY)) != NULL)
        EnableWindow(hBtn,enableApply) ;
    if((hBtn=GetDlgItem(hwndDlg,ID_SHOW_ICON)) != NULL)
        EnableWindow(hBtn,FALSE) ;
    if((hBtn=GetDlgItem(hwndDlg,ID_HIDE_ICON)) != NULL)
        EnableWindow(hBtn,FALSE) ;
}

static void
setupShowCurrentIcon(HWND hDlg)
{
    vwIcon *ci, *pi ;
    TCHAR buff[1024] ;
    int rr, len, curSel = SendDlgItemMessage(hDlg, ID_HIDE_LIST, LB_GETCURSEL, 0, 0);
    if((curSel != LB_ERR) && ((ci = iconHideHead[deskCrrnt]) != NULL) &&
       ((len=SendDlgItemMessage(hDlg, ID_HIDE_LIST, LB_GETTEXT, curSel, (LPARAM) buff)) > 0))
    {
        rr = 0 ;
        pi = NULL ;
        while((ci != NULL) && ((rr=_tcscmp(ci->name,buff)) < 0))
        {
            pi = ci ;
            ci = ci->next ;
        }
        if(rr == 0)
        {
            if(pi == NULL)
                iconHideHead[deskCrrnt] = ci->next ;
            else
                pi->next = ci->next ;
            free(ci) ;
        }
    }
}

static void
setupHideCurrentIcon(HWND hDlg)
{
    vwIcon *ci, *ni, *pi ;
    TCHAR buff[1024] ;
    int len, curSel = SendDlgItemMessage(hDlg, ID_SHOW_LIST, LB_GETCURSEL, 0, 0);
    if((curSel != LB_ERR) && ((len=SendDlgItemMessage(hDlg, ID_SHOW_LIST, LB_GETTEXT, curSel, (LPARAM) buff)) > 0) &&
       ((ci = malloc(sizeof(vwIcon)+(len*sizeof(TCHAR)))) != NULL))
    {
        pi = NULL ;
        ni = iconHideHead[deskCrrnt] ;
        while((ni != NULL) && (_tcscmp(ni->name,buff) < 0))
        {
            pi = ni ;
            ni = ni->next ;
        }
        if(pi == NULL)
            iconHideHead[deskCrrnt] = ci ;
        else
            pi->next = ci ;
        ci->next = ni ;
        _tcscpy(ci->name,buff) ;
    }
}


/*
   This is the main function for the dialog. 
 */
static BOOL CALLBACK
DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND hBtn ;
    switch(msg)
    {
    case WM_INITDIALOG:
        initDialog(hwndDlg,FALSE);
        return TRUE;
        
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwndDlg,0);
            return 1;
        
        case ID_OK:
        case ID_APPLY:
            saveConfigFile() ;
            UpdateDesktopIcons() ;
            if(LOWORD(wParam) == ID_OK)
                EndDialog(hwndDlg,0) ;
            else
                initDialog(hwndDlg,FALSE) ;
            return 1;
            
        case ID_SHOW_ICON:
            setupShowCurrentIcon(hwndDlg) ;
            initDialog(hwndDlg,TRUE);
            return 1;
        
        case ID_HIDE_ICON:
            setupHideCurrentIcon(hwndDlg) ;
            initDialog(hwndDlg,TRUE);
            return 1;
        
        case ID_SHOW_LIST:
            if((HIWORD(wParam) == LBN_SELCHANGE) &&
               ((hBtn=GetDlgItem(hwndDlg,ID_HIDE_ICON)) != NULL))
                EnableWindow(hBtn,TRUE) ;
            break ;
            
        case ID_HIDE_LIST:
            if((HIWORD(wParam) == LBN_SELCHANGE) &&
               ((hBtn=GetDlgItem(hwndDlg,ID_SHOW_ICON)) != NULL))
                EnableWindow(hBtn,TRUE) ;
            break ;
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
        MessageBox(hwnd,_T("VirtuaWin failed to send the UserApp path."),_T("VWDesktopIcons Error"), MB_ICONWARNING);
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
        if((deskCrrnt = SendMessage(vwHandle,VW_CURDESK,0,0)) > MAX_DESK)
            deskCrrnt = 0 ;
        if(!initialised)
        {
            /* Get the VW Install path and then the user's path - give VirtuaWin 10 seconds to do this */
            SendMessage(vwHandle, VW_USERAPPPATH, 0, 0);
            SetTimer(hwnd, 0x29a, 10000, startupFailureTimerProc);
        }
        else
            UpdateDesktopIcons() ;
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
                    MessageBox(hwnd,_T("VirtuaWin returned a bad UserApp path."),_T("VWDesktopIcons Error"), MB_ICONWARNING);
                    exit(1) ;
                }
#ifdef _UNICODE
                MultiByteToWideChar(CP_ACP,0,(char *) cds->lpData,-1,userAppPath,MAX_PATH) ;
#else
                strcpy(userAppPath,(char *) cds->lpData) ;
#endif
                initialised = 1 ;
                
                loadConfigFile() ;
            }
        }
        return TRUE ;
        
    case MOD_CHANGEDESK:
        if(lParam > MAX_DESK)
            deskCrrnt = 0 ;
        else
            deskCrrnt = lParam ;
        UpdateDesktopIcons() ;
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
    DWORD dwPID;
    WNDCLASS wc;
    MSG msg;
    
    hInst = hInstance;
    memset(&wc, 0, sizeof(WNDCLASS));
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)MainWndProc;
    wc.hInstance = hInstance ;
    /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
       this for locating the window */
    wc.lpszClassName = _T("VWDesktopIcons.exe");
  
    if(!RegisterClass(&wc))
        return 0;
  
    /* In this example, the window is never shown */
    if((wHnd=CreateWindow(_T("VWDesktopIcons.exe"), 
                          _T("VWDesktopIcons"), 
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
    
    /* get the SystemList handle holding the deskop icon and allocate required resources */ 
    if((diHandle = FindWindow(_T("Progman"),NULL)) == NULL)
    {
        MessageBox(wHnd,_T("Failed to obtain handle to Progman\n"),_T("VWDesktopIcons Error"), MB_ICONERROR) ;
        exit(1) ;
    }
    if((diHandle = FindWindowEx(diHandle,NULL,_T("SHELLDLL_DefView"),NULL)) == NULL)
    {
        MessageBox(wHnd,_T("Failed to obtain handle to SHELLDLL_DefView\n"),_T("VWDesktopIcons Error"), MB_ICONERROR) ;
        exit(1) ;
    }
    if((diHandle = FindWindowEx(diHandle,NULL,_T("SysListView32"),NULL)) == NULL)
    {
        MessageBox(wHnd,_T("Failed to obtain handle to SysListView32\n"),_T("VWDesktopIcons Error"), MB_ICONERROR) ;
        exit(1) ;
    }
    
    GetWindowThreadProcessId(diHandle, &dwPID);
    diProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,FALSE,dwPID);
    diLocalItem = (LVITEM *) VirtualAlloc(NULL,diItemSize,MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    diShareItem = (char *) VirtualAllocEx(diProcess, NULL, diItemSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if((diLocalItem == NULL) || (diShareItem == NULL))
    {
        MessageBox(wHnd,_T("Failed to malloc required memory\n"),_T("VWDesktopIcons Error"), MB_ICONERROR) ;
        exit(1) ;
    }
    diLocalItem->mask = LVIF_TEXT|LVIF_PARAM ; 
    diLocalItem->iSubItem = 0; 
    diLocalItem->pszText = (TCHAR *) (diShareItem + sizeof(LVITEM) + sizeof(POINT)) ; 
    diLocalItem->cchTextMax = MAX_PATH; 
    diItemPos = (POINT *) (((char *) diLocalItem) + sizeof(LVITEM)) ;
    diItemName = (TCHAR *) (((char *) diLocalItem) + sizeof(LVITEM) + sizeof(POINT)) ; 
    
    /* main messge loop */
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    VirtualFree(diLocalItem, 0, MEM_RELEASE);
    VirtualFreeEx(diProcess, diShareItem, 0, MEM_RELEASE);
    CloseHandle(diProcess);
    return msg.wParam;
}
