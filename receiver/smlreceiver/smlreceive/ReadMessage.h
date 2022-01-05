#pragma once
#include <memory>
#include <cstdint>
#include <cstdlib>


struct Message
{
	 std::uint8_t* data;
	size_t				size;

	std::uint16_t		GetCrc() const
	{
		if (size > 2)
		{
			return ((std::uint16_t)(this->data[this->size - 2]) << 8) | this->data[this->size - 1];
		}

		return 0;
	}
};

class CReadMessage
{
private:
	int fd;
	std::uint8_t* buffer;
	std::uint8_t* rawBuffer;
	size_t maxBufferSize;
public:
	CReadMessage(const char* devName);
	~CReadMessage();

	int ReadMessage(Message& message);
private:
	void set_blocking(bool should_block);
	Message ConvertBufferToRawBuffer(size_t size);
};