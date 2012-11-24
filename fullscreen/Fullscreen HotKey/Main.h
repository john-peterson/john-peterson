// Resize window.
// © John Peterson. License GNU GPL 3.

#include <iostream>
#include <string>
#include <windows.h>
#include <math.h>
#include "../../common/common.h"

// enable logging
//#define LOGGING
// Detected window title
#define WINDOW_TITLE L"pSX v1.13"
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

// Resize.cpp
void ShowTaskbar(bool);
void ResizeWindow(HWND, int, bool, bool, bool);

// Main.cpp
void StopWait4pSX(HWND hWnd);
HWND GetHwnd();

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