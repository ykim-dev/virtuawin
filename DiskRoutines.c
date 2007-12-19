//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  DiskRoutines.c - File reading an writing routines.
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

#include "VirtuaWin.h"
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <errno.h>
#include <assert.h>
#include <shlobj.h>  // for SHGetFolderPath
#include <direct.h>  // for mkdir

#include "DiskRoutines.h"
#include "ConfigParameters.h"

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES	((DWORD)-1)
#endif
#define VIRTUAWIN_SUBDIR vwVIRTUAWIN_NAME

TCHAR *VirtuaWinPath=NULL ;
TCHAR *UserAppPath=NULL ;
#ifdef _UNICODE
char *VirtuaWinPathStr=NULL ;
char *UserAppPathStr=NULL ;
#endif

/************************************************** 
 * Gets the local application settings path for the current user.
 * Calling function MUST pre-allocate the return string!
 */
static void getUserAppPath(TCHAR *path)
{
    LPITEMIDLIST idList ;
    TCHAR buff[MAX_PATH], *ss, *se, cc ;
    FILE *fp ;
    int len ;
    
    path[0] = '\0' ;
    
    /* look for a userpath.cfg file */
    _tcscpy(buff,VirtuaWinPath) ;
    _tcscat(buff,_T("userpath.cfg")) ;
    if((fp = _tfopen(buff,_T("r"))) != NULL)
    {
        if((_fgetts(buff,MAX_PATH,fp) != NULL) && (buff[0] != '\0'))
        {
            len = 0 ;
            ss = buff ;
            while(((cc=*ss++) != '\0') && (cc !='\n'))
            {
                if((cc == '$') && (*ss == '{') && ((se=_tcschr(ss,'}')) != NULL))
                {
                    *se++ = '\0' ;
                    if(!_tcscmp(ss+1,_T("VIRTUAWIN_PATH")))
                        ss = VirtuaWinPath ;
                    else
                        ss = _tgetenv(ss+1) ;
                    if(ss != NULL)
                    {
                        _tcscpy(path+len,ss) ;
                        len += _tcslen(ss) ;
                    }
                    ss = se ;
                }
                else
                {
                    if(cc == '/')
                        cc = '\\' ;
                    if((cc != '\\') || (len <= 1) || (path[len-1] != '\\'))
                        path[len++] = cc ;
                }
            }
            if(len && (path[len-1] != '\\'))
                path[len++] = '\\' ;
            path[len] = '\0' ;
        }
        fclose(fp) ;
    }
    if(path[0] == '\0')
    {
        if(SUCCEEDED(SHGetSpecialFolderLocation(NULL,CSIDL_APPDATA,&idList)) && (idList != NULL))
        {
            IMalloc *im ;
            SHGetPathFromIDList(idList,path);
            if(SUCCEEDED(SHGetMalloc(&im)) && (im != NULL))
            {
                im->lpVtbl->Free(im,idList) ;
                im->lpVtbl->Release(im);
            }
        }
        if(path[0] != '\0')
        {
            len = _tcslen(path) ;
            if(path[len - 1] != '\\')
                path[len++] = '\\' ;
            _tcsncpy(path+len,VIRTUAWIN_SUBDIR,MAX_PATH - len) ;
            len += _tcslen(path+len) ;
            path[len++] = '\\' ;
            path[len] = '\0' ;
        }
    }
}

/************************************************
 * Generates a path + username for the requested file type, fills in
 * the supplied string parameter, that MUST be pre-allocated.  Caller
 * is responsible for memory allocation.  It is suggested that the
 * caller allocate the string on the stack, not dynamically on the
 * heap so that it is cleaned up automatically.  Strings can be up to
 * MAX_PATH in length.
 *
 * Return is 1 if successful, 0 otherwise.  (suggestion, convert to
 * type bool after porting to cpp)
 */

