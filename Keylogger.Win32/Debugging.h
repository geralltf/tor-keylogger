#ifndef DEBUGGING_H
#define DEBUGGING_H

#include <string>
#include <iostream>
#include "base64.h"

template< typename T >
std::string int_to_hex(T i, std::streamsize w);

// For analysing memory given a specified starting address and length of memory in bytes.
void MemoryDump(void* address, unsigned long size);

// For analysing memory in Base64 given a specified starting address and length of memory in bytes.
void DumpMemoryAsBase64(void* address, unsigned long size);

#endif