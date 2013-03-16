// Shared code.
// © John Peterson. License GNU GPL 3.

#include "common.h"
u8 verb = 1;

// log
void log(const char* f, ...) {
	va_list l;
	va_start(l, f);
	vfprintf(stderr, f, l);
	va_end(l);
	fflush(stderr);
}
void log(const char *f, va_list &l_) {
	const int len = 0xfff;
	char buf[len];
	va_list *l = &l_;
	vfprintf(stderr, f, *l);
	fflush(stderr);
}
void log1(const char* f, ...) {
	if (verb < 1) return;
	va_list l;
	va_start(l, f);
	log(f, l);
	va_end(l);
}
void log2(const char* f, ...) {
	if (verb < 2) return;
	va_list l;
	va_start(l, f);
	log(f, l);
	va_end(l);
}
void logc(int a, int c, char* f, ...) {
	const int len = 0xfff;
	char buf[len];
	va_list l;
	va_start(l, f);
	vsnprintf(buf, len, f, l);
	va_end(l);
	fprintf(stderr, "\033[%d;%dm%s\033[0m", a, c, buf);
	fflush(stderr);
}
#ifndef _WIN32
void logt(const char* f, ...) {
	const int len = 0xfff;
	char buf[len];
	va_list l;
	va_start(l, f);
	vsnprintf(buf, len, f, l);
	va_end(l);
	fprintf(stderr, "\033[1;30m%s\033[0m %s", time_hm().c_str(), buf);
	fflush(stderr);
}
#endif

// string

#ifndef _WIN32
string format(const char* format, ...) {
	const u32 len = 0x2000;
	char buffer[len];
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buffer, len, format, argptr);
	va_end(argptr);
	return string(buffer);
}
#else
wstring format(const wchar_t* format, ...) {
	const u32 len = 0x2000;
	wchar_t buffer[len];
	va_list argptr;
	va_start(argptr, format);
	_vsnwprintf(buffer, len, format, argptr);
	va_end(argptr);
	return wstring(buffer);
}
#endif
string arr2str(const u8 *buf, int len, int o) {
	string tmp;
	char tmp_c[16];
	for (int i = 0; i < len; i++) {
		sprintf(tmp_c, "%02x%s", buf[i+o], i>1 && (i+1)%16 == 0 ? "": " ");
		tmp.append(tmp_c);
		if(i>1 && (i+1)%8 == 0) tmp.append(" ");
		if(i>1 && (i+1)%16 == 0) tmp.append("\n");
	}
	if (len%16 != 0) tmp.append("\n");
	cerr << tmp;
	return tmp;
}
void* memcpy_rev(void* dest, const void* src, size_t n) {
	for (size_t i = 0; i < n; i++) {
		((u8*)dest)[n-i-1] = ((const u8*)src)[i];
	}
	return dest;
}
bool is_number(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

// string

// string to hex
u32 str2hex(const wchar_t* _szValue) {
	u32 value = 0;
	size_t finish = wcslen(_szValue);

	if (finish > 8)
		finish = 8;  // Max 32-bit values are supported.

	for (size_t count = 0; count < finish; count++) {
		value <<= 4;
		switch (_szValue[count]) {
		    case '0': break;
		    case '1': value += 1; break;
		    case '2': value += 2; break;
		    case '3': value += 3; break;
		    case '4': value += 4; break;
		    case '5': value += 5; break;
		    case '6': value += 6; break;
		    case '7': value += 7; break;
		    case '8': value += 8; break;
		    case '9': value += 9; break;
		    case 'A':
		    case 'a': value += 10; break;
		    case 'B':
		    case 'b': value += 11; break;
		    case 'C':
		    case 'c': value += 12; break;
		    case 'D':
		    case 'd': value += 13; break;
		    case 'E':
		    case 'e': value += 14; break;
		    case 'F':
		    case 'f': value += 15; break;
		    default:
			    return false;
			    break;
		}
	}
	return value;
}

// file
string read_line(s32 line_i, string fn) {
	istringstream line(read_file(fn, line_i));
	return line.str();
}
string read_file(string fn, s32 line_i) {
	ostringstream ss;
	string line;
	ifstream f(fn.c_str(), ifstream::in);
	if (f.is_open()) {
		s32 i = 0;
		while (f.good()) {
			i++;
			getline(f,line);
			if (line_i != -1 && line_i != i) continue;
			ss << line;
			if (line_i != -1) break;
		}
		f.close();
	}
	else log2("Unable to open %s\n", fn.c_str());
	return ss.str();
}
#ifndef _WIN32
void write_file(u32 s, string fn, bool append) {
	ofstream f(fn.c_str(), ios::out | (append ? ios::app : _Ios_Openmode(0)));
	if (f.is_open()) {
		if (append) f << endl;
		f << hex<<s;
		f.close();
	}
	else cerr << "Unable to open " << fn << " for writing" << endl;
}
#endif

// system
#ifndef _WIN32
string shell(const char* f, ...) {
	const int len = 0xfff;
	char buf[len];
	va_list l;
	va_start(l, f);
	vsnprintf(buf, len, f, l);
	va_end(l);
	logc(0, 36, "%s\n", buf);
	FILE *cmd = popen(buf, "r");
	fgets(buf, len, cmd);
	pclose(cmd);
	return string(buf);
}
string pid2cmd(unsigned long pid) {
	ostringstream file;
	file << "/proc/" << pid << "/cmdline";
	return read_file(file.str());
}
pid_t pn2pid(string pn_s) {
	string dir = "/proc";
	string cmd;
	string pn;
	vector<string> files;
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
		log2("Error %d opening %s\n", errno, dir.c_str());
		return 0;
    }
    while ((dirp = readdir(dp)) != NULL) {
		pn = dirp->d_name;
		if (!is_number(pn)) continue;
        files.push_back(pn);
		cmd = read_file(dir+"/"+pn+"/cmdline");
		if (cmd.find(pn_s) != string::npos && cmd.find(pn_s) <= 2) return pid_t(atoi(pn.c_str()));
    }
    closedir(dp);
	log("'%s' is not running\n", pn_s.c_str());
	return 0;
}
void send_signal(pid_t pid, int sig) {
	if (!pid) return;
	ostringstream cmd_k;
	cmd_k << "kill -s " << sig << " " << pid;
	log("Sending signal 9 to %d %s\n", pid, pid2cmd(pid).c_str());
	shell(cmd_k.str().c_str());
}
int is_stdin(int to) {
	fd_set fds;
	struct timeval timeout = {to,0};
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	select(STDIN_FILENO+1, &fds, NULL, NULL, &timeout);
    return FD_ISSET(STDIN_FILENO, &fds);
}
string home_dir() {
	struct passwd *pw = getpwuid(getuid());
	return string(pw->pw_dir);
}
#endif