void
GetFilename(eFileNames filetype, int location, TCHAR *outStr)
{
    static TCHAR *subPath[vwFILE_COUNT] = {
        _T("modules\\*.exe"), _T("virtuawin"), _T("virtuawin.cfg"), _T("userlist.cfg"), _T("tricky.cfg"),
        _T("sticky.cfg"), _T("module.cfg"), _T("assignment.cfg")
    };
    DWORD len ;
    
    if(UserAppPath == NULL)
    {
        /* initialization of paths, find the installation and user paths,
         * exit on failure - initialization happens so early on it is safe to
         * simply exit */
        TCHAR path[MAX_PATH], *ss ;
        
        GetModuleFileName(GetModuleHandle(NULL),path,MAX_PATH) ;
        ss = _tcsrchr(path,'\\') ;
        ss[1] = '\0' ;
        if((VirtuaWinPath = _tcsdup(path)) != NULL)
        {
            getUserAppPath(path) ;
            if(path[0] == '\0')
                UserAppPath = VirtuaWinPath ;
            else
                UserAppPath = _tcsdup(path) ;
        }
        if((VirtuaWinPath == NULL) || (UserAppPath == NULL))
        {
            MessageBox(hWnd,_T("Memory resources appear to be very low, try rebooting.\nIf you still have problems, send a mail to \n") vwVIRTUAWIN_EMAIL,vwVIRTUAWIN_NAME _T(" Error"), MB_ICONERROR);
            exit(1);
        }
#ifdef _UNICODE
        if(WideCharToMultiByte(CP_ACP,0,VirtuaWinPath,-1,(char *) path,MAX_PATH, 0, 0))
            VirtuaWinPathStr = strdup((char *) path) ;
        if(UserAppPath == VirtuaWinPath)
            UserAppPathStr = VirtuaWinPathStr ;
        else if(WideCharToMultiByte(CP_ACP,0,UserAppPath,-1,(char *) path,MAX_PATH, 0, 0))
            UserAppPathStr = strdup((char *) path) ;
#endif
    }
    
    _tcsncpy(outStr,(location) ? UserAppPath:VirtuaWinPath, MAX_PATH);
    if(filetype < vwFILE_COUNT)
    {
        len = MAX_PATH - _tcslen(outStr) ;
        _tcsncat(outStr,subPath[filetype],len) ;
    }
}


/*************************************************
 * Write out the disabled modules
 */
void
saveDisabledList(int theNOfModules, moduleType* theModList)
{
    TCHAR DisabledFileList[MAX_PATH];
    FILE* fp;
    
    GetFilename(vwDISABLED,1,DisabledFileList);
    if(!(fp = _tfopen(DisabledFileList,_T("w"))))
        MessageBox(hWnd,_T("Error saving disabled module state"),vwVIRTUAWIN_NAME _T(" Error"),MB_ICONERROR);
    else
    {
        int i;
        for(i = 0; i < theNOfModules; ++i)
            if(theModList[i].disabled)
                _ftprintf(fp,_T("%s\n"),theModList[i].description);
        fclose(fp);
    }
}

/*************************************************
 * Loads module names that should be disabled 
 */
int loadDisabledModules(disModules *theDisList) 
{
    TCHAR buff[MAX_PATH];
    int len, nOfDisMod = 0;
    FILE *fp;
    
    GetFilename(vwDISABLED,1,buff);
    
    if((fp = _tfopen(buff,_T("r"))) != NULL)
    {
        while(_fgetts(buff,MAX_PATH,fp) != NULL)
        {
            if((len = _tcslen(buff)) > 1)
            {
                if(len > vwMODULENAME_MAX)
                    buff[vwMODULENAME_MAX] = '\0' ;
                else if(buff[len-1] == '\n')
                    buff[len-1] = '\0' ;
                _tcscpy(theDisList[nOfDisMod++].moduleName,buff);
                if(nOfDisMod == (MAXMODULES * 2))
                    break ;
            }
        }
        fclose(fp);
    }
    return nOfDisMod;
}

/*************************************************
 * Loads window match list from a file
 */
