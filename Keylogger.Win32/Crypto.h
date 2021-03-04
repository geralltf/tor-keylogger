// Crypto.h
// Memory management and cryptography functions.

#ifndef CRYPTO_H
#define CRYPTO_H

#include "aes.h"
#include "base64.h"

#define __STDC_WANT_LIB_EXT1__ 1

#include <Windows.h>
#include <iostream>

#define LTM_DESC
#include <tomcrypt.h>
#include <time.h>
#define PTSIZE 87
#define CPSIZE 1024
#define BUFFERSIZE 1024

// Turned off DEP in linker settings so that program memory can be overwritten.
// Linker -> Advanced -> Data Execution Prevention (DEP): No /NXCOMPAT:NO
// Also optimisation under C++ -> Optimisation is disabled.

// Virtual memory security override functions.
void UnlockMemRWX(void* address, unsigned long size);
void UnlockMemRX(void* address, unsigned long size);
void UnlockMemRW(void* address, unsigned long size);
void LockMem(void* address, unsigned long size);

// Secure memory erasure
void EraseMemory(void* address, unsigned long size);
void* SetMemory(void* address, size_t size_data, int value, size_t size_to_remove);

class cCrypto
{
public:
	void XORMemory(DWORD address, DWORD length, std::string key, DWORD KeyLength);
};

class cSecureString
{
public:
	char* szText;
	bool bEncrypted;
	void CreateString(char* szString, cCrypto* Crypt);
	void XORString(std::string key, cCrypto* Crypt);
	char* GetString(std::string key, cCrypto* Crypt);
};

// Simpler version:
void XORMemory_Simple(void* address, unsigned long memorySize, std::string key);
void EncryptMemory(void* address, unsigned long memorySize, std::string key);
void DecryptMemory(void* address, unsigned long memorySize, std::string key);

unsigned long GetProcSize(void* Function, void* StubFunction);

void AESEncryptStr(std::string plain, std::string* cipher, std::string key, bool isCipherBase64);
void AESDecryptStr(std::string cipher, std::string* plain, std::string key, bool isCipherBase64);

#endif