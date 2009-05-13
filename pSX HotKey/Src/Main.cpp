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

	Command lines:	Command lines can be written in two groups of letters, the letters are
						v:		Vista size borders (stretch the window more because it assumes 8 px borders)
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
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
#include "Main.h"

bool KeyPressed = false, ZoomKeyPressed = false;
int FS_KEY = VK_ESCAPE;
float TOO_SMALL_RATIO = 0;
std::string WelcomeMessage[3];
int ConsoleLeft, ConsoleTop;
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
	SetWindowText(hWnd, "HotKey");
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
bool ZoomKey(HWND hWnd)
{
	// Z key
	return (GetAsyncKeyState(0x5A) && GetForegroundWindow() == hWnd);
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
void DoPrintMessage() 
{
	ClearScreen(); PrintMessage(WelcomeMessage, 3); WhiteLine();
}
void Wait4pSX(HWND hWnd) 
{
	SetWindowText(hWnd, "HotKey Disabled ...");

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
		DoPrintMessage();
	}

	// Run again
	Sleep(1000 / 25);
}
void StopWait4pSX(HWND hWnd) 
{ 
	// Update the title
	SetWindowText(hWnd, "HotKey Enabled");

	WelcomeMessage[2] = StringFromFormat(
	"                       STATUS:                   \n"
	"           pSX Fullscreen Mode HotKey Enabled    \n");
	DoPrintMessage();
}
void SettingsMessage(HWND hWnd, bool Vista, bool FiveFour) 
{ 
	WelcomeMessage[1] = StringFromFormat(
	"                      SETTINGS:                  \n"
	"   Borders: %s | Screen: %s | V. Resize: %0.3f\n",
	Vista ? "Vista" : "XP   ", FiveFour ? "5:4" : "16:10", TOO_SMALL_RATIO);
}
//////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Main
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
int main(int ArgC, char *ArgV[])
{
	bool Vista = false, FiveFour = false, KeepAR = false;
	int ZoomMode = 0;

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
			if (TmpStr == "k") KeepAR = true;
			if (TmpStr == "1") {ZoomMode = 1; TOO_SMALL_RATIO = TOO_SMALL_RATIO1;}
			if (TmpStr == "2") {ZoomMode = 2; TOO_SMALL_RATIO = TOO_SMALL_RATIO2;}
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
	Position();
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
	SettingsMessage(hWnd, Vista, FiveFour);
	ClearScreen(); PrintMessage(WelcomeMessage, 2); WhiteLine();
	// ---------------------------------------------

	StopWait4pSX(hWnd);

	HWND hWndTarget = FindWindow(NULL, WINDOW_TITLE);
	//printf ("hWndTarget: 0x%08x\n", hWndTarget);
	//printf ("hProcess: 0x%08x\n", hProcess);

	// ----------------------------------------------------------------------
	// Load DLL to install hooks to hide the mouse cursor in full screen mode
	// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	unsigned long ThreadID = HWND2Thread(hWndTarget);
	HINSTANCE hInstDLL = LoadLibrary((LPCTSTR) "DLL.dll");
	typedef BOOL (CALLBACK *InsHook)(unsigned long);
	//typedef BOOL (CALLBACK *MouseProc)(bool); 
	InsHook InsCBHook;
	if (hInstDLL > 0)
	{
		InsCBHook = (InsHook)GetProcAddress(hInstDLL, "InstallHook");
		//MouseProc pMouseProc = (MouseProc)GetProcAddress(hInstDLL, "HideMouse");
		//printf("Installed\n");
	}
	else
	{
		//printf("Failed\n");
	}
	// -------------------------------------

	// ----------------------------------------------------------------------
	// Main loop
	// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	while (!EscWindow(hWnd))
	{
		// Default to true to install the DLL
		static bool Waiting4pSX = true;

		// If the pSX window goes away
		if ((hWndTarget = FindWindow(NULL, WINDOW_TITLE)) == NULL)
		{
			Waiting4pSX = true;
			Wait4pSX(hWnd);
		}
		else
		{
			if (Waiting4pSX)
			{
				StopWait4pSX(hWnd);
				// Install the DLL into pSX again
				if (hInstDLL > 0)
				{
					ThreadID = HWND2Thread(hWndTarget);
					InsCBHook(ThreadID);
				}
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
				ResizeWindow(2, Vista, FiveFour, KeepAR);
			}
			// ----------------------------------------------

			// KeyPressed will disable repeated events
			if (GetAsyncKeyState(FS_KEY) && !KeyPressed)
			{
				KeyPressed = true;
				if (ResizeKey(hWndTarget, hWnd)) ResizeWindow(-1, Vista, FiveFour, KeepAR);
			}
			else if (!GetAsyncKeyState(FS_KEY) && KeyPressed)
			{
				KeyPressed = false;
			}

			Sleep(1000 / 25);
		}

		// Togle zoom mode
		if (ZoomKey(hWnd) && !ZoomKeyPressed)
		{
			ZoomKeyPressed = true;
			ZoomMode++;
			if (ZoomMode > 2) ZoomMode = 0;
			switch(ZoomMode)
			{
				case 0: TOO_SMALL_RATIO = TOO_SMALL_RATIO0; break;
				case 1: TOO_SMALL_RATIO = TOO_SMALL_RATIO1; break;
				case 2: TOO_SMALL_RATIO = TOO_SMALL_RATIO2; break;
			}
			SettingsMessage(hWnd, Vista, FiveFour);
			DoPrintMessage();
		}
		else if (!ZoomKey(hWnd) && ZoomKeyPressed)
		{
			ZoomKeyPressed = false;
		}

		// Logging
		//ClearScreen();
		//printf("Ht:%i, H:%i, KeyF:%i", hWndTarget, hWnd, GetAsyncKeyState(FS_KEY));
	}
	// -------------------------------------


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

	// End program
	return 0;
}
//////////////////////////////////////////////