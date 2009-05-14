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


//////////////////////////////////////////////////////////////////////////////////////////
// Includes
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
#include "Main.h"
/////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Colors
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void RegularText()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD Color = FOREGROUND_RED | FOREGROUND_GREEN |  FOREGROUND_BLUE | SCR_BACKGROUND;
	SetConsoleTextAttribute(hConsole, Color);
}
void BrightText()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD Color = FOREGROUND_RED | FOREGROUND_GREEN |  FOREGROUND_BLUE | FOREGROUND_INTENSITY | SCR_BACKGROUND;
	SetConsoleTextAttribute(hConsole, Color);
}
void BlueBackground()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD Color = SCR_BACKGROUND;
	SetConsoleTextAttribute(hConsole, Color);
}
void BlackBackground()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD Color = NULL;
	SetConsoleTextAttribute(hConsole, Color);
}
void WhiteLine()
{
	RegularText();
	printf("                %c%c%c----", 250, 250, 250);
	BrightText();
	printf("--------");
	RegularText();
	printf("----%c%c%c      \n", 250, 250, 250);

}
void PrintMessage(std::string Str[3], int Rows)
{
	printf("\n");
	for (int i = 0; i < Rows; i++)
	{
		WhiteLine();
		printf("%s", Str[i].c_str());
	}


}
/////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Console letter space
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void Position()
{
	RECT Rc;
	GetWindowRect(GetConsoleWindow(), &Rc);
	ConsoleLeft = Rc.left;
	ConsoleTop = Rc.top;
}
void LetterSpace()
{
	// Redraw the screen
	#ifndef LOGGING
		BlueBackground();
		ClearScreen();
		StopWait4pSX(GetConsoleWindow());
	#endif

	// Console handle
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 

	// Console size
	int Width = 53;
	int Height = 13;

	// Get console info
	CONSOLE_SCREEN_BUFFER_INFO ConInfo;
	GetConsoleScreenBufferInfo(hConsole, &ConInfo);

	// Change the screen buffer window size
	SMALL_RECT coo = {0,0, Width, Height}; // top, left, right, bottom
	bool SW = SetConsoleWindowInfo(hConsole, TRUE, &coo);

	// Change screen buffer to the screen buffer window size
	COORD Co = {Width + 1, Height + 1};
	bool SB = SetConsoleScreenBufferSize(hConsole, Co);

	// Resize the window too
	MoveWindow(GetConsoleWindow(), ConsoleLeft,ConsoleTop, (Width*8 + 50),(Height*12 + 50), true);

	// Logging
	//printf("[SB:%i SW:%i] X:%i Y:%i W:%i | W:%i H:%i\n", SB, SW, ConInfo.dwSize.X, ConInfo.dwSize.Y,
	//	(ConInfo.srWindow.Right - ConInfo.srWindow.Left), Width, Height);
}
void PixelSpace(int Left, int Top, int Width, int Height)
{
	// Clear the screen
	#ifndef LOGGING
		BlackBackground();
		ClearScreen();
	#endif

	// Console handle
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// Get console info
	CONSOLE_SCREEN_BUFFER_INFO ConInfo;
	GetConsoleScreenBufferInfo(hConsole, &ConInfo);

	// Letter space
	int LWidth = floor((float)(Width / 8));
	int LHeight = floor((float)(Height / 12));	

	// Change the screen buffer window size
	SMALL_RECT coo = {0,0, LWidth, LHeight}; // top, left, right, bottom
	bool SW = SetConsoleWindowInfo(hConsole, TRUE, &coo);

	// Change screen buffer to the screen buffer window size
	COORD Co = {LWidth + 1, LHeight + 1};
	bool SB = SetConsoleScreenBufferSize(hConsole, Co);

	// Resize the window too
	MoveWindow(GetConsoleWindow(), Left,Top, (Width*8 + 50),(Height*12 + 50), true);

	// Logging
	//printf("[SB:%i SW:%i] X:%i Y:%i W:%i | W:%i H:%i\n", SB, SW, ConInfo.dwSize.X, ConInfo.dwSize.Y,
	//	(ConInfo.srWindow.Right - ConInfo.srWindow.Left), Width, Height);
}
/////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Clear console screen
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void ClearScreen() 
{ 
	COORD coordScreen = { 0, 0 }; 
	DWORD cCharsWritten; 
	CONSOLE_SCREEN_BUFFER_INFO csbi; 
	DWORD dwConSize; 

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
    
	GetConsoleScreenBufferInfo(hConsole, &csbi); 
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y; 
	FillConsoleOutputCharacter(hConsole, TEXT(' '), dwConSize, 
		coordScreen, &cCharsWritten); 
	GetConsoleScreenBufferInfo(hConsole, &csbi); 
	FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, 
		coordScreen, &cCharsWritten); 
	SetConsoleCursorPosition(hConsole, coordScreen); 
}
////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Disable Cursor
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void DisableCursor()
{
    CONSOLE_CURSOR_INFO info;
	HANDLE hOutput = GetStdHandle (STD_OUTPUT_HANDLE);

	// Turn the cursor off
	info.bVisible = FALSE;
	info.dwSize = 1;
    if( SetConsoleCursorInfo(hOutput,&info) == 0 )
	{
	
	}
}
////////////////////////////////////////////