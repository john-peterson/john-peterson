//////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright Information
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
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
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
#include "Main.h"

HHOOK hkb = NULL;
HINSTANCE hIns = NULL;
HWND hWnd = NULL;
HANDLE hProcess = NULL;
FILE *f1;
bool FSMode = false;
//////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// Resize Window
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
void ResizeWindow(int Mode, bool Vista, bool FiveFour, bool KeepAR)
{
	// Get HWND
	HWND hWnd = FindWindow(NULL, WINDOW_TITLE);
	HWND hWndTask = FindWindow("Shell_traywnd", "");
	HWND hWndTaskButton = FindWindow("Button", "Start");

	// Get current resolution
	RECT Rc, WinRc, RcTask;
	GetWindowRect(hWndTask, &RcTask);
	GetWindowRect(GetDesktopWindow(), &Rc);
	GetWindowRect(hWnd, &WinRc);
	int Width, Height, Left, Top, PWidth, PHeight, BorderPixelSize,
		TaskHeight = RcTask.bottom - RcTask.top, ClientHeight = Rc.bottom - TaskHeight;

	// Full screen mode
	FSMode = false;
	SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
	LetterSpace();
	ShowWindow(hWndTask, 1);
	if (hWndTaskButton) ShowWindow(hWndTaskButton, 1);

	// ----------------------------------------------------------------------
	// Select beetween three screen modes
		// 0. Small window
		// 1. Large window
		// 2. Fullscreen
	// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
	if (Mode == -1)
	{
		// Shrink the window if it's at least one pixel bigger than the dektop height
		if (WinRc.bottom - WinRc.top > Rc.bottom)
			Mode = 0;
		else
			Mode = 2;
	}
	// -----------------------------------

	// ----------------------------------------------------------------------
	// Resize window
	// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
	if (Mode == 1)
	{
		// Maximized window
		Width = Rc.right;
		Height = Rc.bottom - TaskHeight;
		Left = 0;
		Top = 0;
	}
	else if (Mode == 0)
	{
		// Produce width and height
		Width = ceil ((float)Rc.right * 0.5);
		Height = ceil ((float)Rc.bottom * 0.5);
		Left = (Rc.right - Width) / 2;
		Top = (Rc.bottom - Height) / 2;
	}
	else
	{
		// Full screen mode
		FSMode = true;

		// Produce width
		int BorderWidth = 0;
		Width = Rc.right + BorderWidth * 2;
		Left = -BorderWidth;

		// Produce height
		// First remove the menu bar
		int MenuBarHeight;
		if (Vista) MenuBarHeight = 20;
			else MenuBarHeight = 19;  // XP
		Height = Rc.bottom + MenuBarHeight + BorderWidth;
		Top = -MenuBarHeight;

		// Then remove the black borders
		BorderPixelSize = Rc.bottom * TOO_SMALL_RATIO;		
		Height = Height + BorderPixelSize;
		Top = Top - BorderPixelSize / 2;

		// The console should still be fullscreen
		int ConsoleLeft = Left, ConsoleWidth = Width;
		int ConsoleBorderWidth = 0;
		if (Vista) ConsoleBorderWidth = 8 + 2;
			else ConsoleBorderWidth = 4 + 9;  // XP

		// Keep 4:3 aspect ratio
		int NewWidth;
		if (KeepAR)
		{
			NewWidth = Rc.bottom * (4.0/3.0);
			int LeftRightSpace = Width - NewWidth;
			Left = Left + LeftRightSpace / 2;
			Width = NewWidth;
		}

		// The picture refused to go to full screen on my 5:4 screen because it would not follow the pSX window
		// upwards into negetive territory. With these adjustments the bottom border will be removed, but the
		// border will be left as it is.
		if (FiveFour)
		{
			BorderPixelSize = Rc.bottom * (TOO_SMALL_RATIO / 2);
			Height = Rc.bottom + MenuBarHeight + BorderWidth;
			Height = Height + BorderPixelSize;
			Top = -MenuBarHeight;
		}

		SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE);
		PixelSpace(ConsoleLeft - ConsoleBorderWidth, Top - ConsoleBorderWidth, ConsoleWidth, Height);
		ShowWindow(hWndTask, 0);
		if (hWndTaskButton) ShowWindow(hWndTaskButton, 0);
		
		// Debug
		//printf("Width:%i[%i] Height:%i Top:%i | BorderPixelSize:%i\n", Width, NewWidth, Height, Top, BorderPixelSize);
	}
	// ----------------------------------------------------------

	// Set window size
	SetWindowPos(hWnd, HWND_TOP, Left,Top, Width,Height, SWP_NOSENDCHANGING | SWP_FRAMECHANGED);
	// Show the window
	//SetForegroundWindow(GetConsoleWindow());
	SetForegroundWindow(hWnd);
}
////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Str to Hex
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
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
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
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