#include "Debugging.h"
#include <iomanip>
#include <cstdio>
#include <sstream>
//include <stdio.h>

template< typename T >
std::string int_to_hex(T i, std::streamsize w)
{
	std::stringstream stream;
	stream //<< "0x"
		<< std::setfill('0') 
		//<< std::setw(sizeof(T) * 2)
		<< std::setw(w)
		<< std::hex << i;
	return stream.str();
}

void MemoryDump(void* address, unsigned long size) // Dump Memory
{
	unsigned long i;
	unsigned char buff[17];
	unsigned char* data = (unsigned char*)address;

	// Process every byte in the data.
	for (i = 0; i <= size; i++)
	{
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0)
		{
			// Just don't print ASCII for the zeroth line.
			if (i != 0)
			{
				//printf("  %s\n", buff);
				std::cout << "  " << buff << std::endl;
			}

			// Output the offset.
			//printf("  %04x ", i);
			std::cout << " " << int_to_hex(i,4) << " ";
		}

		// Now the hex code for the specific character.

		printf(" %02x", data[i]);
		//std::cout << " " << int_to_hex(data[i], 2);

		// And store a printable ASCII character for later.
		if ((data[i] < 0x20) || (data[i] > 0x7e))
		{
			buff[i % 16] = '.';
		}
		else
		{
			buff[i % 16] = data[i];
		}

		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0)
	{
		//printf("   ");
		std::cout << "   ";
		i++;
	}

	// And print the final ASCII bit.
	//printf("  %s\n", buff);
	std::cout << "  " << buff << std::endl;
}

void DumpMemoryAsBase64(void* address, unsigned long size)
{
	const unsigned char* data = (const unsigned char*)address;
	std::string b64 = base64_encode(data, size);

	std::cout << b64 << std::endl;
}