#include "ReadMessage.h"
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <cerrno>
#include <algorithm>

using namespace std;

bool Message::TryGetEffectivePower(std::uint32_t* power)
{
	static const uint8_t pattern[] = { 0x77, 0x07, 0x01, 0x00, 0x10, 0x07, 0x00, 0xFF,
										0x01,0x01,0x62,0x1B,0x52,0xff,0x55 };
	const uint8_t* p = search(this->data, this->data + this->size, pattern, pattern + sizeof(pattern));
	if (p == this->data + this->size)
	{
		return false;
	}

	p = p + sizeof(pattern);

	uint32_t v = (((uint32_t)p[0]) << 24) | (((uint32_t)p[1]) << 16) | (((uint32_t)p[2]) << 8) | (((uint32_t)p[3]));
	if (power != nullptr)
	{
		*power = v;
	}

	return true;
}

bool Message::TryGetEffectivePowerInWatts(double* power)
{
	uint32_t p;
	if (this->TryGetEffectivePower(&p))
	{
		if (power != nullptr)
		{
			*power = p * 0.1;
		}

		return true;
	}

	return false;
}

bool Message::TryGetTotalEnergy(std::uint64_t* energy)
{
	/*static const uint8_t pattern[] = {
	0x77,		0x07,		0x01,		0x00,		0x01,		0x08,		0x00,		0xFF,		0x64,
	0x00,		0x00,		0x82,		0x01,		0x62,		0x1E,		0x52,		0xFF,		0x56
	};*/

	static const uint8_t pattern[] = {
	0x77,0x07,0x01,0x00,0x01,0x08,0x01,0xFF,0x01,
	0x01,0x62,0x1E,0x52,0xFF,0x56,0x00 };
		//0x00,0x07,0xB1,0x52,0x05,0x01


	const uint8_t* p = search(this->data, this->data + this->size, pattern, pattern + sizeof(pattern));
	if (p == this->data + this->size)
	{
		return false;
	}

	p = p + sizeof(pattern);

	uint64_t v = (((uint32_t)p[0]) << 24) | (((uint32_t)p[1]) << 16) | (((uint32_t)p[2]) << 8) | (((uint32_t)p[3]));
	if (energy != nullptr)
	{
		*energy = v;
	}

	return true;
}

bool Message::TryGetTotalEnergyInWattHours(double* energy)
{
	uint64_t en;
	if (this->TryGetTotalEnergy(&en))
	{
		if (energy != nullptr)
		{
			*energy = en * 0.1;
		}

		return true;
	}

	return false;
}

CReadMessage::CReadMessage(const char* devName)
{
	this->fd = open(devName, O_RDWR | O_NOCTTY | O_SYNC);
	this->set_blocking(true);
	this->maxBufferSize = 8192;
	this->buffer = (uint8_t*)malloc(this->maxBufferSize);
	this->rawBuffer = (uint8_t*)malloc(this->maxBufferSize / 2);
}

CReadMessage::~CReadMessage()
{
	free(this->buffer);
	free(this->rawBuffer);
}

int
CReadMessage::ReadMessage(Message& message)
{
	size_t bytesRead = 0;
	for (;;)
	{
		uint8_t c;
		int n = read(fd, &c, 1);
		if (n <= 0)
		{
			printf("ERROR with read\n");
		}

		if (c == '\n' || c == '\r')
		{
			if (bytesRead == 0)
			{
				continue;
			}

			message = this->ConvertBufferToRawBuffer(bytesRead);
			//message.size = bytesRead;
			//message.data = this->buffer;
			return 0;
		}

		if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' ||
			c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f' || c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F')
		{
			this->buffer[bytesRead] = c;
			++bytesRead;
			if (bytesRead == this->maxBufferSize)
			{
				message = this->ConvertBufferToRawBuffer(bytesRead);
				/*message.size = bytesRead;
				message.data = this->buffer;*/
				return 0;
			}
		}
	}
}

void
CReadMessage::set_blocking(bool should_block)
{
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if (tcgetattr(this->fd, &tty) != 0)
	{
		fprintf(stderr, "error %d from tggetattr\n", errno);
		return;
	}

	tty.c_cc[VMIN] = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr(this->fd, TCSANOW, &tty) != 0)
	{
		fprintf(stderr, "error %d setting term attributes\n", errno);
	}
}

std::uint8_t HexToVal(char c)
{
	switch (c)
	{
	case '0':return 0;
	case '1':return 1;
	case '2':return 2;
	case '3':return 3;
	case '4':return 4;
	case '5':return 5;
	case '6':return 6;
	case '7':return 7;
	case '8':return 8;
	case '9':return 9;

	case 'a':case 'A':return 10;
	case 'b':case 'B':return 11;
	case 'c':case 'C':return 12;
	case 'd':case 'D':return 13;
	case 'e':case 'E':return 14;
	case 'f':case 'F':return 15;
	}

	return 0;
}

Message
CReadMessage::ConvertBufferToRawBuffer(size_t size)
{
	for (size_t i = 0; i < size / 2; ++i)
	{
		char c1 = HexToVal((char)*(this->buffer + i * 2));
		char c2 = HexToVal((char)*(this->buffer + i * 2 + 1));
		this->rawBuffer[i] = c1 * 16 + c2;
	}

	Message msg;
	msg.data = this->rawBuffer;
	msg.size = size / 2;
	return msg;
}