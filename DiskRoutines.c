//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K
// 
//  Copyright (c) 1999, 2000 jopi
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
#include "DiskRoutines.h"
#include "ConfigParameters.h"


/*************************************************
 * Routines for getting VirtuaWin path from the registry 
 * and fixup the different filenames
 */
void loadFilePaths()
{
   HKEY hkey = NULL;
   DWORD dwType;
   DWORD cbData = 0;
   long lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\VirtuaWin\\Settings", 0,
                               KEY_READ, &hkey);
   if (hkey) {
      lResult = RegQueryValueEx(hkey, "Path", NULL, &dwType, NULL, &cbData);
      vwPath   = (LPSTR)malloc(cbData * sizeof(char));
      vwConfig = (LPSTR)malloc(cbData * sizeof(char) + 13);
      vwList   = (LPSTR)malloc(cbData * sizeof(char) + 13);
      vwHelp   = (LPSTR)malloc(cbData * sizeof(char) + 10);
      vwSticky = (LPSTR)malloc(cbData * sizeof(char) + 11);
      vwState  = (LPSTR)malloc(cbData * sizeof(char) + 12);
      vwLock   = (LPSTR)malloc(cbData * sizeof(char) + 12);
      vwModules = (LPSTR)malloc(cbData * sizeof(char) + 15);
      vwDisabled = (LPSTR)malloc(cbData * sizeof(char) + 15);
      vwWindowsState = (LPSTR)malloc(cbData * sizeof(char) + 19);
      /* Get the key value. */
      lResult = RegQueryValueEx(hkey, "Path", NULL, &dwType,
                                (LPBYTE)vwPath, &cbData);

      sprintf(vwConfig, "%svwconfig.cfg", vwPath);
      sprintf(vwList, "%suserlist.cfg", vwPath);
      sprintf(vwHelp, "%svirtuawin", vwPath);
      sprintf(vwSticky, "%ssticky.cfg", vwPath);
      sprintf(vwState, "%svwstate.cfg", vwPath);
      sprintf(vwLock, "%s.vwLock.cfg", vwPath);
      sprintf(vwModules, "%smodules\\*.exe", vwPath);
      sprintf(vwDisabled, "%svwDisabled.cfg", vwPath);
      sprintf(vwWindowsState, "%svwWindowsState.cfg", vwPath);
   } else {
      MessageBox(hWnd, "VirtuaWin is not correctly installed, try to reinstall.\nIf you still have problems, send a mail to \nvirtuawin@iname.com", "Registry Error", MB_ICONWARNING);
      PostQuitMessage(0);
      //return 0;
   }
}

/*************************************************
 * Write out the disabled modules
 */
void writeDisabledList(int* theNOfModules, moduleType* theModList)
{
   FILE* fp;
   
   if(!(fp = fopen(vwDisabled, "w"))) {
      MessageBox(hWnd, "Error saving disabled module state", NULL, MB_ICONWARNING);
   } else {
      int i;
      for(i = 0; i < *theNOfModules; ++i) {
         if (theModList[i].Disabled) {
            fprintf(fp, "%s\n", &theModList[i].description);
         }
      }
      fclose(fp);
   }
}

/*************************************************
 * Loads module names that should be disabled 
 */
int loadDisabledModules(disModules* theDisList) 
{
   char dummy[81];
   FILE* fp;
   int nOfDisMod = 0;
   
   if(fp = fopen(vwDisabled, "r")) {
      while(!feof(fp)) {
         fgets(dummy, 80, fp);
         // Remove the newline
         if(dummy[strlen(dummy) - 1] == '\n') {
            dummy[strlen(dummy) - 1] = '\0';
         }
         if(nOfDisMod < (MAXMODULES * 2) && (strlen(dummy) != 0)) {
            strcpy(theDisList[nOfDisMod].moduleName, dummy);
            nOfDisMod++;
         } 
      }
      fclose(fp);
   } 
   return nOfDisMod;
}

/*************************************************
 * Loads window classnames from sticky file
 */
