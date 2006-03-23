#ifndef _RESOURCE_H_
#define _RESOURCE_H

#define	IDC_STATIC	-1
#define	IDD_PROPPAGE_KEYS	102
#define	IDS_APP_TITLE	103
#define	IDC_HOT1	104
#define	IDC_HOT1W	105
#define	IDD_PROPPAGE_ABOUT	106
#define	IDI_VIRTWIN	107
#define	IDI_LARGEAPP	108
#define	IDI_SMALL_DIS	109
#define	IDI_SMALL_NW	110
#define	IDI_SMALL_NE	111
#define	IDI_SMALL_SW	112
#define	IDI_SMALL_SE	113
#define	IDC_HOT2	114
#define	IDC_HOT7	115
#define	IDC_HOT4	116
#define	IDC_HOT3	117
#define	IDC_HOT5	118
#define	IDC_HOT6	119
#define	IDC_HOT8	120
#define	IDC_HOT9	121
#define	IDC_HOT2W	122
#define	IDC_HOT3W	123
#define	IDC_HOT4W	124
#define	IDC_HOT5W	125
#define	IDC_HOT6W	126
#define	IDC_HOT7W	127
#define	IDC_HOT8W	128
#define	IDC_HOT9W	129
#define	IDR_MENU1	130
#define	IDI_ICON0	159
#define	IDI_ICON1	160
#define	IDI_ICON2	161
#define	IDI_ICON3	162
#define	IDI_ICON4	163
#define	IDI_ICON5	164
#define	IDI_ICON6	165
#define	IDI_ICON7	166
#define	IDI_ICON8	167
#define	IDI_ICON9	168
#define	IDD_PROPPAGE_MOUSE	169
#define	IDD_PROPPAGE_MISC	170
#define	IDD_PROPPAGE_MODULES	171
#define IDD_PROPPAGE_EXPERT     172
#define	IDC_PRESZORDER	173
#define	IDC_RECOVERY	174
#define	IDC_DESKCYCLE	175
#define	IDC_MOUSWARP	1001
#define	IDC_KEYS	1002
#define	IDC_TIME	1006
#define	IDC_DESKY	1007
#define	IDC_DESKX	1008
#define	IDC_FOCUS	1013
#define	IDC_SLIDER	1014
#define	IDC_ALT 	1015
#define	IDC_SHIFT	1016
#define	IDC_CTRL	1017
#define	IDC_WIN 	1018
#define	IDC_JUMP	1021
#define	IDC_LASTACTIVE	1023
#define	IDC_SHIFTHOT	1024
#define	IDC_CTRLHOT	1025
#define	IDC_WINHOT	1026
#define	IDC_ALTHOT	1027
#define	IDC_MINIMIZED	1028
#define	IDC_TASKBAR	1030
#define	IDC_SLIDERX	1040
#define	IDC_SLIDERY	1041
#define	IDC_HOTKEYS	1043
#define	IDC_KEYCONTROL	1044
#define	IDC_MODLIST	1051
#define	IDC_MODCONFIG	1052
#define	IDC_MODRELOAD	1053
#define	IDC_STICKYSAVE	1055
#define	IDC_REFRESH	1056
#define	IDC_MOUSEWRAP	1059
#define	IDC_HOTSTICKY	1060
#define	IDC_HTTP	1061
#define	IDC_MAILTO	1062
#define	IDC_INVERTY	1063
#define IDC_MENUSTICKY  1064
#define IDC_MENUACCESS  1065
#define IDC_MENUASSIGN  1066
#define IDC_MODDISABLE  1067
#define IDC_LICENSE     1068
#define IDC_USEASSIGN   1069
#define IDC_SAVENOW     1070
#define IDC_FIRSTONLY   1071
#define IDC_SAVEEXITSTATE 1072
#define IDC_SAVESTICKY 1073
#define IDC_CYCLINGKEYS 1074
#define IDC_HOTCYCLEUP 1075
#define IDC_HOTCYCLEDOWN 1076
#define IDC_HOTMENU 1077
#define IDC_HOTMENUW 1078
#define IDC_HOTMENUEN 1079
#define IDC_DISPLAYICON 1080
#define IDC_TASKBARDETECT 1081
#define IDC_TRICKYSUPPORT 1082
#define IDC_XPSTYLETASKBAR 1083
#define IDC_PERMSTICKY  1084
#define IDC_HOTSTICKYEN 1085
#define IDC_HOTSTICKYW  1086
#define IDC_HOTCYCLEUPW 1087
#define IDC_HOTCYCLEDOWNW 1088
#define IDC_POPUPRHWIN    1089
#define IDC_HWINPOPUP     1090
#define IDC_ASSIGNWINNOW  1091

#define	ID_SETUP	32771
#define	ID_EXIT	        32772
#define	ID_FORWARD      32773
#define	ID_BACKWARD     32774
#define	ID_GATHER	32775
#define ID_DISABLE      32776
#define	ID_HELP	        32777

#endif

/*
 * $Log$
 * Revision 1.10  2004/02/28 18:54:01  jopi
 * SF904069 Added possibility to choose if sticky should be permanent for all instances of the same classname.
 *
 * Revision 1.9  2003/06/24 19:47:04  jopi
 * SF693876 Fixed option to handle XP skinned style taskbars
 *
 * Revision 1.8  2003/03/10 20:48:20  jopi
 * Changed so that doubleclick will bring up setup and added a disabled menu item instead.
 *
 * Revision 1.7  2002/12/23 15:42:28  jopi
 * Added config options to disable taskbar detection and the alternative hiding technique.
 *
 * Revision 1.6  2002/12/23 14:16:47  jopi
 * Added a new setup tab, "expert" and moved some settings from misc.
 *
 * Revision 1.5  2002/06/01 21:15:23  Johan Piculell
 * Multiple fixes by Christian Storm.
 *
 * Revision 1.4  2001/11/12 21:39:14  Johan Piculell
 * Added functionality for disabling the systray icon
 *
 * Revision 1.3  2001/01/28 16:10:00  Administrator
 * Added #ifndef/#define
 *
 * Revision 1.2  2000/08/18 23:43:07  Administrator
 *  Minor modifications by Matti Jagula <matti@proekspert.ee> List of modifications follows: Added window title sorting in popup menus (Assign, Direct, Sticky) Added some controls to Setup Misc tab and support for calling the popup menus from keyboard.
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  Administrator
 * Added first time
 *
 */
