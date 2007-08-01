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
#include "../../Defines.h"
#include "../../Messages.h"
#include "Resource.h"

#define vwICON_HIDDEN 0x01
#define vwICON_MOVED  0x02

typedef struct vwIcon {
    struct vwIcon *next ;
    unsigned char  flags[vwDESKTOP_SIZE] ;
    POINT          point[vwDESKTOP_SIZE] ;
    TCHAR          name[1] ;
} vwIcon ;

#define diItemSize (sizeof(LVITEM) + sizeof(POINT) + (sizeof(TCHAR) * MAX_PATH))

unsigned char initialised=0 ;
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

int     deskCrrnt ;
int     deskCopy ;
vwIcon *iconHead ;

static void
CheckDesktopSettings(void)
{
    HKEY hkey=NULL ;
    if((RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Microsoft\\Windows\\Shell\\Bags\\1\\Desktop"), 0,KEY_READ, &hkey) == ERROR_SUCCESS) && (hkey != NULL))
    {
        DWORD fflags, fflagsType, fflagsLen ;
        fflagsLen = 4 ;
        if((RegQueryValueEx(hkey,_T("FFlags"),NULL,&fflagsType,(LPBYTE) &fflags,&fflagsLen) == ERROR_SUCCESS) && 
           (fflagsType == REG_DWORD) && (fflagsLen == 4) && (fflags & 0x05))
        {
            MessageBox(wHnd,_T("Your current Windows desktop settings appear to be incompatible\n")
                       _T("with VWDesktopIcons, you need to disable 'Auto Arrange' and\n")
                       _T("'Align to Grid' settings in the 'Arrange Icons By' desktop context\n")
                       _T("menu to use this module.\n\nPlease ignore this warning if you have already made this change."),_T("VWDesktopIcons Warning"), MB_ICONWARNING) ;
        }            
    }
}

static void
LoadConfigFile(void)
{
    vwIcon *ci ;
    TCHAR buff[1024], *ss, *fs, cc ;
    int desk, len, rr ;
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
               ((desk = (unsigned short) _ttoi(buff)) > 0) && (desk < vwDESKTOP_SIZE) &&
               ((fs=_tcschr(buff,' ')) != NULL) &&
               ((ss=_tcschr(fs+1,' ')) != NULL) && (ss != fs) &&
               ((len = _tcslen(++ss)) > 1))
            {
                if(ss[len-1] == '\n')
                    ss[--len] = '\0' ;
                rr = 1 ;
                ci = iconHead ;
                while((ci != NULL) && ((rr=_tcscmp(ci->name,ss)) < 0))
                    ci = ci->next ;
                if(rr == 0)
                {
                    ss[-1] = '\0' ;
                    while((cc = *++fs) != '\0')
                    {
                        if(cc == 'H')
                            ci->flags[desk] |= vwICON_HIDDEN ;
                        else if(cc == 'P')
                        {
                            ci->point[desk].x = _ttoi(++fs) ;
                            if((fs=_tcschr(fs,':')) != NULL)
                            {
                                ci->point[desk].y = _ttoi(++fs) ;
                                fs = _tcschr(fs,':') ;
                            }
                            if(fs != NULL)
                                ci->flags[desk] |= vwICON_MOVED ;
                            else
                                fs = ss - 1 ;
                        }
                    }
                }
            }
        }
        fclose(fp) ;
    }
}

