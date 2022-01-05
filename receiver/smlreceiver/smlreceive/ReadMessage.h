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
	std::unique_ptr<std::uint8_t,decltype(std::free)> buffer;
public:
	CReadMessage(const char* devName);

	int ReadMessage(Message& message);
private:
	void set_blocking(bool should_block);
};