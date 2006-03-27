//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K/XP
// 
//  Copyright (c) 1999-2003, 2004 Johan Piculell
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <errno.h>
#include <assert.h>
#include <shlobj.h>  // for SHGetFolderPath
#include <winbase.h> // for GetTempPath
#include <stddef.h>  // for size_t
#include <direct.h>  // for mkdir
#include <winreg.h>  // For regisry calls

#include "VirtuaWin.h"
#include "DiskRoutines.h"
#include "ConfigParameters.h"

#define VIRTUAWIN_SUBDIR vwVIRTUAWIN_NAME

char *VirtuaWinPath=NULL ;
char *UserAppPath=NULL ;

/************************************************** 
 * Gets the local application settings path for the current user.
 * Calling function MUST pre-allocate the return string!
 */
static void getUserAppPath(char* path)
{
    LPITEMIDLIST idList ;
    int len ;
    
    path[0] = '\0' ;
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
        len = strlen(path) ;
        if(path[len - 1] != '\\')
            path[len++] = '\\' ;
        strncpy(path+len,VIRTUAWIN_SUBDIR,MAX_PATH - len) ;
        len += strlen(path+len) ;
        path[len++] = '\\' ;
        path[len] = '\0' ;
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

int GetFilename(eFileNames filetype, int location, char* outStr)
{
    static char *subPath[vwFILE_COUNT] = {
        "modules\\*.exe", "virtuawin", "virtuawin.cfg", "userlist.cfg", "tricky.cfg",
        "sticky.cfg", "module.cfg", "assignment.cfg"
    };
    DWORD len ;
    
    if(UserAppPath == NULL)
    {
        /* initialization of paths, find the installation and user paths,
         * exit on failure - initialization happens so early on it is safe to
         * simply exit */
        char path[MAX_PATH], *ss ;
        
        GetModuleFileName(GetModuleHandle(NULL),path,MAX_PATH) ;
        ss = strrchr(path,'\\') ;
        ss[1] = '\0' ;
        VirtuaWinPath = strdup(path) ;
        getUserAppPath(path) ;  // for now I only want multiuser, we'll probably get rid of the registry entry
        if(path[0] == '\0')
            UserAppPath = VirtuaWinPath ;
        else
            UserAppPath = strdup(path) ;
        if((VirtuaWinPath == NULL) || (UserAppPath == NULL))
        {
            MessageBox(hWnd, "Memory resources appear to be very low, try rebooting.\nIf you still have problems, send a mail to \n" vwVIRTUAWIN_EMAIL,vwVIRTUAWIN_NAME " Error", MB_ICONWARNING);
            exit(1);
        }
    }
    
    if(filetype >= vwFILE_COUNT)
        return 0 ;
    strncpy(outStr,(location) ? UserAppPath:VirtuaWinPath, MAX_PATH);
    len = MAX_PATH - strlen(outStr) ;
    strncat(outStr,subPath[filetype],len) ;
    return 1;
}


/*************************************************
 * Write out the disabled modules
 */
void saveDisabledList(int theNOfModules, moduleType* theModList)
{
    FILE* fp;
    
    char DisabledFileList[MAX_PATH];
    GetFilename(vwDISABLED,1,DisabledFileList);
    
    if(!(fp = fopen(DisabledFileList, "w")))
        MessageBox(hWnd, "Error saving disabled module state", NULL, MB_ICONWARNING);
    else
    {
        int i;
        for(i = 0; i < theNOfModules; ++i)
            if(theModList[i].Disabled)
                fprintf(fp, "%s\n", (char*)&theModList[i].description);
        fclose(fp);
    }
}

/*************************************************
 * Loads module names that should be disabled 
 */
int loadDisabledModules(disModules *theDisList) 
{
    char buff[MAX_PATH];
    int len, nOfDisMod = 0;
    FILE *fp;
    
    GetFilename(vwDISABLED,1,buff);
    
    if((fp = fopen(buff,"r")) != NULL)
    {
        while(fgets(buff,MAX_PATH,fp) != NULL)
        {
            if((len = strlen(buff)) > 1)
            {
                if(len > vwMODULENAME_MAX)
                    buff[vwMODULENAME_MAX] = '\0' ;
                else if(buff[len-1] == '\n')
                    buff[len-1] = '\0' ;
                strcpy(theDisList[nOfDisMod++].moduleName,buff);
                if(nOfDisMod == (MAXMODULES * 2))
                    break ;
            }
        }
        fclose(fp);
    }
    return nOfDisMod;
}

/*************************************************
 * Loads window classnames from sticky file
 */
int loadStickyList(stickyType *theStickyList) 
{
    char buff[MAX_PATH];
    int len, nOfSticky = 0, winClassLen;
    FILE* fp;
    
    GetFilename(vwSTICKY,1,buff);
    
    if((fp = fopen(buff,"r")) != NULL)
    {
        while(fgets(buff,MAX_PATH,fp) != NULL)
        {
            if((len = strlen(buff)) > 1)
            {
                winClassLen = vwCLASSNAME_MAX ;
                if(len > vwCLASSNAME_MAX)
                    buff[vwCLASSNAME_MAX] = '\0' ;
                else
                {
                    if(buff[len-1] == '\n')
                        len-- ;
                    if(buff[len-1] == '*')
                    {
                        len-- ;
                        winClassLen = len ;
                    }
                    buff[len] = '\0' ;
                }                    
                if((theStickyList[nOfSticky].winClassName = strdup(buff)) != NULL)
                {
                    theStickyList[nOfSticky].winClassLen = winClassLen ;
                    nOfSticky++;
                }
            }
        }
        fclose(fp);
    }
    return nOfSticky;
}

/*************************************************
 * Writes down the classnames of the sticky windows on file
 * File format:
 *   <WinClassName>\n
 *   <WinClassName>\n
 */
void saveStickyWindows(int theNOfWin, windowType *theWinList)
{
    char className[vwCLASSNAME_MAX+2];
    FILE* fp;
    
    char vwStickyList[MAX_PATH];
    GetFilename(vwSTICKY,1,vwStickyList);
    
    if((fp = fopen(vwStickyList, "w")) == NULL)
        MessageBox(hWnd, "Error writing sticky file", NULL, MB_ICONWARNING);
    else
    {
        int i;
        for(i = 0; i < theNOfWin; ++i)
        {
            if(theWinList[i].Sticky)
            {
                GetClassName(theWinList[i].Handle,className,vwCLASSNAME_MAX);
                className[vwCLASSNAME_MAX] = '\0' ;
                fprintf(fp, "%s\n",className);
            }
        }
        fclose(fp);
    }
}

/*************************************************
 * Loads window classnames from tricky file
 */
int loadTrickyList(stickyType *theTrickyList) 
{
    char buff[MAX_PATH];
    int len, nOfTricky = 0, winClassLen;
    FILE* fp;
    
    GetFilename(vwTRICKY,1,buff);
    if((fp = fopen(buff, "r")) != NULL) 
    {
        while(fgets(buff,MAX_PATH,fp) != NULL)
        {
            if((len = strlen(buff)) > 1)
            {
                winClassLen = vwCLASSNAME_MAX ;
                if(len > vwCLASSNAME_MAX)
                    buff[vwCLASSNAME_MAX] = '\0' ;
                else
                {
                    if(buff[len-1] == '\n')
                        len-- ;
                    if(buff[len-1] == '*')
                    {
                        len-- ;
                        winClassLen = len ;
                    }
                    buff[len] = '\0' ;
                }                    
                if((theTrickyList[nOfTricky].winClassName = strdup(buff)) != NULL)
                {
                    theTrickyList[nOfTricky].winClassLen = winClassLen ;
                    nOfTricky++;
                }
            }
        }
        fclose(fp);
    }
    return nOfTricky;
}


/*************************************************
 * Writes down the current desktop layout to a file
 * File format:
 *   <desk #> <WinClassName>\n
*   <desk #> <WinClassName>\n
*/
void saveAssignedList(int theNOfWin, windowType *theWinList)
{
    char buff[MAX_PATH];
    FILE *fp;
    
    GetFilename(vwWINDOWS_STATE,1,buff);
    if((fp = fopen(buff, "w")) == NULL)
        MessageBox(hWnd, "Error writing desktop configuration file", NULL, MB_ICONWARNING);
    else
    {
        int i;
        for(i = 0; i < theNOfWin; ++i)
        {
            GetClassName(theWinList[i].Handle,buff,vwCLASSNAME_MAX);
            buff[vwCLASSNAME_MAX] = '\0' ;
            fprintf(fp, "%d %s\n",theWinList[i].Desk,buff);
        }
        fclose(fp);
    }
}

/*************************************************
 * Loads the list with classnames that has an desktop assigned
 */
int loadAssignedList(assignedType *theAssignList) 
{
    char buff[MAX_PATH], *ss ;
    int curAssigned = 0, desk, len, winClassLen ;
    FILE *fp;
    
    GetFilename(vwWINDOWS_STATE,1,buff) ;
    if((fp = fopen(buff, "r")) != NULL)
    {
        while(fgets(buff,MAX_PATH,fp) != NULL)
        {
            
            if(((desk = atoi(buff)) > 0) &&
               ((ss=strchr(buff,' ')) != NULL) && ((len = strlen(ss+1)) > 1))
            {
                ss += 1 ;
                winClassLen = vwCLASSNAME_MAX ;
                if(len > vwCLASSNAME_MAX)
                    ss[vwCLASSNAME_MAX] = '\0' ;
                else
                {
                    if(ss[len-1] == '\n')
                        len-- ;
                    if(ss[len-1] == '*')
                    {
                        len-- ;
                        winClassLen = len ;
                    }
                    ss[len] = '\0' ;
                }
                if((theAssignList[curAssigned].winClassName = strdup(ss)) != NULL)
                {
                    theAssignList[curAssigned].winClassLen = winClassLen ;
                    theAssignList[curAssigned].desktop = desk ;
                    curAssigned++;
                }
            }
        }
        fclose(fp);
    }
    return curAssigned;
}

/*************************************************
 * Loads window titles/classnames from user file 
 */
int loadUserList(userType* theUserList) 
{
    char buff[MAX_PATH];
    FILE* fp;
    int curUser = 0;
    
    GetFilename(vwLIST,1,buff);
    
    if((fp = fopen(buff, "r")))
    {
        while(!feof(fp))
        {
            fgets(buff, 99, fp);
            // Remove the newline
            if(buff[strlen(buff) - 1] == '\n')
                buff[strlen(buff) - 1] = '\0';
            if(curUser < MAXUSER && buff[0] != ':' && (strlen(buff) != 0) && !feof(fp) &&
               ((theUserList[curUser].winNameClass = strdup(buff)) != NULL))
            {
                theUserList[curUser].isClass = TRUE;
                curUser++;
            } 
        }
        fclose(fp);
    }
    
    return curUser;
}

/************************************************
 * Writes down the current configuration on file
 */
void writeConfig(void)
{
    FILE* fp;
    int ii ;
    
    char VWConfigFile[MAX_PATH];
    GetFilename(vwCONFIG,1,VWConfigFile);
    
    if((fp = fopen(VWConfigFile, "w")) == NULL) {
        MessageBox(NULL, "Error writing config file", NULL, MB_ICONWARNING);
    } else {
        fprintf(fp, "Mouse_warp# %i\n", mouseEnable);
        fprintf(fp, "Mouse_delay# %i\n", configMultiplier);
        fprintf(fp, "Key_support# %i\n", keyEnable);
        fprintf(fp, "Release_focus# %i\n", releaseFocus);
        fprintf(fp, "Keep_active# %i\n", 1);
        fprintf(fp, "Control_key_alt# %i\n", modAlt);
        fprintf(fp, "Control_key_shift# %i\n", modShift);
        fprintf(fp, "Control_key_ctrl# %i\n", modCtrl);
        fprintf(fp, "Control_key_win# %i\n", modWin);
        fprintf(fp, "Warp_jump# %i\n", warpLength);
        fprintf(fp, "Switch_minimized# %i\n", minSwitch);
        fprintf(fp, "Taskbar_warp# %i\n", taskBarWarp);
        fprintf(fp, "Desk_Ysize# %i\n", nDesksY);
        fprintf(fp, "Desk_Xsize# %i\n", nDesksX);
        fprintf(fp, "Hot_key_support# %i\n", hotKeyEnable);
        for(ii=1 ; ii<10 ; ii++) {
            fprintf(fp, "Hot_key_%d# %i\n", ii,deskHotkey[ii]);
            fprintf(fp, "Hot_key_Mod%d# %i\n", ii,deskHotkeyMod[ii]);
            fprintf(fp, "Hot_key_Win%d# %i\n", ii,deskHotkeyWin[ii]);
        }
        fprintf(fp, "Mouse_control_key_support# %i\n", useMouseKey);
        fprintf(fp, "Mouse_key_alt# %i\n", mouseModAlt);
        fprintf(fp, "Mouse_key_shift# %i\n", mouseModShift);
        fprintf(fp, "Mouse_key_ctrl# %i\n", mouseModCtrl);
        fprintf(fp, "Save_sticky_info# %i\n", saveSticky);
        fprintf(fp, "Refresh_after_warp# %i\n", refreshOnWarp);
        fprintf(fp, "No_mouse_wrap# %i\n", noMouseWrap);
        fprintf(fp, "Sticky_modifier# %i\n", hotkeyStickyMod);
        fprintf(fp, "Sticky_key# %i\n", hotkeySticky);
        fprintf(fp, "Preserve_zorder# %i\n", preserveZOrder);
        fprintf(fp, "Desktop_cycling# %i\n", deskWrap);
        fprintf(fp, "Invert_Y# %i\n", invertY);
        fprintf(fp, "WinMenu_sticky# %i\n", stickyMenu);
        fprintf(fp, "WinMenu_assign# %i\n", assignMenu);
        fprintf(fp, "WinMenu_direct# %i\n", directMenu);
        fprintf(fp, "Desktop_assignment# %i\n", useDeskAssignment);
        fprintf(fp, "Save_layout# %i\n", saveLayoutOnExit);
        fprintf(fp, "Assign_first# %i\n", assignOnlyFirst);
        fprintf(fp, "UseCyclingKeys# %i\n", cyclingKeysEnabled);
        fprintf(fp, "CycleUp# %i\n", hotCycleUp);
        fprintf(fp, "CycleUpMod# %i\n", hotCycleUpMod);
        fprintf(fp, "CycleDown# %i\n", hotCycleDown);
        fprintf(fp, "CycleDownMod# %i\n", hotCycleDownMod);
        fprintf(fp, "Hot_key_Menu_Support# %i\n", hotkeyMenuEn);
        fprintf(fp, "Hot_key_Menu# %i\n", hotkeyMenu);
        fprintf(fp, "Hot_key_ModMenu# %i\n", hotkeyMenuMod);
        fprintf(fp, "Hot_key_WinMenu# %i\n", hotkeyMenuWin);
        fprintf(fp, "Display_systray_icon# %i\n", displayTaskbarIcon);
        fprintf(fp, "Sticky_Win# %i\n", hotkeyStickyWin);
        fprintf(fp, "Taskbar_detection# %i\n", noTaskbarCheck);
        fprintf(fp, "Use_trickywindows# %i\n", trickyWindows);
        fprintf(fp, "XPStyleTaskbar# %i\n", taskbarOffset);
        fprintf(fp, "PermanentSticky# %i\n", permanentSticky);
        fprintf(fp, "CycleUpWin# %i\n", hotCycleUpWin);
        fprintf(fp, "CycleDownWin# %i\n", hotCycleDownWin);
        fprintf(fp, "Sticky_Win_En# %i\n", hotkeyStickyEn);
        fprintf(fp, "Hot_key_10# %i\n",deskHotkey[10]);
        fprintf(fp, "Hot_key_Mod10# %i\n",deskHotkeyMod[10]);
        fprintf(fp, "Hot_key_Win10# %i\n",deskHotkeyWin[10]);
        fprintf(fp, "AssignImmediately# %i\n", assignImmediately);
        fprintf(fp, "HiddenWindowRaise# %i\n", hiddenWindowRaise);
        fprintf(fp, "HiddenWindowPopup# %i\n", hiddenWindowPopup);
        fprintf(fp, "DismissHotkeyEn# %i\n", hotkeyDismissEn);
        fprintf(fp, "DismissHotkey# %i\n", hotkeyDismiss);
        fprintf(fp, "DismissHotkeyMod# %i\n", hotkeyDismissMod);
        fprintf(fp, "DismissHotkeyWin# %i\n", hotkeyDismissWin);
        fclose(fp);
    }
}

/*************************************************
 * Reads a saved configuration from file
 */
void readConfig(void)
{
    char buff[MAX_PATH], buff2[2048], *ss ;
    FILE *fp, *wfp;
    int ii, jj ;
    
    GetFilename(vwCONFIG,1,buff);
    if(GetFileAttributes(buff) < 0)
    {
        /* config file does not exist - new user, setup configuration, check the user path exists first */
        ss = strrchr(buff,'\\') ;
        *ss = '\0' ;
        if(((GetFileAttributes(buff) & (0xf0000000|FILE_ATTRIBUTE_DIRECTORY)) != FILE_ATTRIBUTE_DIRECTORY) &&
           (CreateDirectory(buff,NULL) == 0))
        {
            sprintf(buff2,vwVIRTUAWIN_NAME " cannot create the user config directory:\n\n    %s\n\nPlease check file permissions. If you continue to have problems, send e-mail to:\n\n    " vwVIRTUAWIN_EMAIL,buff);
            MessageBox(hWnd,buff2,vwVIRTUAWIN_NAME " Error",MB_ICONERROR);
            exit(1) ;
        }
        /* now copy all the config files across to the user area */
        ii = vwFILE_COUNT ;
        while(--ii >= vwCONFIG)
        {
            GetFilename(ii,0,buff);
            if((fp = fopen(buff, "rb")) != NULL)
            {
                GetFilename(ii,1,buff2);
                if((wfp = fopen(buff2, "wb")) == NULL)
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
        GetFilename(vwCONFIG,1,buff);
        /* check a main config file has been copied, if not create a dummy one */
        if((ii < vwCONFIG) && (fp == NULL) &&
           (((wfp = fopen(buff, "wb")) == NULL) || (fclose(wfp) != 0)))
            ii = vwCONFIG ;
            
        /* check we did not break out due to an error */
        if(ii >= vwCONFIG)
        {
            MessageBox(hWnd, "Error occurred creating new user configuration, please check file permissions.\nIf you continue to have problems, send e-mail to:\n\n    " vwVIRTUAWIN_EMAIL,vwVIRTUAWIN_NAME " Error",MB_ICONERROR);
            exit(1) ;
        }
            
        sprintf(buff2,"Welcome to %s\n\nA new config been been created, right click on tray icon to access the Setup dialog.",vwVIRTUAWIN_NAME_VERSION) ;
        MessageBox(hWnd,buff2,vwVIRTUAWIN_NAME,MB_ICONINFORMATION);
    }
    if((fp = fopen(buff,"r")) == NULL)
    {
        sprintf(buff2, "Error reading config file:\n\n    %s\n\nPlease check file permissions. If you continue to have problems, send e-mail to:\n\n    " vwVIRTUAWIN_EMAIL,buff);
        MessageBox(hWnd,buff2,vwVIRTUAWIN_NAME " Error",MB_ICONERROR) ;
        exit(1) ;
    }
    else
    {   
        fscanf(fp, "%s%i", buff, &mouseEnable);
        fscanf(fp, "%s%i", buff, &configMultiplier);
        fscanf(fp, "%s%i", buff, &keyEnable);
        fscanf(fp, "%s%i", buff, &releaseFocus);
        fscanf(fp, "%s%i", buff, &ii);
        fscanf(fp, "%s%i", buff, &modAlt);
        fscanf(fp, "%s%i", buff, &modShift);
        fscanf(fp, "%s%i", buff, &modCtrl);
        fscanf(fp, "%s%i", buff, &modWin);
        fscanf(fp, "%s%i", buff, &warpLength);
        fscanf(fp, "%s%i", buff, &minSwitch);
        fscanf(fp, "%s%i", buff, &taskBarWarp);
        fscanf(fp, "%s%i", buff, &nDesksY);
        fscanf(fp, "%s%i", buff, &nDesksX);
        fscanf(fp, "%s%i", buff, &hotKeyEnable);
        for(ii=1 ; ii<10 ; ii++)
        {
            fscanf(fp, "%s%i", buff, deskHotkey + ii);
            fscanf(fp, "%s%i", buff, deskHotkeyMod + ii);
            fscanf(fp, "%s%i", buff, deskHotkeyWin + ii);
        }
        fscanf(fp, "%s%i", buff, &useMouseKey);
        fscanf(fp, "%s%i", buff, &mouseModAlt);
        fscanf(fp, "%s%i", buff, &mouseModShift);
        fscanf(fp, "%s%i", buff, &mouseModCtrl);
        fscanf(fp, "%s%i", buff, &saveSticky);
        fscanf(fp, "%s%i", buff, &refreshOnWarp);
        fscanf(fp, "%s%i", buff, &noMouseWrap);
        fscanf(fp, "%s%i", buff, &hotkeyStickyMod);
        fscanf(fp, "%s%i", buff, &hotkeySticky);
        fscanf(fp, "%s%i", buff, &preserveZOrder);
        fscanf(fp, "%s%i", buff, &deskWrap);
        fscanf(fp, "%s%i", buff, &invertY);
        fscanf(fp, "%s%hi", buff, &stickyMenu);
        fscanf(fp, "%s%hi", buff, &assignMenu);
        fscanf(fp, "%s%hi", buff, &directMenu);
        fscanf(fp, "%s%i", buff, &useDeskAssignment);
        fscanf(fp, "%s%i", buff, &saveLayoutOnExit);
        fscanf(fp, "%s%i", buff, &assignOnlyFirst);
        fscanf(fp, "%s%i", buff, &cyclingKeysEnabled);
        fscanf(fp, "%s%i", buff, &hotCycleUp);
        fscanf(fp, "%s%i", buff, &hotCycleUpMod);
        fscanf(fp, "%s%i", buff, &hotCycleDown);
        fscanf(fp, "%s%i", buff, &hotCycleDownMod);
        fscanf(fp, "%s%i", buff, &hotkeyMenuEn);
        fscanf(fp, "%s%i", buff, &hotkeyMenu);
        fscanf(fp, "%s%i", buff, &hotkeyMenuMod);
        fscanf(fp, "%s%i", buff, &hotkeyMenuWin);
        fscanf(fp, "%s%i", buff, &displayTaskbarIcon);
        fscanf(fp, "%s%i", buff, &hotkeyStickyWin);
        fscanf(fp, "%s%i", buff, &noTaskbarCheck);
        fscanf(fp, "%s%i", buff, &trickyWindows);
        fscanf(fp, "%s%i", buff, &taskbarOffset);
        fscanf(fp, "%s%i", buff, &permanentSticky);
        fscanf(fp, "%s%i", buff, &hotCycleUpWin);
        fscanf(fp, "%s%i", buff, &hotCycleDownWin);
        fscanf(fp, "%s%i", buff, &hotkeyStickyEn);
        fscanf(fp, "%s%i", buff, deskHotkey + 10);
        fscanf(fp, "%s%i", buff, deskHotkeyMod + 10);
        fscanf(fp, "%s%i", buff, deskHotkeyWin + 10);
        fscanf(fp, "%s%i", buff, &assignImmediately);
        fscanf(fp, "%s%i", buff, &hiddenWindowRaise);
        fscanf(fp, "%s%i", buff, &hiddenWindowPopup);
        fscanf(fp, "%s%i", buff, &hotkeyDismissEn);
        fscanf(fp, "%s%i", buff, &hotkeyDismiss);
        fscanf(fp, "%s%i", buff, &hotkeyDismissMod);
        fscanf(fp, "%s%i", buff, &hotkeyDismissWin);
        fclose(fp);
    }
}
