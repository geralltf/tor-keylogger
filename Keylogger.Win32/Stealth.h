#ifndef STEALTH_H
#define STEALTH_H

#include <Windows.h>
#include "EntryPoint.h"
#include "Crypto.h"
#include "Debugging.h"

// Activate user-mode stealth functionality.
void Stealth();

// Show the console window.
void ShowConsole();

// Hide the console window.
void HideConsole();

// Hide/show console window.
void Console();

// This function will erase the current images
// PE header from memory preventing a successful image
// if dumped.
void ErasePEHeaderFromMemory();

#endif