static void
SaveConfigFile(void)
{
    TCHAR *ss ;
    int ii ;
    vwIcon *ci ;
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
        ci = iconHead ;
        while(ci != NULL)
        {
#ifdef _UNICODE
            name[0] = '\0' ;
#else
            name = ci->name ;
#endif
            for(ii=1 ; ii< vwDESKTOP_SIZE ; ii++)
            {
                if(ci->flags[ii])
                {
#ifdef _UNICODE
                    if(name[0] == '\0')
                        WideCharToMultiByte(CP_ACP,0,ci->name,-1,name,1024, 0, 0) ;
#endif
                    if(ci->flags[ii] & vwICON_MOVED)
                        fprintf(fp,"%d %sP%d:%d: %s\n",ii,(ci->flags[ii] & vwICON_HIDDEN) ? "H":"",
                                (int) ci->point[ii].x,(int) ci->point[ii].y,name) ;
                    else
                        fprintf(fp,"%d H %s\n",ii,name) ;
                }
            }
            ci = ci->next ;
        }
        fclose(fp) ;
    }
}


static int
UpdateDesktopIcons(int fdesk, int tdesk)
{
    DWORD dwNumberOfBytes;
    int ii, rr, itemCount ;
    vwIcon *ci, *pi, *ti ;
    
    itemCount = ListView_GetItemCount(diHandle) ;
    for(ii=0 ; ii<itemCount ; ii++)
    {
        diLocalItem->iItem = ii;
        WriteProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
        SendMessage(diHandle, LVM_GETITEM, 0, (LPARAM)diShareItem);
        ListView_GetItemPosition(diHandle,ii,(POINT *)(diShareItem+sizeof(LVITEM)));
        ReadProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
        rr = 1 ;
        pi = NULL ;
        ci = iconHead ;
        while((ci != NULL) && ((rr=_tcscmp(ci->name,diItemName)) < 0))
        {
            pi = ci ;
            ci = ci->next ;
        }
        if(rr != 0)
        {
            if((ti = calloc(1,sizeof(vwIcon)+(_tcslen(diItemName)*sizeof(TCHAR)))) == NULL)
                return 1 ;
            _tcscpy(ti->name,diItemName) ;
            ti->point[0].x = diItemPos->x ;
            ti->point[0].y = diItemPos->y ;
            if(pi == NULL)
                iconHead = ti ;
            else
                pi->next = ti ;
            ti->next = ci ;
            ci = ti ;
        }
        else if(fdesk && ((ci->flags[fdesk] & vwICON_HIDDEN) == 0) &&
                ((ci->point[fdesk].x != diItemPos->x) ||
                 (ci->point[fdesk].y != diItemPos->y) ))
        {
            ci->point[fdesk].x = diItemPos->x ;
            ci->point[fdesk].y = diItemPos->y ;
            if((ci->point[0].x != diItemPos->x) ||
               (ci->point[0].y != diItemPos->y) )
                ci->flags[fdesk] |= vwICON_MOVED ;
            else
                ci->flags[fdesk] &= ~vwICON_MOVED ;
        }
        if(tdesk != fdesk)
        {
            if(ci->flags[tdesk] & vwICON_HIDDEN)
            {
                if((ci->point[0].x != diItemPos->x) || ((ci->point[0].y-20000) != diItemPos->y))
                    ListView_SetItemPosition(diHandle,ii,ci->point[0].x,ci->point[0].y-20000) ;
            }
            else if(ci->flags[tdesk] & vwICON_MOVED)
            {
                if((ci->point[tdesk].x != diItemPos->x) || (ci->point[tdesk].y != diItemPos->y))
                    ListView_SetItemPosition(diHandle,ii,ci->point[tdesk].x,ci->point[tdesk].y) ;
            }
            else if((ci->point[0].x != diItemPos->x) || (ci->point[0].y != diItemPos->y))
                ListView_SetItemPosition(diHandle,ii,ci->point[0].x,ci->point[0].y) ;
        }
    }
    return 0 ;
}

