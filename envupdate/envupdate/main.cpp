#include <atlbase.h>
#include <atlstr.h>
#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include "rev.h"

// broadcast WM_SETTINGCHANGE
void update_env() {
	DWORD dwReturnValue;
	SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, &dwReturnValue);
	wprintf(L"WM_SETTINGCHANGE sent to HWND_BROADCAST.\n");
}

void usage() {
	wprintf(L"Usage: envupdate [-v]\n-v, --version\tprint version\n");
}

int _tmain(int argc, _TCHAR* argv[]) {
	// arguments
	if (argc > 1) {
		CString s(argv[1]);
		if (!s.Compare(L"-v") || !s.Compare(L"--version")) {
			wprintf(L"%s %s\n", TEXT(VER_DATE), TEXT(VER));			
		} else {
			wprintf(L"Invalid argument.\n\n");
			usage();
		}
		return 0;
	}

	update_env();

	return 0;
}