static void
loadWindowMatchList(TCHAR *fname, int hasDesk, vwWindowType **matchList)
{
    vwWindowType *wm, *pwm ;
    vwUByte type, desk=0 ;
    TCHAR buff[1024], *ss ;
    int len ;
    FILE *fp ;
    
    if((wm = *matchList) != NULL)
    {
        do {
            pwm = wm->next ;
            free(wm->match) ;
            free(wm) ;
        } while((wm = pwm) != NULL) ; 
        *matchList = NULL ;
    }
    pwm = NULL ;
    if((fp = _tfopen(fname,_T("r"))) != NULL)
    {
        while(_fgetts(buff,1024,fp) != NULL)
        {
            ss = NULL ;
            if(buff[0] != ':')
            {
                if(!hasDesk)
                    ss = buff ;
                else if(((desk = (vwUByte) _ttoi(buff)) > 0) &&
                        ((ss=_tcschr(buff,' ')) != NULL))
                    ss++ ;
            }            
            if((ss != NULL) && ((len = _tcslen(ss)) > 1))
            {
                if(ss[len-1] == '\n')
                    ss[--len] = '\0' ;
                if((ss[2] == ':') &&
                   ((ss[0] == 'c') || (ss[0] == 'w')) &&
                   ((ss[1] == 'n') || (ss[1] == 'r')))
                {
                    len -= 3 ;
                    type = (ss[0] == 'w') | ((ss[1] == 'r') ? 2:0) ;
                    ss += 3 ;
                }
                else
                    type = 0 ;
                if(((type & 2) == 0) && (len > vwCLASSNAME_MAX))
                    ss[vwCLASSNAME_MAX] = '\0' ;
                if(((wm = malloc(sizeof(vwWindowType))) != NULL) &&
                   ((wm->match = _tcsdup(ss)) != NULL))
                {
                    wm->next = NULL ;
                    wm->type = type ;
                    wm->desk = desk ;
                    if(pwm == NULL)
                        *matchList = wm ;
                    else
                        pwm->next = wm ;
                    pwm = wm ;
                }
            }
        }
        fclose(fp) ;
    }
}

/*************************************************
 * Loads window classnames from sticky file
 */
void
loadStickyList(void) 
{
    TCHAR fname[MAX_PATH];
    
    GetFilename(vwSTICKY,1,fname);
    loadWindowMatchList(fname,0,&stickyList) ;
}

/*************************************************
 * Loads window classnames from tricky file
 */
void
loadTrickyList(void) 
{
    TCHAR fname[MAX_PATH];
    
    GetFilename(vwTRICKY,1,fname);
    loadWindowMatchList(fname,0,&trickyList) ;
}

/*************************************************
 * Loads the list with classnames that has an desktop assigned
 */
void
loadAssignedList(void) 
{
    TCHAR fname[MAX_PATH];
    
    GetFilename(vwWINDOWS_STATE,1,fname);
    loadWindowMatchList(fname,1,&assignedList) ;
}

/*************************************************
 * Loads window titles/classnames from user file 
 */
void
loadUserList(void) 
{
    TCHAR fname[MAX_PATH];
    
    GetFilename(vwLIST,1,fname);
    loadWindowMatchList(fname,0,&userList) ;
}

/************************************************
 * Writes down the current configuration on file
 */
