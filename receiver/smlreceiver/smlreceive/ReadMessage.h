#pragma once
#include <memory>
#include <cstdint>
#include <cstdlib>


struct Message
{
	const std::uint8_t* data;
	size_t				size;
};

class CReadMessage
{
private:
	int fd;
	std::uint8_t* buffer;
	size_t maxBufferSize;
public:
	CReadMessage(const char* devName);
	~CReadMessage();

	int ReadMessage(Message& message);
private:
	void set_blocking(bool should_block);
};