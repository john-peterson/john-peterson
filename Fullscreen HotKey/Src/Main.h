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
#include <iostream>
#include <string>
#include <windows.h>
#include <math.h>
//////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Declarations
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
typedef unsigned __int32 u32;

// Uncomment this to enable logging
//#define LOGGING
// Detected window title
#define WINDOW_TITLE "pSX v1.13"
// Key for closing the console, when it's selected [Currently Disabled]
#define END_KEY -1 // VK_RETURN
// Fullscreen key
extern int FS_KEY;
// Fullscreen mode enabled
extern bool FSMode;
// The current too big ratio
extern float TOO_SMALL_RATIO, X_OFFSET;
// An adjustment that was needed for some reason. Notice: This was okay for a 900 pixels high screen
extern float Adj;
// Console background color
#define SCR_BACKGROUND BACKGROUND_BLUE;
// Save the initial console window location
extern int ConsoleLeft, ConsoleTop, ConsoleIconic;

// Resize.cpp
void ShowTaskbar(bool);
void ResizeWindow(HWND, int, bool, bool, bool);
std::string VKToString(int);

// Console.cpp
void RegularText(); void BrightText(); void BlueBackground(); void BlackBackground();
void WhiteLine (); void PrintMessage(std::string Str[3], int);
void LetterSpace(); void PixelSpace(int, int, int, int);
void Position(); void Iconic(); void ShowWindowNoAnimate(HWND, int);
void ClearScreen();
void DisableCursor();

// Main.cpp
void StopWait4pSX(HWND hWnd);
HWND GetHwnd();
u32 Str2Hex(const char* _szValue);
//////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Black borders screen rations
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
/* 
	#0 (Default)
*/
	#define TOO_SMALL_RATIO0 0
/* 
	#1
		Final Fantasy 8 (NTSC): Top and bottom
		Tekken 3 (PAL): Top and bottom
		Vagrant Story (NTSC): Top and bottom
*/
	#define TOO_SMALL_RATIO1 0.066
/*
	#2
		Chrono Cross (NTSC): Top and bottom
*/
	#define TOO_SMALL_RATIO2 0.100
/*
	#3
		Final Fantasy 7 (PAL): Top and bottom
*/
	#define TOO_SMALL_RATIO3 0.125
/*
	#4
		Metal Gear Solid (PAL): Right
*/
/////////////////////////////////////