// time
#ifndef _WIN32
double seconds() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000.0)+(tv.tv_usec/1000.0);
}
string time_hm() {
	time_t t = time(0);
	const u8 len = 0xff;
	char buf[len];
	strftime(buf, len, "%H:%M", localtime(&t));
	return string(buf);
}
#endif
#ifdef _WIN32

// other
void COMMON_API OutputDebugStringEx(const wchar_t* format, ...) {
	wchar_t buffer[1024*8];
	va_list argptr;
	va_start(argptr, format);
	_vsnwprintf(buffer, 1024*8, format, argptr);
	va_end(argptr);
	OutputDebugString(buffer);
}
wstring COMMON_API GetLastErrorEx() {
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);
	return wstring((LPWSTR)lpMsgBuf);
}

// windows

#ifndef __MINGW32__
wstring WMTranslate(UINT message) {
	switch(message) {
	case WM_ACTIVATE: return L"WM_ERASEBKGND";
	case WM_ERASEBKGND: return L"WM_ERASEBKGND";
	case WM_SETCURSOR: return L"WM_SETCURSOR";
	case WM_MOUSEACTIVATE: return L"WM_SETCURSOR";
	case WM_NOTIFY: return L"WM_SETCURSOR";
	case WM_NCMOUSEMOVE: return L"WM_NCMOUSEMOVE";
	case WM_NCLBUTTONDOWN: return L"WM_NCLBUTTONDOWN";
	case WM_NCLBUTTONUP: return L"WM_NCLBUTTONDOWN";
	case WM_NCLBUTTONDBLCLK: return L"WM_NCLBUTTONDOWN";
	case WM_NCHITTEST: return L"WM_NCHITTEST";
	case WM_CTLCOLORBTN: return L"WM_CTLCOLORBTN";
	case WM_CTLCOLORDLG: return L"WM_CTLCOLORDLG";
	case WM_MOUSEFIRST: return L"WM_MOUSEFIRST";
	case WM_CAPTURECHANGED: return L"WM_MOUSEFIRST";
	case WM_NCMOUSELEAVE: return L"WM_NCMOUSELEAVE";
	case WM_PRINTCLIENT: return L"WM_PRINTCLIENT";
	}
	return format(L"0x%x", message);
}

