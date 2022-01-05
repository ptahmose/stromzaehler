#include "ReadMessage.h"
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <cerrno>

using namespace std;

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