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
// Project Description
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
/*
	Function:	Produce a full screen mode for pSX that removes the black borders that occur in some games.
				Mostly on the top and bottom of the screen, when they occur.
	How to use: Open the program, press <Esc> to toggle the pSX window size. The hotkey is customizable from
				the command line, any Microsoft Virtual Key code can be used for the hotkey.
	Critical assumptions:	The program will look for a window with the name "pSX v1.13". No other windows will
							be considered.
	DLL Project:	The DLL is nessesarey for hiding the mouse cursor in full screen mode. The DLL will install
					a mouse event hook in pSX's thread.
*/
///////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Includes
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
#include "Main.h"

bool KeyPressed = false;
int FS_KEY = VK_ESCAPE;
float TOO_SMALL_RATIO = 0;
std::string WelcomeMessage[3];
/////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// String function
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
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
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
HWND WindowName ()
{
	HWND hWnd = GetConsoleWindow();
	SetWindowText(hWnd, "HotKey Enabled");
	return hWnd;
}
//////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Set window icon
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
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
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
bool EscWindow(HWND hWnd)
{
	return (GetAsyncKeyState(END_KEY) && GetForegroundWindow() == hWnd);
}
bool ResizeKey(HWND hWndTarget, HWND hWnd)
{
	// Accept the fullscreen key when the console is selected to
	return (GetAsyncKeyState(FS_KEY) && (GetForegroundWindow() == hWndTarget || GetForegroundWindow() == hWnd));
}
//////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// HWND to Thread
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
unsigned long HWND2Thread(HWND hWnd) 
{ 
    HANDLE hProcess;
    unsigned long ProcessId, pTID, ThreadID; 
    ThreadID = GetWindowThreadProcessId(hWnd, &ProcessId);

	// Logging
	//printf ("Thread: %08x\n", ThreadID);

    return ThreadID; 
}
//////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Wait for pSX
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void Wait4pSX() 
{ 
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
		"                   Waiting for pSX %s            \n", Dot.c_str());
		ClearScreen(); PrintMessage(WelcomeMessage, 3); WhiteLine();
	}

	// Run again
	Sleep(1000 / 25);
}
void StopWait4pSX() 
{ 
	WelcomeMessage[2] = StringFromFormat(
	"                       STATUS:                   \n"
	"           pSX Fullscreen Mode HotKey Enabled    \n");
	ClearScreen(); PrintMessage(WelcomeMessage, 3); WhiteLine();
}
//////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Main
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
int main(int ArgC, char *ArgV[])
{
	bool Vista = false, FiveFour = false;

	// ----------------------------------------------------------
	// Get command line arguments
	// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	if (ArgC > 1)
	{
		std::string SArgV = ArgV[1];
		for (int i = 0; i < SArgV.length(); i++)
		{
			//printf("%i: %c\n", i, SArgV.substr(i, 1).c_str());
			std::string TmpStr = SArgV.substr(i, 1);
			if (TmpStr == "v") Vista = true;
			if (TmpStr == "f") FiveFour = true;
			if (TmpStr == "1") TOO_SMALL_RATIO = TOO_SMALL_RATIO1;
			if (TmpStr == "2") TOO_SMALL_RATIO = TOO_SMALL_RATIO2;
		}
	}
	if (ArgC > 2)
	{
		std::string SArgV = ArgV[2];
		FS_KEY = Str2Hex(SArgV.substr(0, 2).c_str());
	}
	// -------------------------

	// Setup window
	WindowIcon();
	LetterSpace();
	DisableCursor();
	HWND hWnd = WindowName();
	BlueBackground();
	ClearScreen();

	// ----------------------------------------------------------
	// Print instruction
	// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯

	// Update the HotKey name
	char KeyStr[64] = {0};
	strcpy(KeyStr, VKToString(FS_KEY).c_str());

	WelcomeMessage[0] = StringFromFormat(
	"                        USAGE:                   \n"
	"     Press <%s> To Enter and Exit Fullscreen Mode\n"
	"        Close this Window to Disable the HotKey  \n", KeyStr);
	WelcomeMessage[1] = StringFromFormat(
	"                      SETTINGS:                  \n"
	"    Borders: %s | Screen: %s | V. Resize: %0.3f  \n",
	Vista ? "Vista" : "XP   ", FiveFour ? "5:4" : "16:10", TOO_SMALL_RATIO);
	ClearScreen(); PrintMessage(WelcomeMessage, 2); WhiteLine();

	if (FindWindow(NULL, WINDOW_TITLE) == NULL)
	{
		while (FindWindow(NULL, WINDOW_TITLE) == NULL && !EscWindow(hWnd))
		{
			Wait4pSX();
		}
		if (EscWindow(hWnd)) ExitProcess(0);
	}
	// ---------------------------------------------

	StopWait4pSX();

	HWND hWndTarget = FindWindow(NULL, WINDOW_TITLE);
	//printf ("hWndTarget: 0x%08x\n", hWndTarget);
	//printf ("hProcess: 0x%08x\n", hProcess);

	if(hWndTarget > 0)
	{
		// ----------------------------------------------------------------------
		// Load DLL and install hooks to hide the mouse cursor in full screen mode
		// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
		unsigned long ThreadID = HWND2Thread(hWndTarget);
		HINSTANCE hInstDLL = LoadLibrary((LPCTSTR) "DLL.dll");
		typedef BOOL (CALLBACK *InsHook)(unsigned long);
		InsHook InsCBHook;
		if (hInstDLL > 0)
		{
			InsCBHook = (InsHook)GetProcAddress(hInstDLL, "InstallHook");
			InsCBHook(ThreadID);
			//printf("Installed\n");
		}
		else
		{
			//printf("Failed\n");
		}

		typedef BOOL (CALLBACK *MouseProc)(bool); 
		MouseProc pMouseProc = (MouseProc)GetProcAddress(hInstDLL, "HideMouse");
		// -------------------------------------

		// Check for keys
		while (!EscWindow(hWnd))
		{
			static bool Waiting4pSX = false;

			// If the pSX window goes away
			if ((hWndTarget = FindWindow(NULL, WINDOW_TITLE)) == NULL)
			{
				Waiting4pSX = true;
				Wait4pSX();
			}
			else
			{
				if (Waiting4pSX)
				{
					StopWait4pSX();
					// Install the DLL into pSX again
					ThreadID = HWND2Thread(hWndTarget);
					InsCBHook(ThreadID);
				}
				Waiting4pSX = false;

				// ----------------------------------------------------------------------
				// When entering and ending fast forwarding the wind will jump away from the maximized mode
				// This will kind of fix that by changing it back to full screen.
				// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
				RECT Rc, WinRc;
				GetWindowRect(GetDesktopWindow(), &Rc);
				GetWindowRect(hWndTarget, &WinRc);
				if (FSMode && WinRc.bottom < Rc.bottom)
				{
					ResizeWindow(2, Vista, FiveFour);
				}
				// ----------------------------------------------

				// KeyPressed will disable repeated events
				if (GetAsyncKeyState(FS_KEY) && !KeyPressed)
				{
					KeyPressed = true;
					if (ResizeKey(hWndTarget, hWnd)) ResizeWindow(-1, Vista, FiveFour);
				}
				else if (!GetAsyncKeyState(FS_KEY) && KeyPressed)
				{
					KeyPressed = false;
				}

				Sleep(1000 / 25);
			}

			// Logging
			//ClearScreen();
			//printf("Ht:%i, H:%i, KeyF:%i", hWndTarget, hWnd, GetAsyncKeyState(FS_KEY));
		}

		// ----------------------------------------------------------------------
		// Unload hooks
		// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
		if (hInstDLL > 0)
		{
			typedef BOOL (CALLBACK *UnsHook)(); 
			UnsHook UnstCBHook = (UnsHook)GetProcAddress(hInstDLL, "UnHook"); 
			UnstCBHook();
		}
		// -----------------------------
	}
	else
	{
		// This should not happen
	}

	return 0;
}
//////////////////////////////////////////////