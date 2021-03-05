# tor-keylogger
A keylogger that sends keystrokes to tor onion endpoints using a self-contained tor library. (mini-tor by Petr Benes)
Contains code to implement asymmetric encryption (RSA, AES, Base64) on capture data to be used if you would like to 
secure communication before data is sent to the tor endpoint.
Also contains Lua to be used later to implement a polymorphic engine.


Only build that works is Debug/X86 - 32 bit build targets with Debug. 
Couldn't build everything in Release as there are linker errors and X64 targets don't build either.
Release builds would be ideal for optimisations and to run stealth code (which requires self-modification of virtual memory you only get safely when built in release.)
