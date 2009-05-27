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
// Project Description
// ���������������������
/*
	Function:	Produce a full screen mode for pSX that removes the black borders that occur in some games.
				Mostly on the top and bottom of the screen, when they occur.

	How to use: Open the program, press <Esc> to toggle the pSX window size. The hotkey is customizable from
				the command line, any Microsoft Virtual Key code can be used for the hotkey.

	Critical assumptions:	The program will look for a window with the name "pSX v1.13". No other windows will
							be considered.

	DLL Project:	The DLL is nessesarey for hiding the mouse cursor in full screen mode. The DLL will install
					a mouse event hook in pSX's thread.

	Command lines:	Command lines can be written in two groups of letters, the letters are
						k:		Keep 4:3 aspect ratio in fullscreen mode
						1:		Stretch the picture vertically by a factor of 0.067
						2:		Stretch the picture vertically by a factor of 0.133
						f:		Some adjustments to the window scaling due to the fact that the game picture
								don't follow the pSX window if it means that ther picture will fall outside the
								top of the desktop screen. This only occurs on 5:4 screens (and possible on 4:3
								screens), 16:10 screens don't have this problems. It can't be fixed with any
								aspect ratio correction option, they only seems to have an effect in pSX's full-
								screen mode.
						VK ID:	Microsoft Virtual Key code to use as a fullscreen hotkey. The codes must be
								entered in hexadecimal form. See http://msdn.microsoft.com/en-us/library/ms6455
								40(VS.85).aspx for a list of key codes.
					Command lines example:
						'pSX HotKey.exe vf2 0d': 
								- Vista size borders
								- f ('five-four' mode)
								- 0.133 black borders correction
								- VK_ENTER (0x0d) used as fullscreen hotkey		
*/
///////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Includes
// ������������������
#include "Main.h"

bool KeyPressed = false, ZoomKeyPressed = false, XOffsetKeyPressed = false;
int FS_KEY = VK_ESCAPE;
float TOO_SMALL_RATIO = 0, X_OFFSET = 1;
float Adj = 1.1;
std::string WelcomeMessage[3], WindowTitle = WINDOW_TITLE;
int ConsoleLeft, ConsoleTop, ConsoleIconic = 0;
/////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// String function
// ������������������
std::string StringFromFormat(const char* format, ...)
{
	int writtenCount = -1;
	int newSize = (int)strlen(format) + 4;
	char *buf = 0;
	va_list args;
	while (writtenCount < 0)
	{
		delete [] buf;
		buf = new char[newSize + 1];
	
	    va_start(args, format);
		writtenCount = vsnprintf(buf, newSize, format, args);
		va_end(args);
		if (writtenCount >= (int)newSize) {
			writtenCount = -1;
		}
		newSize *= 2;
	}

	buf[writtenCount] = '\0';
	std::string temp = buf;
	return temp;
}
/////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Set window name
// ������������������
HWND WindowName ()
{
	HWND hWnd = GetConsoleWindow();
	SetWindowText(hWnd, "HotKey");
	return hWnd;
}
//////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Set window icon
// ������������������
void WindowIcon ()
{
	// Handles
	HMODULE hMainMod = GetModuleHandle( 0 );
	HICON hMainIcon = ::LoadIcon( hMainMod, MAKEINTRESOURCE( 1001 ));	   

	// Load kernel 32 library
	HMODULE hMod = LoadLibrary("Kernel32.dll");
	typedef BOOL (CALLBACK *InsHook)(unsigned long, HANDLE); 
	InsHook InsCBHook;

	// Load console icon changing procedure
	typedef DWORD ( __stdcall *SCI )( HICON );
	SCI pfnSetConsoleIcon = reinterpret_cast<SCI>( GetProcAddress( hMod, "SetConsoleIcon" ));

	// Call function to change icon
	pfnSetConsoleIcon( hMainIcon );
	// Free Kernel32.dll
	FreeLibrary( hMod );
}
//////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Check for Key and Windows Combinations
// ������������������
bool EscWindow(HWND hWnd)
{
	return (GetAsyncKeyState(END_KEY) && GetForegroundWindow() == hWnd);
}
bool ResizeKey(HWND hWndTarget, HWND hWnd)
{
	// Accept the fullscreen key when the console is selected to
	return (GetAsyncKeyState(FS_KEY) && (GetForegroundWindow() == hWndTarget || GetForegroundWindow() == hWnd));
}
bool ZoomKey(HWND hWnd)
{
	// Z key
	return (GetAsyncKeyState(0x5A) && GetForegroundWindow() == hWnd);
}

bool XOffsetKey(HWND hWnd)
{
	// X key
	return (GetAsyncKeyState(0x58)); // && GetForegroundWindow() == hWnd);
}

