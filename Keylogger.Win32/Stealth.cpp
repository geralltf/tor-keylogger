#include "Stealth.h"

//#ifdef visible
#include <io.h>
#include <fcntl.h>
//#endif

void Stealth()
{
	// Wisibility of window.
	Console();

	// Prevent runtime imaging of executable, hence debugging. Only works when project is built in release mode.
	//ErasePEHeaderFromMemory();
}

void ShowConsole() 
{
	AllocConsole();
	HANDLE stdHandle;
	int hConsole;
	FILE* fp;
	stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	hConsole = _open_osfhandle((long)stdHandle, _O_TEXT);
	fp = _fdopen(hConsole, "w");
	freopen_s(&fp, "CONOUT$", "w", stdout);

	HWND hWnd = GetConsoleWindow();
	ShowWindow(hWnd, SW_NORMAL); // visible window
}

void HideConsole()
{
	// 2. Second way to hide console - still shows button momentarily in taskbar.
	//HWND hWnd = FindWindowA("ConsoleWindowClass", nullptr);
	HWND hWnd = GetConsoleWindow();

	long style = GetWindowLong(hWnd, GWL_EXSTYLE);
	style &= ~(WS_VISIBLE);    // this works - window become invisible 

	//style |= WS_EX_TOOLWINDOW;   // flags don't work - windows remains in taskbar
	//style &= ~(WS_EX_APPWINDOW);

	ShowWindow(hWnd, SW_HIDE); // hide the window
	SetWindowLong(hWnd, GWL_EXSTYLE, style); // set the style
	ShowWindow(hWnd, SW_SHOW); // show the window for the new style to come into effect
	ShowWindow(hWnd, SW_HIDE); // hide the window so we can't see it

	// 3. Simple way to hide console - still shows button momentarily in taskbar.
	//ShowWindow(hWnd, SW_HIDE); // invisible window
}

void Console()
{
#ifdef visible
	ShowConsole();
	//HideConsole();
#endif

#ifdef invisible
	// 1. Best way to hide console - also hides button showing in taskbar.
	// Subsystem is set to WINDOWS an WinMain entrypoint included
	// so console is disabled by default.

	// 2. Second way to hide console - still shows button momentarily in taskbar.
	//HWND hWnd = FindWindowA("ConsoleWindowClass", nullptr);
	////HWND hWnd = GetConsoleWindow();

	//long style = GetWindowLong(hWnd, GWL_EXSTYLE);
	//style &= ~(WS_VISIBLE);    // this works - window become invisible 

	//style |= WS_EX_TOOLWINDOW;   // flags don't work - windows remains in taskbar
	//style &= ~(WS_EX_APPWINDOW);

	//ShowWindow(hWnd, SW_HIDE); // hide the window
	//SetWindowLong(hWnd, GWL_EXSTYLE, style); // set the style
	//ShowWindow(hWnd, SW_SHOW); // show the window for the new style to come into effect
	//ShowWindow(hWnd, SW_HIDE); // hide the window so we can't see it

	// 3. Simple way to hide console - still shows button momentarily in taskbar.
	//ShowWindow(hWnd, SW_HIDE); // invisible window

	//FreeConsole();
#endif
}

void ErasePEHeaderFromMemory()
{
	// Get base address of module
	void* baseAddr = (void*)GetModuleHandle(nullptr);

	std::cout << std::endl << "PE Header:" << std::endl;
	MemoryDump(baseAddr, 200);
	
	EraseMemory(baseAddr, 0x1000); // '4096' - Assume x86/x64 4Kb page size

	std::cout << std::endl << "PE Header Securely Erased:" << std::endl;
	MemoryDump(baseAddr, 200);

	std::cout << std::endl;
}