#pragma once
#include <cstdint>

struct Message
{
	const std::uint8_t* data;
	size_t				size;
};

class CReadMessage
{
private:
	int fd;
	std::unique_ptr<std::uint8_t,decltype(free)> buffer;
public:
	CReadMessage(const char* devName);

	int ReadMessage(Message& message);
private:
	void set_blocking(bool should_block);
};