wstring WSTranslate(LONG style) {
	wstring s = L"";
	if(style&WS_BORDER) s += L"WS_BORDER|";
	if(style&WS_CAPTION) s += L"WS_CAPTION|";
	if(style&WS_CHILD) s += L"WS_CHILD|";
	if(style&WS_CHILDWINDOW) s += L"WS_CHILDWINDOW|";
	if(style&WS_CLIPCHILDREN) s += L"WS_CLIPCHILDREN|";
	if(style&WS_CLIPSIBLINGS) s += L"WS_CLIPSIBLINGS|";
	if(style&WS_DISABLED) s += L"WS_DISABLED|";
	if(style&WS_DLGFRAME) s += L"WS_DLGFRAME|";
	if(style&WS_GROUP) s += L"WS_GROUP|";
	if(style&WS_HSCROLL) s += L"WS_HSCROLL|";
	if(style&WS_ICONIC) s += L"WS_ICONIC|";
	if(style&WS_MAXIMIZE) s += L"WS_MAXIMIZE|";
	if(style&WS_MAXIMIZEBOX) s += L"WS_MAXIMIZEBOX|";
	if(style&WS_MINIMIZE) s += L"WS_MINIMIZE|";
	if(style&WS_MINIMIZEBOX) s += L"WS_MINIMIZEBOX|";
	if(style&WS_OVERLAPPED) s += L"WS_OVERLAPPED|";
	if(style&WS_POPUP) s += L"WS_POPUP|";
	if(style&WS_SIZEBOX) s += L"WS_SIZEBOX|";
	if(style&WS_SYSMENU) s += L"WS_SYSMENU|";
	if(style&WS_TABSTOP) s += L"WS_TABSTOP|";
	if(style&WS_THICKFRAME) s += L"WS_THICKFRAME|";
	if(style&WS_TILED) s += L"WS_TILED|";
	if(style&WS_VISIBLE) s += L"WS_VISIBLE|";
	if(style&WS_VSCROLL) s += L"WS_VSCROLL|";
	if(style&WS_OVERLAPPED
		&& style&WS_CAPTION
		&& style&WS_SYSMENU
		&& style&WS_THICKFRAME
		&& style&WS_MINIMIZEBOX
		&& style&WS_MAXIMIZEBOX) s += L" WS_OVERLAPPEDWINDOW";
	if(style&WS_POPUP
		&& style&WS_BORDER
		&& style&WS_SYSMENU) s += L" WS_POPUPWINDOW";
	if(style&WS_OVERLAPPED
		&& style&WS_CAPTION
		&& style&WS_SYSMENU
		&& style&WS_THICKFRAME
		&& style&WS_MINIMIZEBOX
		&& style&WS_MAXIMIZEBOX) s += L" WS_TILEDWINDOW";
	return s;
}

wstring WSEXTranslate(LONG style) {
	wstring s = L"";
	if(style&WS_EX_DLGMODALFRAME) s += L"WS_EX_DLGMODALFRAME|";
	if(style&WS_EX_NOPARENTNOTIFY) s += L"WS_EX_NOPARENTNOTIFY|";
	if(style&WS_EX_TOPMOST) s += L"WS_EX_TOPMOST|";
	if(style&WS_EX_ACCEPTFILES) s += L"WS_EX_ACCEPTFILES|";
	if(style&WS_EX_TRANSPARENT) s += L"WS_EX_TRANSPARENT|";
	if(style&WS_EX_MDICHILD) s += L"WS_EX_MDICHILD|";
	if(style&WS_EX_TOOLWINDOW) s += L"WS_EX_TOOLWINDOW|";
	if(style&WS_EX_WINDOWEDGE) s += L"WS_EX_WINDOWEDGE|";
	if(style&WS_EX_CLIENTEDGE) s += L"WS_EX_CLIENTEDGE|";
	if(style&WS_EX_CONTEXTHELP) s += L"WS_EX_CONTEXTHELP|";
	if(style&WS_EX_RIGHT) s += L"WS_EX_RIGHT|";
	if(style&WS_EX_LEFT) s += L"WS_EX_LEFT|";
	if(style&WS_EX_RTLREADING) s += L"WS_EX_RTLREADING|";
	if(style&WS_EX_LTRREADING) s += L"WS_EX_LTRREADING|";
	if(style&WS_EX_LEFTSCROLLBAR) s += L"WS_EX_LEFTSCROLLBAR|";
	if(style&WS_EX_RIGHTSCROLLBAR) s += L"WS_EX_RIGHTSCROLLBAR|";
	if(style&WS_EX_CONTROLPARENT) s += L"WS_EX_CONTROLPARENT|";
	if(style&WS_EX_STATICEDGE) s += L"WS_EX_STATICEDGE|";
	if(style&WS_EX_APPWINDOW) s += L"WS_EX_APPWINDOW|";
	return s;
}

