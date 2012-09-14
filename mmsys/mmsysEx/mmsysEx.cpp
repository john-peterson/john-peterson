// mmsys.cpl windows extension
// (C) John Peterson, GNU GPL 3
#include <iostream>
#include <windows.h>
#include "../../Common/Common.h"
using namespace std;

WNDPROC pWndProc = 0;
struct EnumStruct {
	HWND hBox;
};

void ShowStyle(HWND hWnd) {
	OutputDebugStringEx(L"GWL_STYLE: %s", WSTranslate(GetWindowLong(hWnd, GWL_STYLE)).c_str());
	OutputDebugStringEx(L"GWL_EXSTYLE: %s", WSEXTranslate(GetWindowLong(hWnd, GWL_EXSTYLE)).c_str());
}

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam) {
	EnumStruct* s = (EnumStruct*)lParam;
	wstring clsName = GetClassNameEx(hWnd);
	wstring wndName = GetWindowTextEx(hWnd);
	RECT rcBox, rcBox2, rc, rwBox, rwBox2, rw;
	GetClientRect(s->hBox, &rcBox); GetWindowRect(s->hBox, &rwBox);
	GetClientRect(hWnd, &rc); GetWindowRect(hWnd, &rw);
	if (!wndName.compare(L"Playback")
		|| !wndName.compare(L"Sounds")
		|| !wndName.compare(L"Recording")
		|| !wndName.compare(L"Communications")
	 ) {
		//OutputDebugStringEx(L"§0x%08x: %s | %d %d\n", hWnd, GetWindowTextEx(hWnd).c_str(), rcBox.right-rc.right, rcBox.bottom-rc.bottom);
		SetWindowPos(hWnd, 0, 0, 0, rcBox.right-20, rcBox.bottom-69, SWP_NOMOVE);
	}
	if (!clsName.compare(L"SysTabControl32")) {
		//OutputDebugStringEx(L"§0x%08x: %s | %d %d\n", hWnd, GetClassNameEx(hWnd).c_str(), rcBox.right-rc.right, rcBox.bottom-rc.bottom);
		SetWindowPos(hWnd, 0, 0, 0, rcBox.right-12, rcBox.bottom-43, SWP_NOMOVE);
	}
	if (!clsName.compare(L"SysListView32")) {
		//OutputDebugStringEx(L"§0x%08x: %s | %d %d\n", hWnd, GetClassNameEx(hWnd).c_str(), rcBox.right-rc.right, rcBox.bottom-rc.bottom);
		SetWindowPos(hWnd, 0, 0, 0, rcBox.right-45, rcBox.bottom-149, SWP_NOMOVE);
		ShowScrollBar(hWnd, SB_BOTH, FALSE);
	}
	if (!wndName.compare(L"&Configure")) {
		//OutputDebugStringEx(L"§0x%08x: %s | %d %d\n", hWnd, GetWindowTextEx(hWnd).c_str(), rwBox.right-rw.left, rwBox.bottom-rw.top);
		SetWindowPos(hWnd, 0, rcBox.right-385-10+8, rcBox.bottom-79-29+8, 0, 0, SWP_NOSIZE);
	}
	if (!wndName.compare(L"&Set Default")) {
		//OutputDebugStringEx(L"§0x%08x: %s | %d %d\n", hWnd, GetWindowTextEx(hWnd).c_str(), rwBox.right-rw.left, rwBox.bottom-rw.top);
		SetWindowPos(hWnd, 0, rcBox.right-199-10+8, rcBox.bottom-79-29+8, 0, 0, SWP_NOSIZE);
	}
	if (!wndName.compare(L"&Properties")) {
		//OutputDebugStringEx(L"§0x%08x: %s | %d %d\n", hWnd, GetWindowTextEx(hWnd).c_str(), rwBox.right-rw.left, rwBox.bottom-rw.top);
		SetWindowPos(hWnd, 0, rcBox.right-103-10+8, rcBox.bottom-79-29+8, 0, 0, SWP_NOSIZE);
	}
	if (!wndName.compare(L"OK")) {
		//OutputDebugStringEx(L"§0x%08x: %s | %d %d\n", hWnd, GetWindowTextEx(hWnd).c_str(), rwBox.right-rw.left, rwBox.bottom-rw.top);
		SetWindowPos(hWnd, 0, rcBox.right-251+8, rcBox.bottom-38+8, 0, 0, SWP_NOSIZE);
	}
	if (!wndName.compare(L"Cancel")) {
		//OutputDebugStringEx(L"§0x%08x: %s | %d %d\n", hWnd, GetWindowTextEx(hWnd).c_str(), rwBox.right-rw.left, rwBox.bottom-rw.top);
		SetWindowPos(hWnd, 0, rcBox.right-170+8, rcBox.bottom-38+8, 0, 0, SWP_NOSIZE);
	}
	if (!wndName.compare(L"&Apply")) {
		//OutputDebugStringEx(L"§0x%08x: %s | %d %d\n", hWnd, GetWindowTextEx(hWnd).c_str(), rwBox.right-rw.left, rwBox.bottom-rw.top);
		SetWindowPos(hWnd, 0, rcBox.right-89+8, rcBox.bottom-38+8, 0, 0, SWP_NOSIZE);
	}
	return true;
}

void OnResize(HWND hWnd) {
	EnumStruct es;
	es.hBox = hWnd;
	EnumChildWindows(hWnd, EnumChildProc, (LPARAM)&es);
}

LRESULT WINAPI WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	//OutputDebugStringEx(L"§WM: %s", WMTranslate(message).c_str());
	static bool bHTMINBUTTON = false, bHTMAXBUTTON = false;
	switch(message) {
		case WM_NCLBUTTONDOWN:
			if (bHTMINBUTTON) ShowWindow(hWnd, SW_MINIMIZE);
			if (bHTMAXBUTTON) ShowWindow(hWnd, IsZoomed(hWnd) ? SW_RESTORE: SW_MAXIMIZE);
			break;
		case WM_NCMOUSEMOVE:
			bHTMINBUTTON = (wParam == HTMINBUTTON);
			bHTMAXBUTTON = (wParam == HTMAXBUTTON);
			break;
		case WM_PAINT:
		case WM_SIZE: OnResize(hWnd); break;
	}
	return CallWindowProc(pWndProc, hWnd, message, wParam, lParam);
def:
	return DefDlgProc(hWnd, message, wParam, lParam);
}



DWORD WINAPI ThreadFunction(LPVOID lpParam) {
	HWND hWnd = 0;
	while (!hWnd) {
		hWnd = FindWindowByTitle(L"Sound");
		Sleep(1000/25);
	}
	ChangeStyle(hWnd, WS_VISIBLE|WS_OVERLAPPEDWINDOW);
	OverrideWndProc(pWndProc, WindowProc, hWnd);
	return 0;
}

void RunThread() {
	CreateThread(NULL, NULL, ThreadFunction, NULL, NULL, NULL);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		OutputDebugStringEx(L"DLL_PROCESS_ATTACH\n");
		RunThread();
		break;
	case DLL_PROCESS_DETACH:
		OutputDebugStringEx(L"DLL_PROCESS_DETACH\n");
		break;
	case DLL_THREAD_DETACH:
		OutputDebugStringEx(L"DLL_THREAD_DETACH\n");
		break;
	}
	return TRUE;
}