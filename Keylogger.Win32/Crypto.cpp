#include "Crypto.h"
#include <stdlib.h>

BYTE Key[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED, 0xBE, 0xAD };

DWORD previousProtection = 0x00;

void UnlockMemRWX(void* address, unsigned long size)
{
	// Get read/write/execute privileges for the specified region of virtual memory:
	VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &previousProtection);
}


void UnlockMemRX(void* address, unsigned long size)
{
	// Get read/execute privileges for the specified region of virtual memory:
	VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READ, &previousProtection);
}

void UnlockMemRW(void* address, unsigned long size)
{
	// Get read/write privileges for the specified region of virtual memory:
	VirtualProtect((LPVOID)address, size, PAGE_READWRITE, &previousProtection);
}

void LockMem(void* address, unsigned long size)
{
	// Put back previous memory protections:
	VirtualProtect((LPVOID*)(address), size, previousProtection, &previousProtection);
}

void EraseMemory(void* address, unsigned long size)
{
	// Change memory protection.
	UnlockMemRW(address, size);

	// Erase the memory.
//#ifdef __STDC_LIB_EXT1__
	//memset_s(address, size, 0x00, size); // TODO: fill with random bytes
//#else
	unsigned long i = size;
	volatile unsigned char* p = (volatile unsigned char*)address;
	while (i--)
	{
		*p++ = (char)rand(); // Secure erase. 
		//TODO: Use OpenSSL's RAND_bytes()  #include <openssl/rand.h>

		//*p++ = 0x00; // Zero memory.
	}
//#endif

	LockMem(address, size);
}

void* SetMemory(void* address, size_t size_data, int value, size_t size_to_remove)
{
#ifdef __STDC_LIB_EXT1__
	memset_s(pointer, size_data, value, size_to_remove);
#else
	if (size_to_remove > size_data) size_to_remove = size_data;
	volatile unsigned char* p = (volatile unsigned char*)address;
	while (size_to_remove--)
	{
		*p++ = value;
	}
#endif
	return address;
}

char* cSecureString::GetString(std::string key, cCrypto* Crypt)
{
	if (bEncrypted)
	{
		XORString(key, Crypt);
		return szText;
	}
	return szText;
}

void cSecureString::CreateString(char* szString, cCrypto* Crypt)
{
	static const int LENGTH = strlen(szString);
	szText = new char[LENGTH + 1];
	strcpy_s(szText, sizeof(szString), szString);
	bEncrypted = false;
	XORString(std::string((char*)&Key), Crypt);
}

void cSecureString::XORString(std::string key, cCrypto* Crypt)
{
	bEncrypted = !bEncrypted;
	Crypt->XORMemory((DWORD)szText, (DWORD)strlen(szText), key, strlen((char*)key.c_str()));
}

void cCrypto::XORMemory(DWORD address, DWORD length, std::string key, DWORD KeyLength)
{
	BYTE buf[2048];
	unsigned long OldProtection;

	VirtualProtect((LPVOID)(address), length, PAGE_EXECUTE_READWRITE, &OldProtection);
	memcpy((void*)buf, (const void*)address, length);
	VirtualProtect((LPVOID)(address), length, OldProtection, nullptr);

	int KeyIndex = 0;
	for (DWORD i = 0x0; i < length; i++)
	{
		if (KeyIndex == KeyLength)
			KeyIndex = 0;
		buf[i] = buf[i] ^ key[KeyIndex];
		KeyIndex++;
	}

	VirtualProtect((LPVOID)(address), length, PAGE_EXECUTE_READWRITE, &OldProtection);
	memcpy((void*)address, (const void*)buf, length);
	VirtualProtect((LPVOID)(address), length, OldProtection, nullptr);
}

void AESMemoryEncrypt(void* address, unsigned long memorySize, std::string key)
{
	// Get privileges to the specific region of memory.
	UnlockMemRWX(address, memorySize);

	// AES enncrypt the memory region.
	uint8_t* k = (uint8_t*)key.c_str();
	struct AES_ctx aes;

	AES_init_ctx(&aes, k);

	uint8_t* buffer = (uint8_t*)address;

	AES_ECB_encrypt(&aes, buffer);

	// Release privileges to the specified memory.
	LockMem(address, memorySize);
}