int loadStickyList(stickyType* theStickyList) 
{
   char dummy[80];
   FILE* fp;
   int nOfSticky = 0;
   
   if(fp = fopen(vwSticky, "r")) {
      while(!feof(fp)) {
         fscanf(fp, "%79s", &dummy);
         if((strlen(dummy) != 0) && !feof(fp)) {
            theStickyList[nOfSticky].winClassName = malloc(sizeof(char) * strlen(dummy) + 1);
            strcpy(theStickyList[nOfSticky].winClassName, dummy);
            nOfSticky++;
         }
      }
      fclose(fp);
   }
   return nOfSticky;
}

/*************************************************
 * Writes down the classnames on the sticky windows on file
 */
void saveStickyWindows(int* theNOfWin, windowType* theWinList)
{
   char className[80];
   FILE* fp;
    
   if(!(fp = fopen(vwSticky, "w"))) {
      MessageBox(hWnd, "Error writing sticky file", NULL, MB_ICONWARNING);
   } else {
      int i;
      for(i = 0; i < *theNOfWin; ++i) {
         if (theWinList[i].Sticky) {
            GetClassName(theWinList[i].Handle, className, 79);
            fprintf(fp, "%s\r", &className);
         }
      }
      fclose(fp);
   }
}

/*************************************************
 * Writes down the classnames of the windows currently in list
 * This method is used for the crash recovery routines, should not be 
 * mixed up with saveDesktopConfiguration()
 */
void saveDesktopState(int* theNOfWin, windowType* theWinList)
{
   char className[51];
   FILE* fp;
    
   if(!(fp = fopen(vwState, "wc"))) {
      MessageBox(hWnd, "Error writing state file", NULL, MB_ICONWARNING);
   } else {
      int i;
      for(i = 0; i < *theNOfWin; ++i) {
         GetClassName(theWinList[i].Handle, className, 50);
         fprintf(fp, "%s\r", &className);
      }
      fflush(fp); // Make sure the file is physically written to disk
      fclose(fp);
   }
}

/*************************************************
 * Writes down the current desktop layout to a file
 * 
 */
void saveDesktopConfiguration(int* theNOfWin, windowType* theWinList)
{
   char className[51];
   FILE* fp;
    
   if(!(fp = fopen(vwWindowsState, "w"))) {
      MessageBox(hWnd, "Error writing desktop configuration file", NULL, MB_ICONWARNING);
   } else {
      int i;
      for(i = 0; i < *theNOfWin; ++i) {
         GetClassName(theWinList[i].Handle, className, 50);
         fprintf(fp, "%s %d\n", &className, theWinList[i].Desk);
      }
      fclose(fp);
   }
}

/*************************************************
 * Loads the list with classnames that has an desktop assigned
 */
int loadAssignedList(assignedType* theAssignList) 
{
   char dummy[51];
   FILE* fp;
   int curAssigned = 0;
   
   if(fp = fopen(vwWindowsState, "r")) {
      while(!feof(fp)) {
         fscanf(fp, "%s%i", &dummy, &theAssignList[curAssigned].desktop);
         if((strlen(dummy) != 0) && !feof(fp)) {
            theAssignList[curAssigned].winClassName = malloc(sizeof(char) * strlen(dummy) + 1);
            strcpy(theAssignList[curAssigned].winClassName, dummy);
            curAssigned++;
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
   char dummy[100];
   FILE* fp;
   int curUser = 0;
   
   if(fp = fopen(vwList, "r")) {
      while(!feof(fp)) {
         fgets(dummy, 99, fp);
         // Remove the newline
         if(dummy[strlen(dummy) - 1] == '\n') {
            dummy[strlen(dummy) - 1] = '\0';
         }
         if(curUser < MAXUSER && dummy[0] != ':' && (strlen(dummy) != 0) && !feof(fp)) {
            theUserList[curUser].winNameClass = malloc(sizeof(char) * strlen(dummy) + 1);
            strcpy(theUserList[curUser].winNameClass, dummy);
            theUserList[curUser].isClass = TRUE;
            curUser++;
         } 
      }
      fclose(fp);
   }
   
   return curUser;
}

/*
 * $Log$
 */