void writeConfig(void)
{
    TCHAR VWConfigFile[MAX_PATH];
    FILE* fp;
    int ii, jj ;
    
    GetFilename(vwCONFIG,1,VWConfigFile);
    if((fp = _tfopen(VWConfigFile,_T("w"))) == NULL)
    {
        MessageBox(NULL,_T("Error writing config file"),vwVIRTUAWIN_NAME _T(" Error"),MB_ICONERROR);
    }
    else
    {
        fprintf(fp, "ver# 2\n") ;
        fprintf(fp, "hotkeyCount# %d\n", hotkeyCount);
        for(ii=0 ; ii<hotkeyCount ; ii++) 
            fprintf(fp, "hotkey%d# %d %d %d %d\n",ii+1,hotkeyList[ii].key,hotkeyList[ii].modifier,hotkeyList[ii].command,hotkeyList[ii].desk) ;
        jj = vwDESKTOP_MAX ;
        while(jj && (desktopName[jj] == NULL))
            jj-- ;
        fprintf(fp, "desktopNameCount# %d\n",jj);
        for(ii=1 ; ii<=jj ; ii++) 
            _ftprintf(fp, _T("desktopName%d# %s\n"),ii,(desktopName[ii] == NULL) ? _T(""):desktopName[ii]);
        fprintf(fp, "deskX# %d\n", nDesksX);
        fprintf(fp, "deskY# %d\n", nDesksY);
        fprintf(fp, "deskWrap# %d\n", deskWrap);
        fprintf(fp, "useDeskAssignment# %d\n", useDeskAssignment);
        fprintf(fp, "assignImmediately# %d\n", assignImmediately);
        fprintf(fp, "winListContent# %d\n", winListContent);
        fprintf(fp, "winListCompact# %d\n", winListCompact);
        fprintf(fp, "mouseEnable# %d\n", mouseEnable);
        fprintf(fp, "mouseJumpLength# %d\n", mouseJumpLength);
        fprintf(fp, "mouseDelay# %d\n", mouseDelay);
        fprintf(fp, "mouseWarp# %d\n", mouseWarp);
        fprintf(fp, "mouseKnock# %d\n", mouseKnock);
        fprintf(fp, "mouseModifierUsed# %d\n", mouseModifierUsed);
        fprintf(fp, "mouseModifier# %d\n", mouseModifier);
        fprintf(fp, "preserveZOrder# %d\n", preserveZOrder);
        fprintf(fp, "hiddenWindowAct# %d\n", hiddenWindowAct);
        fprintf(fp, "minSwitch# %d\n", minSwitch);
        fprintf(fp, "releaseFocus# %d\n", releaseFocus);
        fprintf(fp, "refreshOnWarp# %d\n", refreshOnWarp);
        fprintf(fp, "invertY# %d\n", invertY);
        fprintf(fp, "noTaskbarCheck# %d\n", noTaskbarCheck);
        fprintf(fp, "trickyWindows# %d\n", trickyWindows);
        fprintf(fp, "displayTaskbarIcon# %d\n", displayTaskbarIcon);
        fprintf(fp, "logFlag# %d\n", vwLogFlag);
        fclose(fp);
    }
}

/*************************************************
 * Reads a saved configuration from file
 */
/* Get the list of hotkey commands */
#define VW_COMMAND(a, b, c, d) a = b ,
enum {
#include "vwCommands.def"
} ;
#undef  VW_COMMAND

static void
addOldHotkey(int key, int mod, int win, int cmd, int desk)
{
    if(key != 0)
    {
        hotkeyList[hotkeyCount].key = (vwUByte) key ;
        hotkeyList[hotkeyCount].modifier = 0 ;
        if(mod & HOTKEYF_ALT)
            hotkeyList[hotkeyCount].modifier |= vwHOTKEY_ALT ;
        if(mod & HOTKEYF_CONTROL)
            hotkeyList[hotkeyCount].modifier |= vwHOTKEY_CONTROL;
        if(mod & HOTKEYF_SHIFT)
            hotkeyList[hotkeyCount].modifier |= vwHOTKEY_SHIFT;
        if(mod & HOTKEYF_EXT)
            hotkeyList[hotkeyCount].modifier |= vwHOTKEY_EXT;
        if(win)
            hotkeyList[hotkeyCount].modifier |= vwHOTKEY_WIN;
        hotkeyList[hotkeyCount].command = (vwUByte) cmd ;
        hotkeyList[hotkeyCount].desk = (vwUByte) desk ;
        hotkeyCount++ ;
    }
}

