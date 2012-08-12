// mmsys.cpl windows extension
// (C) John Peterson, GNU GPL 3
#include <iostream>
#include <windows.h>
#include "../../Common/Common.h"
using namespace std;

bool GetFileExists(wstring filename)
{
	DWORD fileAttr;
	if (GetFileAttributes(filename.c_str()) == 0xFFFFFFFF) return false;
	return true;
}

WNDPROC pOldWndProc = 0;
HWND hWnd = 0;

BOOL AttachDLL(DWORD ProcessID, wstring dllName) 
{
	HANDLE hProc; 
	LPVOID lpFileName, lpLoadLibrary; 

	if (!ProcessID) { wprintf(L"No process ID specified\n"); return false; }
	
	if (!(hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID)))
		wprintf(L"OpenProcess: %s\n", GetLastErrorEx().c_str());

	lpLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");

	lpFileName = (LPVOID)VirtualAllocEx(hProc, NULL, dllName.length()*2, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!WriteProcessMemory(hProc, (LPVOID)lpFileName, dllName.c_str(), dllName.length()*2, NULL))
		wprintf(L"WriteProcessMemory: %s\n", GetLastErrorEx().c_str());

	if (!CreateRemoteThread(hProc, NULL, NULL, (LPTHREAD_START_ROUTINE)lpLoadLibrary, (LPVOID)lpFileName, NULL, NULL))
		wprintf(L"CreateRemoteThread: %s\n", GetLastErrorEx().c_str());

	CloseHandle(hProc);
	return true;
}

int main() {
	wstring Process = L"rundll32.exe Shell32.dll,Control_RunDLL mmsys.cpl";	
	wstring DLL = L"mmsysEx.dll";
	if (!GetFileExists(DLL)) wprintf(L"Could not find '%s'\n", DLL.c_str());

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);
	if (!CreateProcess(NULL, LPWSTR(Process.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		wprintf(L"CreateProcess: %s\n", GetLastErrorEx().c_str());
	AttachDLL(pi.dwProcessId, DLL);
	//cin.get();
	//TerminateProcess(pi.hProcess, 0);
	//CloseHandle(pi.hProcess);
	//CloseHandle(pi.hThread);
    return 0;
}