//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  ModuleRoutines.c - Module handling routines.
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

// Includes
#include "VirtuaWin.h"
#include "ModuleRoutines.h"
#include "Messages.h"
#include "ConfigParameters.h"
#include "DiskRoutines.h"

// Standard includes
#include <io.h>
#include <string.h>


/*************************************************
 * Unloads all modules in the module list
 */
void unloadModules(void)
{
    sendModuleMessage(MOD_QUIT, 0, 0);
}

/*************************************************
 * Checks if a module is disabled
 */
static BOOL checkDisabledList(TCHAR *theModName)
{
    int modIndex;
    
    for (modIndex = 0; modIndex < curDisabledMod; ++modIndex)
        if(!_tcsncmp(disabledModules[modIndex].moduleName,theModName,(_tcslen(theModName) - 4)))
            return TRUE; // Module disabled
    return FALSE;  // Not disabled
}

/*************************************************
 * Adds a module to a list, found by loadModules()
 */
static void addModule(TCHAR *moduleName, TCHAR *path)
{
    TCHAR tmpPath[MAX_PATH];
    TCHAR errMsg[150];
    HWND myModule;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;  
    int retVal = 1;
    
    if(nOfModules >= MAXMODULES)
    {
        _stprintf(errMsg,_T("Max number of modules where added.\n'%s' won't be loaded."), moduleName);
        MessageBox(hWnd, errMsg,vwVIRTUAWIN_NAME _T(" Error"),MB_ICONWARNING);
        return;
    }
    
    // Is the module disabled
    if(!checkDisabledList(moduleName))
    {
        if((myModule = FindWindow(moduleName, NULL)))
        {
            _stprintf(errMsg,_T("The module '%s' seems to already be running and will be re-used. \nThis is probably due to incorrect shutdown of VirtuaWin"), moduleName);
            MessageBox(hWnd,errMsg,vwVIRTUAWIN_NAME _T(" Error"),MB_ICONWARNING);
        }
        else
        {
            // Startup the module
            tmpPath[0] = '"' ;
            _tcscpy(tmpPath+1,path) ;
            _tcscat(tmpPath,moduleName) ;
            _tcscat(tmpPath,_T("\" -module")) ;
            memset(&si, 0, sizeof(si)); 
            si.cb = sizeof(si); 
            if(!CreateProcess(NULL, tmpPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            {
                TCHAR *lpszLastErrorMsg; 
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
                              GetLastError(), 
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language 
                              (TCHAR *) &lpszLastErrorMsg, 
                              0, 
                              NULL ); 
                
                _stprintf(errMsg,_T("Failed to load module '%s'.\n %s"), moduleName, lpszLastErrorMsg);
                MessageBox(hWnd,errMsg,vwVIRTUAWIN_NAME _T(" Error"),MB_ICONWARNING);
                return;
            }
            // Wait max 20 sec for the module to initialize itself
            retVal = WaitForInputIdle( pi.hProcess, 20000); 
            
            // Find the module with classname 
            myModule = FindWindow(moduleName, NULL);
        }
        if(!myModule)
        {
            _stprintf(errMsg,_T("Failed to load module '%s'.\n Maybe wrong class/filename.\nErrcode %d"),moduleName,retVal);
            MessageBox(hWnd,errMsg,vwVIRTUAWIN_NAME _T(" Error"),MB_ICONWARNING);
        }
        else
        {
            moduleList[nOfModules].Handle = myModule;
            moduleList[nOfModules].Disabled = FALSE;
            moduleName[_tcslen(moduleName)-4] = '\0'; // remove .exe
            _tcsncpy(moduleList[nOfModules].description, moduleName, 79);
            PostMessage(myModule, MOD_INIT, (WPARAM) hWnd , 0);
            nOfModules++;
        }
    } 
    else
    { // Module disabled
        moduleList[nOfModules].Handle = NULL;
        moduleList[nOfModules].Disabled = TRUE;
        moduleName[_tcslen(moduleName)-4] = '\0'; // remove .exe
        _tcsncpy(moduleList[nOfModules].description, moduleName, 79);
        nOfModules++;
    }
}

/*************************************************
 * Locates modules in "Modules" directory, that is 
 * all files with an .exe extension
 */
void loadModules(void)
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
            addModule(exe_file.cFileName,buff);
        } while(FindNextFile(hFile,&exe_file)) ;
        
        FindClose(hFile);
    }
}

/*************************************************
 * Sends a message to all modules in the list
 */
void sendModuleMessage(UINT Msg, WPARAM wParam,	LPARAM lParam)
{
    int index;
    for(index = 0; index < nOfModules; ++index)
        if(moduleList[index].Handle != NULL) 
            SendMessageTimeout(moduleList[index].Handle,Msg,wParam,lParam,SMTO_ABORTIFHUNG|SMTO_BLOCK,10000,NULL);
}

/*************************************************
 * Posts a message to all modules in the list
 */
void postModuleMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    int index;
    for(index = 0; index < nOfModules; ++index) {
        if(moduleList[index].Handle != NULL)
            PostMessage(moduleList[index].Handle, Msg, wParam, lParam);
    }
}