static int
SetDesktopIcons(int tdesk)
{
    DWORD dwNumberOfBytes;
    int ii, itemCount ;
    vwIcon *ci ;
    
    itemCount = ListView_GetItemCount(diHandle) ;
    for(ii=0 ; ii<itemCount ; ii++)
    {
        diLocalItem->iItem = ii;
        WriteProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
        SendMessage(diHandle, LVM_GETITEM, 0, (LPARAM)diShareItem);
        ListView_GetItemPosition(diHandle,ii,(POINT *)(diShareItem+sizeof(LVITEM)));
        ReadProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
        ci = iconHead ;
        while(ci != NULL)
        {
            if(!_tcscmp(ci->name,diItemName))
            {
                if(ci->flags[tdesk] & vwICON_HIDDEN)
                    ListView_SetItemPosition(diHandle,ii,ci->point[0].x,ci->point[0].y-20000) ;
                else if(ci->flags[tdesk] & vwICON_MOVED)
                    ListView_SetItemPosition(diHandle,ii,ci->point[tdesk].x,ci->point[tdesk].y) ;
                else
                    ListView_SetItemPosition(diHandle,ii,ci->point[0].x,ci->point[0].y) ;
                break ;
            }
            ci = ci->next ;
        }
    }
    return 0 ;
}

