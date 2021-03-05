# tor-keylogger
A keylogger that sends keystrokes to tor onion endpoints using a self-contained tor library. (mini-tor by Petr Benes)
Contains code to implement asymmetric encryption (RSA, AES, Base64) on capture data to be used if you would like to 
secure communication before data is sent to the tor endpoint.
Also contains Lua to be used later to implement a polymorphic engine.