void readConfig(void)
{
    TCHAR buff[MAX_PATH], buff2[2048], *ss ;
    FILE *fp, *wfp;
    int ii, jj, ll, ia[24], hk[4] ;
    
    GetFilename(vwCONFIG,1,buff);
    if(GetFileAttributes(buff) == INVALID_FILE_ATTRIBUTES)
    {
        /* config file does not exist - new user, setup configuration, check
         * the user path exists first and if not try to create it - note that
         * multiple levels may need to be created due to the userpath.cfg */
        if((buff[0] == '\\') && (buff[1] == '\\') && ((ss = _tcschr(buff+2,'\\')) != NULL) && (ss[1] != '\0'))
            ;
        else if(buff[1] == ':')
            ss = buff + 2 ;
        else
            ss = buff ;
        while((ss = _tcschr(ss+1,'\\')) != NULL)
        {
            *ss = '\0' ;
            if(((GetFileAttributes(buff) & (0xf0000000|FILE_ATTRIBUTE_DIRECTORY)) != FILE_ATTRIBUTE_DIRECTORY) &&
               (CreateDirectory(buff,NULL) == 0))
            {
                _stprintf(buff2,vwVIRTUAWIN_NAME _T(" cannot create the user config directory:\n\n    %s\n\nPlease check file permissions. If you continue to have problems, send e-mail to:\n\n    ") vwVIRTUAWIN_EMAIL,buff);
                MessageBox(hWnd,buff2,vwVIRTUAWIN_NAME _T(" Error"),MB_ICONERROR);
                exit(1) ;
            }
            *ss = '\\' ;
        }
        
        /* If the user path is not the installation path then copy all the
         * config files across to the user area */
        if(_tcsicmp(VirtuaWinPath,UserAppPath))
        {
            ii = vwFILE_COUNT ;
            while(--ii >= vwCONFIG)
            {
                GetFilename(ii,0,buff);
                if((fp = _tfopen(buff,_T("rb"))) != NULL)
                {
                    GetFilename(ii,1,buff2);
                    if((wfp = _tfopen(buff2,_T("wb"))) == NULL)
                        break ;
                    for(;;)
                    {
                        if((jj=fread(buff2,1,2048,fp)) <= 0)
                            break ;
                        if(fwrite(buff2,1,jj,wfp) != (size_t) jj)
                        {
                            jj = -1 ;
                            break ;
                        }
                    }
                    fclose(fp) ;
                    if((fclose(wfp) != 0) || (jj < 0))
                        break ;
                }
            }
        }
        else
        {
            /* must create the main VirtuaWin.cfg file */
            ii = vwCONFIG - 1 ;
            fp = NULL ;
        }
        GetFilename(vwCONFIG,1,buff);
        /* check a main config file has been copied, if not create a dummy one */
        if((ii < vwCONFIG) && (fp == NULL) &&
           (((wfp = _tfopen(buff,_T("wb"))) == NULL) || (fclose(wfp) != 0)))
            ii = vwCONFIG ;
            
        /* check we did not break out due to an error */
        if(ii >= vwCONFIG)
        {
            MessageBox(hWnd,_T("Error occurred creating new user configuration, please check file permissions.\nIf you continue to have problems, send e-mail to:\n\n    ") vwVIRTUAWIN_EMAIL,vwVIRTUAWIN_NAME _T(" Error"),MB_ICONERROR);
            exit(1) ;
        }
            
        _stprintf(buff2,_T("Welcome to %s\n\nA new user configuration has been created in directory:\n\n    %s\n\nRight click on tray icon to access the Setup dialog."),vwVIRTUAWIN_NAME_VERSION,UserAppPath) ;
        MessageBox(hWnd,buff2,vwVIRTUAWIN_NAME,MB_ICONINFORMATION);
    }
    if(((fp = _tfopen(buff,_T("r"))) == NULL) ||
       (fscanf(fp, "%s%d", (char *) buff, &ii) != 2))
    {
        _stprintf(buff2,_T("Error reading config file:\n\n    %s\n\nPlease check file permissions. If you continue to have problems, send e-mail to:\n\n    ") vwVIRTUAWIN_EMAIL,buff);
        MessageBox(hWnd,buff2,vwVIRTUAWIN_NAME _T(" Error"),MB_ICONERROR) ;
        exit(1) ;
    }
    else if(strcmp(buff,"ver#"))
    {   
        int kk, hkc[5], hks[3] ;
        ia[7] = ii ;
        hotkeyCount = 0 ;
        fscanf(fp, "%s%i", (char *) buff, ia + 9);
        fscanf(fp, "%s%i", (char *) buff, &jj);
        fscanf(fp, "%s%i", (char *) buff, ia + 17);
        fscanf(fp, "%s%i", (char *) buff, &ii);
        fscanf(fp, "%s%i", (char *) buff, hk + 0);
        fscanf(fp, "%s%i", (char *) buff, hk + 1);
        fscanf(fp, "%s%i", (char *) buff, hk + 2);
        fscanf(fp, "%s%i", (char *) buff, hk + 3);
        if(jj)
        {
            jj = hk[0] | hk[1] | hk[2] | hk[3] | vwHOTKEY_EXT ;
            ii = 3 ;
            do {
                hotkeyList[ii].modifier = jj ;
                hotkeyList[ii].command = vwCMD_NAV_MOVE_LEFT + ii ;
                hotkeyList[ii].desk = 0 ;
            } while(--ii >= 0) ;
            hotkeyList[0].key = VK_LEFT ;
            hotkeyList[1].key = VK_RIGHT ;
            hotkeyList[2].key = VK_UP ;
            hotkeyList[3].key = VK_DOWN ;
            hotkeyCount = 4 ;
        }
        fscanf(fp, "%s%i", (char *) buff, ia + 8);
        fscanf(fp, "%s%i", (char *) buff, ia + 16);
        fscanf(fp, "%s%i", (char *) buff, &ii);
        fscanf(fp, "%s%i", (char *) buff, ia + 1);
        fscanf(fp, "%s%i", (char *) buff, ia + 0);
        fscanf(fp, "%s%i", (char *) buff, &ii);
        for(ii=1,jj=9,kk=0 ; ii<=jj ; ii++)
        {
            fscanf(fp, "%s%i", (char *) buff, hk + 1);
            if((ii==1) && !strcmp((char *) buff,"Desk_count#"))
            {
                jj = hk[1] ;
                hk[1] = 0 ;
                ii = 0 ;
                kk = 1 ;
            }
            else
            {
                fscanf(fp, "%s%i", (char *) buff, hk + 2);
                fscanf(fp, "%s%i\n", (char *) buff, hk + 3);
                if(hk[1])
                    addOldHotkey(hk[1],hk[2],hk[3],vwCMD_NAV_MOVE_DESKTOP,ii) ;
                if(kk && (_fgetts(buff2,2048,fp) != NULL) && ((ss=_tcschr(buff2,' ')) != NULL) &&
                   ((ll=_tcslen(++ss)) > 1))
                {
                    if(ss[ll-1] == '\n')
                        ss[ll-1] = '\0' ;
                    desktopName[ii] = _tcsdup(ss) ;
                }
            }
        }
        fscanf(fp, "%s%i", (char *) buff, ia + 12);
        ia[13] = 0 ;
        fscanf(fp, "%s%i", (char *) buff, &ii);
        if(ii)  ia[13] |= vwHOTKEY_ALT ;
        fscanf(fp, "%s%i", (char *) buff, &ii);
        if(ii)  ia[13] |= vwHOTKEY_SHIFT ;
        fscanf(fp, "%s%i", (char *) buff, &ii);
        if(ii)  ia[13] |= vwHOTKEY_CONTROL ;
        fscanf(fp, "%s%i", (char *) buff, &ii);
        fscanf(fp, "%s%i", (char *) buff, ia + 18);
        fscanf(fp, "%s%i", (char *) buff, ia + 10);
        ia[10] = (ia[10] == 0) ;
        fscanf(fp, "%s%i", (char *) buff, hks + 0);
        fscanf(fp, "%s%i", (char *) buff, hks + 1);
        fscanf(fp, "%s%i", (char *) buff, ia + 14);
        fscanf(fp, "%s%i", (char *) buff, ia + 2);
        fscanf(fp, "%s%i", (char *) buff, ia + 19);
        ia[5] = 0 ;
        fscanf(fp, "%s%i", (char *) buff, &ii);
        if(ii)  ia[5] |= vwWINLIST_STICKY ;
        fscanf(fp, "%s%i", (char *) buff, &ii);
        if(ii)  ia[5] |= vwWINLIST_ASSIGN ;
        fscanf(fp, "%s%i", (char *) buff, &ii);
        if(ii)  ia[5] |= vwWINLIST_ACCESS ;
        fscanf(fp, "%s%i", (char *) buff, ia + 3);
        fscanf(fp, "%s%i", (char *) buff, &ii);
        fscanf(fp, "%s%i", (char *) buff, &ii);
        fscanf(fp, "%s%i", (char *) buff, hkc + 0);
        fscanf(fp, "%s%i", (char *) buff, hkc + 1);
        fscanf(fp, "%s%i", (char *) buff, hkc + 2);
        fscanf(fp, "%s%i", (char *) buff, hkc + 3);
        fscanf(fp, "%s%i", (char *) buff, hkc + 4);
        fscanf(fp, "%s%i", (char *) buff, hk + 0);
        fscanf(fp, "%s%i", (char *) buff, hk + 1);
        fscanf(fp, "%s%i", (char *) buff, hk + 2);
        fscanf(fp, "%s%i", (char *) buff, hk + 3);
        if(hk[0])
        {
            kk = hotkeyCount ;
            addOldHotkey(hk[1],hk[2],hk[3],vwCMD_UI_WINLIST_STD,0) ;
        }
        else
            kk = -1 ;
        fscanf(fp, "%s%i", (char *) buff, ia + 22);
        fscanf(fp, "%s%i", (char *) buff, hks + 2);
        fscanf(fp, "%s%i", (char *) buff, ia + 20);
        fscanf(fp, "%s%i", (char *) buff, ia + 21);
        fscanf(fp, "%s%i", (char *) buff, &ii);
        fscanf(fp, "%s%i", (char *) buff, &ii);
        fscanf(fp, "%s%i", (char *) buff, &ii);
        fscanf(fp, "%s%i", (char *) buff, &jj);
        if(hkc[0])
        {
            addOldHotkey(hkc[1],hkc[2],ii,vwCMD_NAV_MOVE_NEXT,0) ;
            addOldHotkey(hkc[3],hkc[4],jj,vwCMD_NAV_MOVE_PREV,0) ;
        }
        fscanf(fp, "%s%i", (char *) buff, &ii);
        if(ii)
            addOldHotkey(hks[1],hks[0],hk[2],vwCMD_WIN_STICKY,0) ;
        fscanf(fp, "%s%i", (char *) buff, &ii);
        fscanf(fp, "%s%i", (char *) buff, &ii);
        fscanf(fp, "%s%i", (char *) buff, &ii);
        fscanf(fp, "%s%i", (char *) buff, ia + 4);
        fscanf(fp, "%s%i", (char *) buff, ia + 15);
        fscanf(fp, "%s%i", (char *) buff, &ii);
        fscanf(fp, "%s%i", (char *) buff, hk + 0);
        fscanf(fp, "%s%i", (char *) buff, hk + 1);
        fscanf(fp, "%s%i", (char *) buff, hk + 2);
        fscanf(fp, "%s%i", (char *) buff, hk + 3);
        if(hk[0])
            addOldHotkey(hk[1],hk[2],hk[3],vwCMD_WIN_DISMISS,0) ;
        fscanf(fp, "%s%i", (char *) buff, ia + 23);
        fscanf(fp, "%s%i", (char *) buff, ia + 11);
        fscanf(fp, "%s%i", (char *) buff, ia + 6);
        if(ia[6] && (kk >= 0))
            hotkeyList[kk].command = vwCMD_UI_WINMENU_CMP ;
        fscanf(fp, "%s%i", (char *) buff, hk + 0);
        fscanf(fp, "%s%i", (char *) buff, hk + 1);
        fscanf(fp, "%s%i", (char *) buff, hk + 2);
        fscanf(fp, "%s%i", (char *) buff, hk + 3);
        if(hk[0])
            addOldHotkey(hk[1],hk[2],hk[3],vwCMD_UI_WINMENU_STD,0) ;
        fscanf(fp, "%s%i", (char *) buff, &ii);
        if(ii)  ia[5] |= vwWINLIST_SHOW ;
        fclose(fp);
    }
    else if(ii == 2)
    {
        /* read the hotkeys and desktop names */
        hotkeyCount = 0 ;
        fscanf(fp, "%s%d", (char *) buff, &jj);
        for(ii=0 ; ii<jj ; ii++) 
        {
            if(fscanf(fp, "%s %d %d %d %d", (char *) buff, hk+0, hk+1, hk+2, hk+3) == 5)
            {
                hotkeyList[hotkeyCount].key = hk[0] ;
                hotkeyList[hotkeyCount].modifier = hk[1] ;
                hotkeyList[hotkeyCount].command = hk[2] ;
                hotkeyList[hotkeyCount].desk = hk[3] ;
                hotkeyCount++ ;
            }
        }
        ii = vwDESKTOP_SIZE ;
        while(--ii >= 0)
        {
            if(desktopName[ii] != NULL)
            {
                free(desktopName[ii]) ;
                desktopName[ii] = NULL ;
            }
        }
        fscanf(fp, "%s%d\n", (char *) buff, &jj);
        for(ii=1 ; ii<=jj ; ii++)
        {
            if((_fgetts(buff2,2048,fp) != NULL) && ((ss=_tcschr(buff2,' ')) != NULL) &&
               ((ll=_tcslen(++ss)) > 1))
            {
                if(ss[ll-1] == '\n')
                    ss[ll-1] = '\0' ;
                desktopName[ii] = _tcsdup(ss) ;
            }
        }
        /* now read all the simple flags */
        for(ii=0 ; ii<24 ; ii++) 
            fscanf(fp, "%s%d", (char *) buff, ia+ii);
        fclose(fp);
    }
    else
    {
        fclose(fp);
        _stprintf(buff2,_T("Error reading config file:\n\n    %s\n\nUnsupported version %d, please remove."),buff,ii);
        MessageBox(hWnd,buff2,vwVIRTUAWIN_NAME _T(" Error"),MB_ICONERROR) ;
        exit(1) ;
    }
    nDesksX = ia[0] ;
    nDesksY = ia[1] ;
    nDesks = nDesksX * nDesksY ;
    deskWrap = ia[2] ;
    useDeskAssignment = ia[3] ;
    assignImmediately = ia[4] ;
    winListContent = ia[5] ;
    winListCompact = ia[6] ;
    mouseEnable = ia[7] ;
    mouseJumpLength = ia[8] ;
    mouseDelay = ia[9] ;
    mouseWarp = ia[10] ;
    mouseKnock = ia[11] ;
    mouseModifierUsed = ia[12] ;
    mouseModifier = ia[13] ;
    preserveZOrder = ia[14] ;
    hiddenWindowAct = ia[15] ;
    minSwitch = ia[16] ;
    releaseFocus = ia[17] ;
    refreshOnWarp = ia[18] ;
    invertY = ia[19] ;
    noTaskbarCheck = ia[20] ;
    trickyWindows = ia[21] ;
    displayTaskbarIcon = ia[22] ;
    vwLogFlag = ia[23] ;
}
