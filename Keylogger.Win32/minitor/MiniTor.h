#ifndef MINI_TOR_H
#define MINI_TOR_H

#include <string>

class MiniTor
{
private:
	void* tor = nullptr; // pointer to tor_client.

public:
	static constexpr size_t hops = 3;
	MiniTor();
	~MiniTor();
	void FetchConsensus();
	void ConnectAgain();
	std::string http_get(std::string url);
};

#endif