// © John Peterson. License GNU GPL 3.
#pragma once
#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#ifndef _WIN32
#include <dirent.h>
#include <getopt.h>
#include <pwd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#else
#include <windows.h>
#include <shobjidl.h>
#include <propvarutil.h>
#include <propkey.h>
#endif

using namespace std;
#ifndef _WIN32
typedef	uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
#else
typedef unsigned __int8 u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;
typedef signed __int8 s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;
#endif
#define COMMON_API __declspec(dllexport)
extern u8 verb;

// log
void log(const char* f, ...);
void log1(const char* f, ...);
void log2(const char* f, ...);
void logc(int a, int c, char* f, ...);
void logt(const char* f, ...);

// string
#ifndef _WIN32
string format(const char* format, ...);
#else
wstring format(const wchar_t* format, ...);
#endif
string arr2str(const u8 *buf, int len, int o);
void* memcpy_rev(void* dest, const void* src, size_t n);
bool is_number(const std::string& s);
u32 str2hex(const wchar_t* _szValue);

// file
string read_file(string, s32 line_i = -1);
void write_file(u32 p, string fn, bool append = false);
string read_line(s32 line_i, string fn);

// system
#ifndef _WIN32
string shell(const char* f, ...);
string pid2cmd(unsigned long pid);
pid_t pn2pid(string pn_s);
void send_signal(pid_t pid, int sig = 9);
int is_stdin(int to);
string home_dir();
#endif

// time
#ifndef _WIN32
double seconds();
string time_hm();
#endif

// windows
#ifdef _WIN32
void COMMON_API OutputDebugStringEx(const wchar_t* format, ...);
wstring COMMON_API GetLastErrorEx();
#ifndef __MINGW32__
wstring WMTranslate(UINT message);
wstring WSTranslate(LONG style);
wstring WSEXTranslate(LONG style);
wstring VKToString(int KeyCode);
#endif
HWND FindWindowByTitle(wstring find, bool exact = false);
wstring COMMON_API GetClassNameEx(HWND hWnd);
wstring COMMON_API GetWindowTextEx(HWND hWnd);
void OverrideWndProc(WNDPROC pWndProc, WNDPROC WindowProc, HWND hWnd);
void ChangeStyle(HWND hWnd, LONG style);
HWND SetWindowTextEx(wstring text);
void SetIcon(LPCTSTR icon);
void SetAppID(HWND hWnd, wstring appId);

// console
namespace Console {
extern const int SCR_BACKGROUND;
extern int ConsoleLeft, ConsoleTop, ConsoleIconic;

void RegularText(); void BrightText(); void BlueBackground(); void BlackBackground();
void WhiteLine (); void PrintMessage(wstring Str[3], int);
void LetterSpace(); void PixelSpace(int, int, int, int);
void Position(); void SetIconic(); void ShowWindowNoAnimate(HWND, int);
void ClearScreen();
void DisableCursor();
};
#endif