static void
GenerateDesktopIconList(HWND hDlg)
{
    vwIcon *ci ;
    
    SendDlgItemMessage(hDlg,ID_HIDE_LIST,LB_RESETCONTENT,0,0);
    SendDlgItemMessage(hDlg,ID_SHOW_LIST,LB_RESETCONTENT,0,0);
    
    ci = iconHead ;
    while(ci != NULL)
    {
        if(ci->flags[deskCrrnt] & vwICON_HIDDEN)
        {
            /* icon is flagged as hidden */
            SendDlgItemMessage(hDlg,ID_HIDE_LIST,LB_ADDSTRING,0,(LONG) ci->name);
        }
        else
        {
            SendDlgItemMessage(hDlg,ID_SHOW_LIST,LB_ADDSTRING,0,(LONG) ci->name);
        }
        ci = ci->next ;
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
    if((hBtn=GetDlgItem(hwndDlg,ID_PASTE)) != NULL)
        EnableWindow(hBtn,(deskCopy && (deskCopy != deskCrrnt)) ? TRUE:FALSE) ;
}

static void
SetupShowCurrentIcon(HWND hDlg)
{
    vwIcon *ci ;
    TCHAR buff[1024] ;
    int curSel ;
    
    if(((curSel= SendDlgItemMessage(hDlg,ID_HIDE_LIST,LB_GETCURSEL,0,0)) != LB_ERR) &&
       (SendDlgItemMessage(hDlg,ID_HIDE_LIST,LB_GETTEXT,curSel,(LPARAM) buff) > 0))
    {
        ci = iconHead ;
        while(ci != NULL)
        {
            if(!_tcscmp(ci->name,buff))
            {
                ci->flags[deskCrrnt] &= ~vwICON_HIDDEN ;
                break ;
            }
            ci = ci->next ;
        }
    }
}

static void
SetupHideCurrentIcon(HWND hDlg)
{
    vwIcon *ci ;
    TCHAR buff[1024] ;
    int curSel ;
    
    if(((curSel= SendDlgItemMessage(hDlg,ID_SHOW_LIST,LB_GETCURSEL,0,0)) != LB_ERR) &&
       (SendDlgItemMessage(hDlg,ID_SHOW_LIST,LB_GETTEXT,curSel,(LPARAM) buff) > 0))
    {
        ci = iconHead ;
        while(ci != NULL)
        {
            if(!_tcscmp(ci->name,buff))
            {
                ci->flags[deskCrrnt] |= vwICON_HIDDEN ;
                break ;
            }
            ci = ci->next ;
        }
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
            UpdateDesktopIcons(deskCrrnt,deskCrrnt) ;
            EndDialog(hwndDlg,0);
            return 1;
        
        case ID_OK:
        case ID_APPLY:
            SetDesktopIcons(deskCrrnt) ;
            SaveConfigFile() ;
            if(LOWORD(wParam) == ID_OK)
                EndDialog(hwndDlg,0) ;
            else
                initDialog(hwndDlg,FALSE) ;
            return 1;
            
        case ID_RESTORE:
            if(deskCrrnt)
            {
                vwIcon *ci = iconHead ;
                while(ci != NULL)
                {
                    if(ci->point[0].y < -5000)
                        ci->point[0].y += -20000 ;
                    ci->flags[deskCrrnt] = 0 ;
                    ci->point[deskCrrnt] = ci->point[0] ;
                    ci = ci->next ;
                }
                initDialog(hwndDlg,TRUE);
            }
            return 1;
            
        case ID_COPY:
            deskCopy = deskCrrnt ;
            if((hBtn=GetDlgItem(hwndDlg,ID_PASTE)) != NULL)
                EnableWindow(hBtn,FALSE) ;
            return 1;
            
        case ID_PASTE:
            if(deskCopy && deskCrrnt)
            {
                vwIcon *ci = iconHead ;
                while(ci != NULL)
                {
                    ci->flags[deskCrrnt] = ci->flags[deskCopy] ;
                    ci->point[deskCrrnt] = ci->point[deskCopy] ;
                    ci = ci->next ;
                }
                initDialog(hwndDlg,TRUE);
                if((hBtn=GetDlgItem(hwndDlg,ID_PASTE)) != NULL)
                    EnableWindow(hBtn,FALSE) ;
            }
            return 1;
        
        case ID_SHOW_ICON:
            SetupShowCurrentIcon(hwndDlg) ;
            initDialog(hwndDlg,TRUE);
            return 1;
        
        case ID_HIDE_ICON:
            SetupHideCurrentIcon(hwndDlg) ;
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

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int newDesk ;
    switch (msg)
    {
    case MOD_INIT: 
        /* This must be taken care of in order to get the handle to VirtuaWin. */
        /* The handle to VirtuaWin comes in the wParam */
        vwHandle = (HWND) wParam;
        if((deskCrrnt = SendMessage(vwHandle,VW_CURDESK,0,0)) >= vwDESKTOP_SIZE)
            deskCrrnt = 0 ;
        // If not yet initialized get the user path and initialize.
        if(!initialised)
        {
            if(!SendMessage(vwHandle, VW_USERAPPPATH, (WPARAM) hwnd, 0) || !initialised)
            {
                MessageBox(hwnd,_T("VirtuaWin failed to send the UserApp path."),_T("VWDesktopIcons Error"), MB_ICONWARNING);
                exit(1) ;
            }
            SetDesktopIcons(deskCrrnt) ;
            CheckDesktopSettings() ;
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
                    return FALSE ;
                initialised = 1 ;
#ifdef _UNICODE
                MultiByteToWideChar(CP_ACP,0,(char *) cds->lpData,-1,userAppPath,MAX_PATH) ;
#else
                strcpy(userAppPath,(char *) cds->lpData) ;
#endif
                LoadConfigFile() ;
            }
        }
        return TRUE ;
        
    case MOD_CHANGEDESK:
        if(lParam >= vwDESKTOP_SIZE)
            newDesk = 0 ;
        else
            newDesk = lParam ;
        UpdateDesktopIcons(deskCrrnt,newDesk) ;
        deskCrrnt = newDesk ;
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
    
    /* get icon list */ 
    if(UpdateDesktopIcons(0,0))
    {
        MessageBox(wHnd,_T("Failed to get the initial list of desktop icons\n"),_T("VWDesktopIcons Error"), MB_ICONERROR) ;
        exit(1) ;
    }

    /* main messge loop */
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    UpdateDesktopIcons(deskCrrnt,deskCrrnt) ;
    SaveConfigFile() ;
    SetDesktopIcons(0) ;
    VirtualFree(diLocalItem, 0, MEM_RELEASE);
    VirtualFreeEx(diProcess, diShareItem, 0, MEM_RELEASE);
    CloseHandle(diProcess);
    return msg.wParam;
}