// virtual keycodes names
wstring VKToString(int KeyCode) {
	// Default value
	wchar_t KeyStr[64] = {0};
	GetKeyNameText(MapVirtualKey(KeyCode, MAPVK_VK_TO_VSC) << 16, KeyStr, 64);
	wstring KeyString = KeyStr;

	switch(KeyCode)	{
		// Give it some help with a few keys
		case VK_END: return L"END";
		case VK_INSERT: return L"INS";
		case VK_DELETE: return L"DEL";
		case VK_PRIOR: return L"PGUP";
		case VK_NEXT: return L"PGDN";

		case VK_UP: return L"UP";
		case VK_DOWN: return L"DOWN";
		case VK_LEFT: return L"LEFT";
		case VK_RIGHT: return L"RIGHT";

		case VK_LSHIFT: return L"LEFT SHIFT";
		case VK_LCONTROL: return L"LEFT CTRL";
		case VK_RCONTROL: return L"RIGHT CTRL";
		case VK_LMENU: return L"LEFT ALT";

		default: return KeyString;
	}
}
#endif

HWND FindWindowByTitle(wstring find, bool exact) {
	wchar_t _title[256];
	HWND hWnd = GetForegroundWindow();

	while (hWnd != NULL) {
		int len = GetWindowText(hWnd, _title, 256);
		wstring title(_title);
		DWORD PID;
		GetWindowThreadProcessId(hWnd, &PID);

		if (GetCurrentProcessId() == PID) {
			if (exact) {
				if (!title.compare(find))
					break;
			} else {
				if (!title.compare(0, find.length(), find))
					break;
			}
		}
		hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	}

	if (!hWnd)
		OutputDebugStringEx(L"Unable to find window \"%s\"", find.c_str());
	else
		OutputDebugStringEx(L"HWND: 0x%x", hWnd);

	return hWnd;
}

wstring GetClassNameEx(HWND hWnd) {
	wchar_t buf[256];
	GetClassName(hWnd, buf, 256);
	return wstring(buf);
}

wstring GetWindowTextEx(HWND hWnd) {
	wchar_t buf[256];
	GetWindowText(hWnd, buf, 256);
	return wstring(buf);
}

void OverrideWndProc(WNDPROC pWndProc, WNDPROC WindowProc, HWND hWnd) {
	if (!IsWindow(hWnd)) return;
	if (!(pWndProc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC)))
		OutputDebugStringEx(L"pOldWndProc: %s", GetLastErrorEx().c_str());
	if (!SetWindowLongPtr(hWnd, GWLP_WNDPROC, LONG_PTR(WindowProc)))
		OutputDebugStringEx(L"SetWindowLongPtr: %s", GetLastErrorEx().c_str());
}

void ChangeStyle(HWND hWnd, LONG style) {
	SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE|WS_OVERLAPPEDWINDOW);
}

HWND SetWindowTextEx(wstring text) {
	HWND hWnd = GetConsoleWindow();
	SetWindowText(hWnd, text.c_str());
	return hWnd;
}

void SetIcon(LPCTSTR icon) {
	HMODULE hMainMod = GetModuleHandle(NULL);
	HICON hMainIcon = ::LoadIcon(hMainMod, icon);
	HMODULE hMod = LoadLibraryA("Kernel32.dll");
	typedef BOOL (CALLBACK *InsHook)(unsigned long, HANDLE);
	typedef DWORD (__stdcall *SCI)(HICON);
	SCI pfnSetConsoleIcon = reinterpret_cast<SCI>(GetProcAddress(hMod, "SetConsoleIcon"));
	pfnSetConsoleIcon(hMainIcon);
	FreeLibrary(hMod);
}

void SetAppID(HWND hWnd, wstring appId) {
	IPropertyStore *pps;
	HRESULT hr = SHGetPropertyStoreForWindow(hWnd, IID_PPV_ARGS(&pps));
	if (SUCCEEDED(hr)) {
		PROPVARIANT pv;
		hr = InitPropVariantFromString(appId.c_str(), &pv);
		if (SUCCEEDED(hr)) {
			hr = pps->SetValue(PKEY_AppUserModel_ID, pv);
			PropVariantClear(&pv);
		}
		pps->Release();
	}
}

// windows console

