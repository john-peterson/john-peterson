//////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright Information
// ���������������������
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
// ���������������������
#include "Main.h"

HHOOK hkb = NULL;
HINSTANCE hIns = NULL;
HWND hWnd = NULL;
HANDLE hProcess = NULL;
FILE *f1;
bool FSMode = false;
//////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// Show Taskbar
// ���������������������
void ShowTaskbar(bool Show)
{
	HWND hWndTask = FindWindow("Shell_traywnd", "");
	HWND hWndTaskButton = FindWindow("Button", "Start");

	ShowWindow(hWndTask, Show);
	if (hWndTaskButton) ShowWindow(hWndTaskButton, Show);
}
//////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// Resize Window
// ���������������������
void ResizeWindow(HWND hWnd, int _Mode, bool Vista, bool FiveFour, bool KeepAR)
{
	// Get HWND
	HWND hWndTask = FindWindow("Shell_traywnd", "");

	// Get current resolution
	RECT Rc, WinRc, RcTask;
	GetWindowRect(hWndTask, &RcTask);
	GetWindowRect(GetDesktopWindow(), &Rc);
	GetWindowRect(hWnd, &WinRc);
	int Width, Height, Left, Top, PWidth, PHeight, BorderPixelSize,
		TaskHeight = RcTask.bottom - RcTask.top, ClientHeight = Rc.bottom - TaskHeight;
	static int Mode = 0;
	static bool RemoveBorders = true, MenuBar = false;

	// Full screen mode
	SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
	ShowTaskbar(true);

	// ----------------------------------------------------------------------
	// Select beetween three screen modes
		// 0. Small window
		// 1. Large window
		// 2. Fullscreen
	// ������������������
	if (_Mode == -1)
	{
		// Shrink the window if we zre in FSMode
		if (Mode == 0)
			Mode = 1;
		else if (Mode == 1)
			Mode = 2;
		else
			Mode = 1;
	}
	// -----------------------------------

	// Restore the console window status
	if (Mode != 2) LetterSpace();

	// ----------------------------------------------------------------------
	// Hide the mouse cursor in fullscreen mode
	// ������������������
	if (Mode != 2)
	{
		// Use AttachThreadInput so that ShowCursor() works
		AttachThreadInput(GetCurrentThreadId(), GetWindowThreadProcessId(hWnd,NULL), TRUE);
		while (ShowCursor(true) < 0) {}
		AttachThreadInput(GetWindowThreadProcessId(hWnd,NULL), GetCurrentThreadId(), FALSE);
	}
	else
	{
		// Use AttachThreadInput so that ShowCursor() works
		AttachThreadInput(GetCurrentThreadId(), GetWindowThreadProcessId(hWnd,NULL), TRUE);
		while (ShowCursor(false) >= 0) {}
		AttachThreadInput(GetWindowThreadProcessId(hWnd,NULL), GetCurrentThreadId(), FALSE);
	}
	// -----------------------------------

	// ----------------------------------------------------------------------
	// Resize window
	// ������������������
	if (Mode == 0)
	{
		// Maximized window
		FSMode = false;
		Mode = 0;
		//Width = Rc.right;
		//Height = Rc.bottom - TaskHeight;
		Width = 640 + 8;
		Height = 480 + 19 + 30;
		//Left = 0;
		//Top = 0;
		Left = (Rc.right - Width) / 2;
		Top = (Rc.bottom - Height) / 2;
	}
	else if (Mode == 1)
	{
		// Produce width and height
		FSMode = false;
		Mode = 1;
		//Width = ceil ((float)Rc.right * 0.5);
		//Height = ceil ((float)Rc.bottom * 0.5);
		Width = 640 + 8;
		Height = 480 + 19 + 30;
		Left = (Rc.right - Width) / 2;
		Top = (Rc.bottom - Height) / 2;
	}
	else
	{
		// Full screen mode
		FSMode = true;

		// Make sure pSX is not minimized
		while(IsIconic(hWnd))
		{
			ShowWindow(hWnd, SW_RESTORE);
			Sleep(10);
		}

		// Produce width
		Width = Rc.right;
		Left = 0;

		// Produce height
		// First remove the menu bar
		int MenuBarHeight;
		if (MenuBar)
		{
			if (Vista) MenuBarHeight = 20;
				else MenuBarHeight = 19;  // XP
		}
		else
		{
			MenuBarHeight = 0;
		}
		Height = Rc.bottom + MenuBarHeight;
		Top = -MenuBarHeight;

		// Then remove the black borders
		BorderPixelSize = Rc.bottom * TOO_SMALL_RATIO;		
		Height = Height + BorderPixelSize;
		Top = Top - BorderPixelSize / 2.0;

		// The console should still be fullscreen
		int ConsoleLeft = Left, ConsoleTop = Vista ? -20 : -19, ConsoleWidth = Width, ConsoleHeight = Height;
		int ConsoleBorderWidth = ConsoleBorderWidth = 13;  // Enough for both Vista and XP

		// Keep 4:3 aspect ratio
		int NewWidth, NewHeight;
		if (KeepAR)
		{
			// check that we are not dividing by zero
			if (Rc.bottom > 0)
			{
				// Check if the screen is too wide or too high
				if (Rc.right / Rc.bottom > 4.0/3.0)
				{
					NewWidth = Rc.bottom * (4.0/3.0);
					int LeftRightSpace = Width - NewWidth;
					Left = Left + LeftRightSpace / 2;
					Width = NewWidth;
				}
				else
				{
					NewHeight = Rc.right / (4.0/3.0);
					int TopBottomSpace = Height - NewHeight;
					Top = Top + TopBottomSpace / 2;
					Height = NewHeight;
				}
			}
		}

		// The picture refused to go to full screen on my 5:4 screen because it would not follow the pSX window
		// upwards into negetive territory. With these adjustments the bottom border will be removed, but the
		// border will be left as it is.
		if (FiveFour)
		{
			BorderPixelSize = Rc.bottom * (TOO_SMALL_RATIO / 2);
			Height = Rc.bottom + MenuBarHeight;
			Height = Height + BorderPixelSize;
			Top = -MenuBarHeight;
		}

		// Remove window borders
		if (RemoveBorders) SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE);
		// Hide the taskbar
		ShowTaskbar(false);

		// Adjustment in case window borders are not removed
		if (!RemoveBorders)
		{
			if (!Vista)
			{
				Left = Left - 4;
				Width = Width + 8;
				Top = Top - 30;
				Height = Height + 30 + 4;
			}
		}

		// XOffset
		//Left = Left - Width * (1 - X_OFFSET);

		//Left = Left - 100;
		//Width = Width - 40; 
		//Width = Width - 200 * (1 - X_OFFSET);
		Width = Width * X_OFFSET;
		Height = Height * X_OFFSET;
			


		// ----------------------------------------------------------------------
		// Make sure the black window is behind pSX but above all other windows
		// ������������������
		if (KeepAR)
		{
			// Save the current console window status
			Iconic();

			while(IsIconic(GetConsoleWindow()))
			{
				ShowWindowNoAnimate(GetConsoleWindow(), SW_RESTORE);
				Sleep(10);
				#ifdef LOGGING
					printf("IsIconic()\n");
				#endif
			}

			// Save the current console window status
			Position();
			
			// Cover the screen with the black window
			PixelSpace(ConsoleLeft - ConsoleBorderWidth, ConsoleTop - ConsoleBorderWidth, ConsoleWidth, ConsoleHeight);

			// Use BringWindowToTop() instead of SetForegroundWindow() to avoid flashing the taskbar icon in Vista
			while(GetForegroundWindow() != GetConsoleWindow())
			{		
				// Use AttachThreadInput so that BringWindowToTop() works
				AttachThreadInput(GetWindowThreadProcessId(::GetForegroundWindow(),NULL), GetCurrentThreadId(), TRUE);
				BringWindowToTop(GetConsoleWindow());
				AttachThreadInput(GetWindowThreadProcessId(::GetForegroundWindow(),NULL), GetCurrentThreadId(), FALSE);
				Sleep(10);
				#ifdef LOGGING
					printf("ResizeWindow(): %i %i %i\n", GetTopWindow(NULL), GetConsoleWindow(), hWnd);
				#endif
			}
		}
		// ----------------------------------------------------------
		
		// Debug
		#ifdef LOGGING
		printf("ResizeWindow(): W:%i H:%i L:%i T:%i | XOffs: %0.2f BorderPixelSize:%i\n", Width,Height, Left,Top, X_OFFSET, BorderPixelSize);
		#endif
	}
	// ----------------------------------------------------------

	// Set window size
	SetWindowPos(hWnd, HWND_TOP, Left,Top, Width,Height, SWP_NOSENDCHANGING | SWP_FRAMECHANGED);
	// Show the window	
	SetForegroundWindow(hWnd);

	// Debug
	#ifdef LOGGING
			printf("--------------------------------------------------\n");
	#endif
}
////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Str to Hex
// ���������������������
u32 Str2Hex(const char* _szValue)
{
	u32 value = 0;
	size_t finish = strlen(_szValue);

	if (finish > 8)
		finish = 8;  // Max 32-bit values are supported.

	for (size_t count = 0; count < finish; count++)
	{
		value <<= 4;
		switch (_szValue[count])
		{
		    case '0': break;
		    case '1': value += 1; break;
		    case '2': value += 2; break;
		    case '3': value += 3; break;
		    case '4': value += 4; break;
		    case '5': value += 5; break;
		    case '6': value += 6; break;
		    case '7': value += 7; break;
		    case '8': value += 8; break;
		    case '9': value += 9; break;
		    case 'A':
		    case 'a': value += 10; break;
		    case 'B':
		    case 'b': value += 11; break;
		    case 'C':
		    case 'c': value += 12; break;
		    case 'D':
		    case 'd': value += 13; break;
		    case 'E':
		    case 'e': value += 14; break;
		    case 'F':
		    case 'f': value += 15; break;
		    default:
			    return false;
			    break;
		}
	}
	return value;
}
/////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Windows Virtual Key Codes Names
// ���������������������
std::string VKToString(int KeyCode)
{
	// Default value
	char KeyStr[64] = {0};
	GetKeyNameText(MapVirtualKey(KeyCode, MAPVK_VK_TO_VSC) << 16, KeyStr, 64);
	std::string KeyString = KeyStr;

	switch(KeyCode)
	{
		// Give it some help with a few keys
		case VK_END: return "END";
		case VK_INSERT: return "INS";
		case VK_DELETE: return "DEL";
		case VK_PRIOR: return "PGUP";
		case VK_NEXT: return "PGDN";

		case VK_UP: return "UP";
		case VK_DOWN: return "DOWN";
		case VK_LEFT: return "LEFT";
		case VK_RIGHT: return "RIGHT";

		case VK_LSHIFT: return "LEFT SHIFT";
		case VK_LCONTROL: return "LEFT CTRL";
		case VK_RCONTROL: return "RIGHT CTRL";
		case VK_LMENU: return "LEFT ALT";

		default: return KeyString;
	}
}
/////////////////////////////////////////////////////////////////////