void AESMemoryDecrypt(void* address, unsigned long memorySize, std::string key)
{
	// Get privileges to the specific region of memory.
	UnlockMemRWX(address, memorySize);

	// AES enncrypt the memory region.
	uint8_t* k = (uint8_t*)key.c_str();
	struct AES_ctx aes;

	AES_init_ctx(&aes, k);

	uint8_t* buffer = (uint8_t*)address;

	AES_ECB_decrypt(&aes, buffer);

	// Release privileges to the specified memory.
	LockMem(address, memorySize);
}

void XORMemory_Simple(void* address, unsigned long memorySize, std::string key)
{
	unsigned long startAddr = (unsigned long)address;

	DWORD previousProtection, modifiedProtection;

	// Get privileges to the specific region of memory.
	UnlockMemRWX(address, memorySize);

	// XOR Encrypt the memory region:
	unsigned long i;
	char* data = (char*)address;
	int keyIndex = 0;

	for (i = 0; i < memorySize; i++)
	{
		if (keyIndex == key.size())
		{
			keyIndex = 0;
		}
		data[i] = data[i] ^ key[keyIndex];
		keyIndex++;
	}

	// Release privileges to the specified memory.
	LockMem(address, memorySize);
}

void EncryptMemory(void* address, unsigned long memorySize, std::string key)
{
	XORMemory_Simple(address, memorySize, key);
	//AESMemoryEncrypt(address, memorySize, key);
}

void DecryptMemory(void* address, unsigned long memorySize, std::string key)
{
	XORMemory_Simple(address, memorySize, key);
	//AESMemoryDecrypt(address, memorySize, key);
}

unsigned long GetProcSize(void* Function, void* StubFunction)
{
	unsigned long dwFunctionSize = 0;
	unsigned long* fnA = nullptr, * fnB = nullptr;

	fnA = (unsigned long*)Function;
	fnB = (unsigned long*)StubFunction;

	// Get the function size by subtracting stub from original.
	dwFunctionSize = (fnB - fnA); // Memory is not predictable in Debug mode to do this. Project must be set to release.

	return dwFunctionSize;
}

void AESEncryptStr(std::string plain, std::string* cipher, std::string key, bool isCipherBase64)
{
	uint8_t* k = (uint8_t*)key.c_str();
	struct AES_ctx aes;

	AES_init_ctx(&aes, k);
	
	uint8_t* buffer = (uint8_t*)plain.c_str();
	uint8_t* copy = new uint8_t[plain.size()];

	for (size_t i = 0; i < plain.size(); i++)
	{
		copy[i] = buffer[i];
	}

	AES_ECB_encrypt(&aes, copy);

	std::string c = std::string((char*)copy, plain.size());
	delete[] copy;

	if (isCipherBase64) 
	{
		c = base64_encoder(c);
	}

	std::cout << "ENCRYPTING plain: " << plain << " cipher: " << c << std::endl;

	*cipher = c;
}

void AESDecryptStr(std::string cipher, std::string* plain, std::string key, bool isCipherBase64)
{
	uint8_t* k = (uint8_t*)key.c_str();
	struct AES_ctx aes;

	AES_init_ctx(&aes, k);

	std::string b64d_cipher;
	if (isCipherBase64) 
	{
		b64d_cipher = base64_decode(cipher);
	}
	else 
	{
		b64d_cipher = cipher;
	}

	uint8_t* buffer = (uint8_t*)b64d_cipher.c_str();
	uint8_t* copy = new uint8_t[b64d_cipher.size()];

	for (size_t i = 0; i < b64d_cipher.size(); i++)
	{
		copy[i] = buffer[i];
	}

	AES_ECB_decrypt(&aes, copy);

	std::string pt = std::string((char*)copy, b64d_cipher.size());
	delete[] copy;

	std::cout << "DECRYPTING cipher: " << cipher << " plain: " << pt << std::endl;

	*plain = pt;
}