//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  ModuleRoutines.c - Module handling routines.
// 
//  Copyright (c) 1999-2005 Johan Piculell
//  Copyright (c) 2006-2010 VirtuaWin (VirtuaWin@home.se)
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
#include "Messages.h"
#include "ConfigParameters.h"
#include "DiskRoutines.h"

// Standard includes
#include <io.h>
#include <string.h>

/*************************************************
 * Checks if a module is disabled
 */
static int
vwModuleCheckDisabled(TCHAR *theModName)
{
    int modIndex;
    
    for (modIndex = 0; modIndex < curDisabledMod; ++modIndex)
        if(!_tcsncmp(disabledModules[modIndex].moduleName,theModName,(_tcslen(theModName) - 4)))
            return TRUE; // Module disabled
    return FALSE;  // Not disabled
}

/*************************************************
 * Adds a module to a list, found by vwModulesLoad()
 */
static void
vwModuleAdd(TCHAR *moduleName, TCHAR *path)
{
    TCHAR buff[MAX_PATH];
    HWND myModule;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;  
    int rv1, rv2 ;
    
    if(moduleCount >= MAXMODULES)
    {
        _stprintf(buff,_T("Max number of modules have been added, '%s' won't be loaded."), moduleName);
        MessageBox(hWnd,buff,vwVIRTUAWIN_NAME _T(" Error"),MB_ICONWARNING);
        return;
    }
    
    if(vwModuleCheckDisabled(moduleName))
    {
        // The module is disabled
        moduleList[moduleCount].handle = NULL;
        moduleList[moduleCount].disabled = TRUE;
        moduleName[_tcslen(moduleName)-4] = '\0'; // remove .exe
        _tcsncpy(moduleList[moduleCount].description, moduleName, 79);
        moduleCount++;
        return;
    }
    
    if((myModule = vwFindWindow(moduleName,NULL,0)) != NULL)
    {
        _stprintf(buff,_T("The module '%s' seems to already be running and will be re-used. This is probably due to incorrect shutdown of VirtuaWin."), moduleName);
        MessageBox(hWnd,buff,vwVIRTUAWIN_NAME _T(" Error"),MB_ICONWARNING);
    }
    else
    {
        // Launch the module
        buff[0] = '"' ;
        _tcscpy(buff+1,path) ;
        _tcscat(buff,moduleName) ;
        _tcscat(buff,_T("\" -module")) ;
        memset(&si, 0, sizeof(si)); 
        si.cb = sizeof(si); 
        if(!CreateProcess(NULL,buff,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
        {
            _stprintf(buff,_T("Failed to launch module '%s'. (Err %d)"),moduleName,(int) GetLastError());
            MessageBox(hWnd,buff,vwVIRTUAWIN_NAME _T(" Error"),MB_ICONWARNING);
            return;
        }
        CloseHandle(pi.hThread) ;
        // Wait max 20 sec for the module to initialize itself then close the process handle
        rv1 = WaitForInputIdle(pi.hProcess, 20000); 
        
        // Find the module with classname 
        if((myModule = vwFindWindow(moduleName,NULL,0)) == NULL)
        {
            Sleep(500) ;
            rv2 = WaitForInputIdle(pi.hProcess,10000) ;
            if((myModule = vwFindWindow(moduleName,NULL,1)) == NULL)
            {
                CloseHandle(pi.hProcess) ;
                _stprintf(buff,_T("Failed to load module '%s' - maybe wrong class or file name? (Err %d, %d, %d)"),moduleName,rv1,rv2,(int) GetLastError());
                MessageBox(hWnd,buff,vwVIRTUAWIN_NAME _T(" Error"),MB_ICONWARNING);
                return;
            }
        }
        CloseHandle(pi.hProcess) ;
    }
    moduleList[moduleCount].handle = myModule;
    moduleList[moduleCount].disabled = FALSE;
    moduleName[_tcslen(moduleName)-4] = '\0'; // remove .exe
    _tcsncpy(moduleList[moduleCount].description, moduleName, 79);
    PostMessage(myModule, MOD_INIT, (WPARAM) hWnd , 0);
    moduleCount++;
}

/*************************************************
 * Locates modules in "Modules" directory, that is 
 * all files with an .exe extension
 */
void
vwModulesLoad(void)
{
    WIN32_FIND_DATA exe_file;
    TCHAR buff[MAX_PATH], *ss ;
    HANDLE hFile;
    
    GetFilename(vwMODULES,0,buff);
    
    // Find first .exe file in modules directory
    if((hFile = FindFirstFile(buff,&exe_file)) != INVALID_HANDLE_VALUE)
    {
        if((ss = _tcsrchr(buff,'\\')) != NULL)
            ss[1] = '\0' ;
        do {
            vwModuleAdd(exe_file.cFileName,buff);
        } while(FindNextFile(hFile,&exe_file)) ;
        
        FindClose(hFile);
    }
}

/*************************************************
 * Sends a message to all modules in the list
 */
void
vwModulesSendMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    int index;
    for(index = 0; index < moduleCount; ++index)
        if(moduleList[index].handle != NULL) 
            SendMessageTimeout(moduleList[index].handle,Msg,wParam,lParam,SMTO_ABORTIFHUNG|SMTO_BLOCK,10000,NULL);
}

/*************************************************
 * Posts a message to all modules in the list
 */
void
vwModulesPostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    int index;
    for(index = 0; index < moduleCount; ++index) {
        if(moduleList[index].handle != NULL)
            PostMessage(moduleList[index].handle, Msg, wParam, lParam);
    }
}