namespace Console {
// background color
const int SCR_BACKGROUND = BACKGROUND_BLUE;
// save the initial console window location
int ConsoleLeft, ConsoleTop, ConsoleIconic = 0;

// colors
void RegularText() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD Color = FOREGROUND_RED | FOREGROUND_GREEN |  FOREGROUND_BLUE | SCR_BACKGROUND;
	SetConsoleTextAttribute(hConsole, Color);
}
void BrightText() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD Color = FOREGROUND_RED | FOREGROUND_GREEN |  FOREGROUND_BLUE | FOREGROUND_INTENSITY | SCR_BACKGROUND;
	SetConsoleTextAttribute(hConsole, Color);
}
void BlueBackground() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD Color = SCR_BACKGROUND;
	SetConsoleTextAttribute(hConsole, Color);
}
void BlackBackground() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD Color = NULL;
	SetConsoleTextAttribute(hConsole, Color);
}
void WhiteLine() {
	RegularText();
	wprintf(L"                %c%c%c----", 250, 250, 250);
	BrightText();
	wprintf(L"--------");
	RegularText();
	wprintf(L"----%c%c%c      \n", 250, 250, 250);

}
void PrintMessage(wstring Str[3], int Rows) {
	wprintf(L"\n");
	for (int i = 0; i < Rows; i++) {
		WhiteLine();
		wprintf(L"%s", Str[i].c_str());
	}
}

// window status
void ShowWindowNoAnimate(HWND hWnd, int nCmdShow) {
   // Save the minimize window anitation parameter
   ANIMATIONINFO ai;
   ai.cbSize = sizeof(ai);
   SystemParametersInfo(SPI_GETANIMATION, sizeof(ai), &ai, 0);

   int nMinAnimate = ai.iMinAnimate;

   // No animation
   ai.iMinAnimate = 0;
   SystemParametersInfo(SPI_SETANIMATION, sizeof(ai), &ai, 0);

   // ShowWindow()
   ShowWindow(hWnd, nCmdShow);

   // Restore animation
   ai.iMinAnimate = nMinAnimate;
   SystemParametersInfo(SPI_SETANIMATION, sizeof(ai), &ai, 0);
}

// save console position and status
void Position() {
	RECT Rc;
	GetWindowRect(GetConsoleWindow(), &Rc);
	ConsoleLeft = Rc.left;
	ConsoleTop = Rc.top;

	#ifdef LOGGING
		wprintf(L"Position(): Left: %i Top:%i Iconic:%i\n", ConsoleLeft, ConsoleTop, ConsoleIconic);
	#endif
}

void SetIconic() {
	if (IsIconic(GetConsoleWindow()))
		ConsoleIconic = 1;
	else
		ConsoleIconic = 0;
}

// console letter space
void LetterSpace() {
	// Redraw the screen
	#ifndef LOGGING
		BlueBackground();
		ClearScreen();
	#endif

	// Console handle
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// Console size
	#ifdef LOGGING
		int Width = 93;
	#else
		int Width = 53;
	#endif

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

	// Minimize if it was minimized
	if (ConsoleIconic) ShowWindowNoAnimate(GetConsoleWindow(), SW_MINIMIZE);

	#ifdef LOGGING
		//wprintf(L"LetterSpace(): [SB:%i SW:%i] X:%i Y:%i W:%i | W:%i H:%i\n", SB, SW, ConInfo.dwSize.X, ConInfo.dwSize.Y,
		//	(ConInfo.srWindow.Right - ConInfo.srWindow.Left), Width, Height);
		wprintf(L"LetterSpace(): L:%i T:%i Iconic:%i\n", ConsoleLeft, ConsoleTop, ConsoleIconic);
	#endif
}
void PixelSpace(int Left, int Top, int Width, int Height) {
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
	//wprintf(L"[SB:%i SW:%i] X:%i Y:%i W:%i | W:%i H:%i\n", SB, SW, ConInfo.dwSize.X, ConInfo.dwSize.Y,
	//	(ConInfo.srWindow.Right - ConInfo.srWindow.Left), Width, Height);
}

// clear console screen
void ClearScreen() {
	COORD coordScreen = { 0, 0 };
	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD dwConSize;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	GetConsoleScreenBufferInfo(hConsole, &csbi);
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
	FillConsoleOutputCharacter(hConsole, TEXT(' '), dwConSize, coordScreen, &cCharsWritten);
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
	SetConsoleCursorPosition(hConsole, coordScreen);
}

// Disable Cursor
void DisableCursor() {
    CONSOLE_CURSOR_INFO info;
	HANDLE hOutput = GetStdHandle (STD_OUTPUT_HANDLE);

	// Turn the cursor off
	info.bVisible = FALSE;
	info.dwSize = 1;
    if (SetConsoleCursorInfo(hOutput,&info) == 0) {
	}
}
}
#endif