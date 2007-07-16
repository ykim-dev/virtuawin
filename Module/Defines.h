//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  ConfigParameters.h - Constant definitions used.
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

#ifndef _DEFINES_H_
#define _DEFINES_H

// application name and version defines
#define vwVIRTUAWIN_NAME         _T("VirtuaWin")
#define vwVIRTUAWIN_EMAIL        _T("VirtuaWin@home.se")
#define vwVIRTUAWIN_NAME_VERSION _T("VirtuaWin v3.1 test 2")

// Various defines used on several places 
#define vwWINDOW_MAX     160       // max number of windows to handle
#define vwDESKTOP_MAX     20       // max number of desktops
#define vwDESKTOP_SIZE    (vwDESKTOP_MAX + 2)
#define vwCLASSNAME_MAX   64       // class name buffer size
#define vwWINDOWNAME_MAX 128       // window name buffer size
#define vwMODULENAME_MAX  79       // Maximum length of a module name (buffer needs to be n+1 long)

#define MAXMODULES        10       // max number of modules to handle

// Internal windows messages
#define VW_SYSTRAY        (WM_USER + 1)  // Sent to us by the systray
#define VW_MOUSEWARP      (WM_USER + 9)  // Mouse thread message

#endif
