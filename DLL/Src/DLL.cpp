//////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright Information
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/
///////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
#include <iostream>
#include <windows.h>
#include <math.h>

#include <tchar.h>
#include <strsafe.h>

#define WINDOW_TITLE "pSX v1.13"

HHOOK hkb = NULL;
HINSTANCE hIns = NULL;
HWND hWnd = NULL;
HANDLE hProcess = NULL;
FILE *f1;
bool bShowCursor = true;
//////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Entry point
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hIns = hinstDLL;
	// Logging
	//printf("InitInstance: %i\n", hIns);

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		//printf("Dll.dll Loaded\n");
		break;
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_DETACH:
		//printf("Dll.dll Unloaded\n");
		break;
	}

	return TRUE;
}
////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// Mouse Callback
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) 
{ 
	// Logging
	//Console::Print("MouseProc\n");

	#define WINDOW_TITLE "pSX v1.13"
	HWND hWnd = FindWindow(NULL, WINDOW_TITLE);
	RECT Rc, WinRc;
	GetWindowRect(GetDesktopWindow(), &Rc);
	GetWindowRect(hWnd, &WinRc);
	if (Rc.bottom - Rc.top < WinRc.bottom - WinRc.top)
		bShowCursor = false;
	else
		bShowCursor = true;

	// ----------------------------------------------------------------------
	// Show cursor: Use SetCursor() too in case ShowCursor() doesn't work
	// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	ShowCursor(bShowCursor);	
	HCURSOR hCursor = NULL, hCursorBlank = NULL;
	hCursor = LoadCursor( NULL, IDC_ARROW );
	if (!bShowCursor) SetCursor(hCursorBlank);
		else SetCursor(hCursor);
	// ----------------------------------------------------
 
    return CallNextHookEx(hkb, nCode, wParam, lParam); 
}
////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// Hide Cursor Exported Function
BOOL __declspec(dllexport)__stdcall HideMouse(bool _bShowCursor)
{    	
	bShowCursor = _bShowCursor;
	return true;
}
////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// Install and uninstall hooks
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
BOOL __declspec(dllexport)__stdcall InstallHook(unsigned long ThreadID)
{
	// Logging
	//printf("ThreadID: 0x%08x\n", ThreadID);

	if (ThreadID > 0)
	{
		hkb = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)MouseProc, hIns, ThreadID);
	}

	return TRUE;
}

BOOL __declspec(dllexport)__stdcall UnHook()
{    	
	BOOL unhooked = UnhookWindowsHookEx(hkb);
	//MessageBox(0,"exit","sasa",MB_OK);

	return unhooked;
} 
//////////////////////////////////