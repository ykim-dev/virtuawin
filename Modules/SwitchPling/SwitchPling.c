/*
 * SwitchPling module for VirtuaWin 
 *
 * Created by Johan Piculell 1999
 * VirtuaWin@iname.com
 * Feel free to use in any way you like(well, almost)
 */
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <mmsystem.h>
#include "../../Messages.h"

HINSTANCE hInst;   // Instance handle
HWND hwndMain;	   // Main window handle
HWND vwHandle;     // Handle to VirtuaWin

/* Main message handler */
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

/* Initializes the window */ 
static BOOL InitApplication(void)
{
  WNDCLASS wc;

  memset(&wc, 0, sizeof(WNDCLASS));
  wc.style = 0;
  wc.lpfnWndProc = (WNDPROC)MainWndProc;
  wc.hInstance = hInst;
  wc.lpszClassName = "SwitchPling.exe";
  
  if (!RegisterClass(&wc))
    return 0;
  
  return 1;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
  case MOD_INIT: // This must be taken care of in order to get the handle to VirtuaWin. 
    // The handle to VirtuaWin comes in the wParam 
    vwHandle = (HWND) wParam; // Should be some error handling here if NULL 
    break;
  case MOD_QUIT: // This must be handeled, otherwise VirtuaWin can't shut down the module 
    PostQuitMessage(0);
    break;
  case MOD_SETUP: 
    MessageBox(vwHandle, "Created by Johan Piculell 2000 \n\r\n\rChange the the wav files in the module directory to get new switch sounds.", "SwitchPling 1.0", 0);
    break;
  case MOD_CHANGEDESK:
    
    switch (wParam)
      {
      case MOD_STEPLEFT:
        PlaySound("leftSound.wav", NULL, SND_FILENAME | SND_ASYNC);
        break;
      case MOD_STEPRIGHT:
        PlaySound("rightSound.wav", NULL, SND_FILENAME | SND_ASYNC);
        break;
      case MOD_STEPUP:
        PlaySound("upSound.wav", NULL, SND_FILENAME | SND_ASYNC);
        break;
      case MOD_STEPDOWN:
        PlaySound("downSound.wav", NULL, SND_FILENAME | SND_ASYNC);
        break;
      }
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
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
  MSG msg;
  hInst = hInstance;
  if (!InitApplication())
    return 0;
  
  // In this example, the window is never shown
  if ((hwndMain = CreateWindow("SwitchPling.exe", 
                               "SwitchPling", 
                               WS_POPUP,
                               CW_USEDEFAULT, 
                               0, 
                               CW_USEDEFAULT, 
                               0,
                               NULL,
                               NULL,
                               hInst,
                               NULL)) == (HWND)0)
    return 0;
  
  // main messge loop
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return msg.wParam;
}