//////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Wait for pSX
// ������������������
void DoPrintMessage() 
{
	ClearScreen(); PrintMessage(WelcomeMessage, 3); WhiteLine();
}
std::string LineSpace(int _Length)
{
	// Determine string space
	std::string Space = "";
	int Length = _Length - ((int)WindowTitle.length() / 2);
	if (Length > 0)
	{
		for (int i = 0; i < Length; ++i)
		{
			Space += " ";
		}
	}
	return Space;
}
void Wait4pSX(HWND hWnd) 
{
	SetWindowText(hWnd, "HotKey Disabled ...");

	// Determine string space
	std::string Space = LineSpace(20);

	static int c, iDot;
	std::string Dot;
	c++;
	if (c % 10 == 0)
	{
		iDot ++;
		if (iDot == 1) Dot = "";
		if (iDot == 2) Dot = ".";
		if (iDot == 3) Dot = "..";
		if (iDot == 4) { Dot = "..."; iDot = 0;}

		WelcomeMessage[2] = StringFromFormat(
		"                       STATUS:                   \n"
		"%sWaiting for '%s' %s\n", Space.c_str(), WindowTitle.c_str(), Dot.c_str());
		DoPrintMessage();
	}
}
void StopWait4pSX(HWND hWnd) 
{ 
	// Update the title
	SetWindowText(hWnd, "HotKey Enabled");

	// Determine string space
	std::string Space = LineSpace(10);

	WelcomeMessage[2] = StringFromFormat(
	"                       STATUS:                   \n"
	"%s'%s' Fullscreen Mode HotKey Enabled\n", Space.c_str(), WindowTitle.c_str());
	DoPrintMessage();
}
void SettingsMessage(HWND hWnd, bool FiveFour, bool KeepAR) 
{ 
	WelcomeMessage[1] = StringFromFormat(
	"                      SETTINGS:                  \n"
	"     Keep 4:3: %s | V. Resize: %0.3f | X: %0.2f\n",
	// pSX only
	//"                    Screen: %s\n"
	KeepAR ? "On " : "Off", TOO_SMALL_RATIO / Adj, X_OFFSET
	//FiveFour ? "5:4" : "16:10"
	);
}
//////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Main
// ������������������
int main(int ArgC, char *ArgV[])
{
	bool FiveFour = false, KeepAR = false, Vista = false, FindOnce = false;
	int ZoomMode = 0, XOffsetMode = 0;

	// ----------------------------------------------------------
	// Get OS version
	// ������������������
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if (osvi.dwMajorVersion == 6) Vista = true;
	// --------------------------------

	// ----------------------------------------------------------
	// Get command line arguments
	// ������������������
	if (ArgC > 1)
	{
		std::string SArgV = ArgV[1];
		if (SArgV != "_")
		{
			for (int i = 0; i < SArgV.length(); i++)
			{
				//printf("%i: %c\n", i, SArgV.substr(i, 1).c_str());
				std::string TmpStr = SArgV.substr(i, 1);
				if (TmpStr == "f") FiveFour = true;
				if (TmpStr == "k") KeepAR = true;
				if (TmpStr == "1") {ZoomMode = 1; TOO_SMALL_RATIO = TOO_SMALL_RATIO1 * Adj;}
				if (TmpStr == "2") {ZoomMode = 2; TOO_SMALL_RATIO = TOO_SMALL_RATIO2 * Adj;}
				if (TmpStr == "3") {ZoomMode = 3; TOO_SMALL_RATIO = TOO_SMALL_RATIO3 * Adj;}
			}
		}
	}
	if (ArgC > 2)
	{
		std::string SArgV = ArgV[2];
		if (SArgV != "_") FS_KEY = Str2Hex(SArgV.substr(0, 2).c_str());
	}
	if (ArgC > 3)
	{
		WindowTitle = ArgV[3];
		// Change _ to spaces
		while (WindowTitle.find("_") != std::string::npos)
		{
			int it = WindowTitle.find("_");
			WindowTitle.replace(it, 1, " ");
		}
	}
	// -------------------------

	// Setup window
	//#ifndef LOGGING
		WindowIcon();
		Position();
		LetterSpace();
		DisableCursor();
		BlueBackground();
		ClearScreen();
	//#endif
	HWND hWnd = WindowName();

	// ----------------------------------------------------------
	// Print instruction
	// ������������������
	// Update the HotKey name
	char KeyStr[64] = {0};
	strcpy(KeyStr, VKToString(FS_KEY).c_str());

	WelcomeMessage[0] = StringFromFormat(
	"                        USAGE:                   \n"
	"     Press <%s> to Enter and Exit Fullscreen Mode\n"
	"    Press <Z> (in this Window) to Toggle Zoom Mode\n"
	"        Close this Window to Disable the HotKey  \n", KeyStr);
	SettingsMessage(hWnd, FiveFour, KeepAR);
	ClearScreen(); PrintMessage(WelcomeMessage, 2); WhiteLine();
	// ---------------------------------------------

	StopWait4pSX(hWnd);

	HWND hWndTarget = FindWindow(NULL, WindowTitle.c_str());
	//printf ("hWndTarget: 0x%08x\n", hWndTarget);
	//printf ("hProcess: 0x%08x\n", hProcess);

	// ----------------------------------------------------------------------
	// Main loop
	// ������������������
	while (!EscWindow(hWnd))
	{
		// Default to true to install the DLL
		static bool Waiting4pSX = true;

		// If the game window goes away
		if (FindWindow(NULL, WindowTitle.c_str()) == NULL && !IsWindow(hWndTarget))
		{
			Waiting4pSX = true;
			#ifndef LOGGING
				Wait4pSX(hWnd);
			#endif

			// As a safety precaution in case pSX were to crash we show the taskbar here
			ShowTaskbar(true);
		}
		else
		{
			if (FindWindow(NULL, WindowTitle.c_str())) hWndTarget = FindWindow(NULL, WindowTitle.c_str());

			if(Waiting4pSX)	StopWait4pSX(hWnd);

			// Only update the message once
			Waiting4pSX = false;

			// ----------------------------------------------------------------------
			// When entering and ending fast forwarding the wind will jump away from the maximized mode.
			// This will kind of fix that by changing it back to full screen. But it can also be fixed
			// by disabling VSync for the windowed mode.
			// ������������������
			/*
			RECT Rc, WinRc;
			GetWindowRect(GetDesktopWindow(), &Rc);
			GetWindowRect(hWndTarget, &WinRc);
			if (FSMode && WinRc.bottom < Rc.bottom && !IsIconic(hWnd))
			{
				ResizeWindow(2, Vista, FiveFour, KeepAR);
			}
			*/
			// ----------------------------------------------

			// ----------------------------------------------------------------------
			// Keep the game window on top in fullscreen mode
			// ������������������
			if (FSMode && GetForegroundWindow() != hWndTarget) SetForegroundWindow(hWndTarget);
			// ----------------------------------------------

			// KeyPressed will disable repeated events
			if (GetAsyncKeyState(FS_KEY) && !KeyPressed)
			{
				KeyPressed = true;
				if (ResizeKey(hWndTarget, hWnd)) ResizeWindow(hWndTarget, -1, Vista, FiveFour, KeepAR);
			}
			else if (!GetAsyncKeyState(FS_KEY) && KeyPressed)
			{
				KeyPressed = false;
			}

			#ifdef LOGGING
				//printf("Updated HWND\n");
			#endif
		}

		// ----------------------------------------------------------------------
		// Togle zoom mode
		// ������������������
		if (ZoomKey(hWnd) && !ZoomKeyPressed)
		{
			ZoomKeyPressed = true;
			ZoomMode++;
			if (ZoomMode > 3) ZoomMode = 0;
			switch(ZoomMode)
			{
				case 0: TOO_SMALL_RATIO = TOO_SMALL_RATIO0 * Adj; break;
				case 1: TOO_SMALL_RATIO = TOO_SMALL_RATIO1 * Adj; break;
				case 2: TOO_SMALL_RATIO = TOO_SMALL_RATIO2 * Adj; break;
				case 3: TOO_SMALL_RATIO = TOO_SMALL_RATIO3 * Adj; break;
			}
			SettingsMessage(hWnd, FiveFour, KeepAR);
			DoPrintMessage();
		}
		else if (!ZoomKey(hWnd) && ZoomKeyPressed)
		{
			ZoomKeyPressed = false;
		}
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// Togle x-offset
		// ������������������
		if (XOffsetKey(hWnd) && !XOffsetKeyPressed)
		{
			XOffsetKeyPressed = true;
			XOffsetMode++;
			if (XOffsetMode > 3) XOffsetMode = 0;
			switch(XOffsetMode)
			{
				case 0: X_OFFSET = 1; break;
				case 1: X_OFFSET = 1.03; break;
				case 2: X_OFFSET = 1.035; break;
				case 3: X_OFFSET = 1.04; break;
			}
			SettingsMessage(hWnd, FiveFour, KeepAR);
			DoPrintMessage();
			ResizeWindow(hWndTarget, 2, Vista, FiveFour, KeepAR);
		}
		else if (!XOffsetKey(hWnd) && XOffsetKeyPressed)
		{
			XOffsetKeyPressed = false;
		}
		// ----------------------------------------------------------------------


		// Update rate
		Sleep(1000 / 25);

		// Logging
		/*
		#ifdef LOGGING
			ClearScreen();
			printf("Ht:%i, H:%i, FindWindow:%i IsWindow:%i Other:%i\n",
				hWnd, hWndTarget, FindWindow(NULL, WINDOW_TITLE), IsWindow(hWndTarget),
				FindWindow(NULL, "Sega Rally Championship"));
		#endif
		*/
	}
	// -------------------------------------

	// End program
	return 0;
}
//////////////////////////////////////////////