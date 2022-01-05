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
}

CReadMessage::~CReadMessage()
{
	free(this->buffer);
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
			printf("ERROR with read");
		}

		if (c == '\n' || c == '\r')
		{
			if (bytesRead == 0)
			{
				continue;
			}

			message.size = bytesRead;
			message.data = this->buffer;
			return 0;
		}

		this->buffer[bytesRead] = c;
		++bytesRead;
		if (bytesRead == this->maxBufferSize)
		{
			message.size = bytesRead;
			message.data = this->buffer;
			return 0;
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
		fprintf(stderr, "error %d from tggetattr", errno);
		return;
	}

	tty.c_cc[VMIN] = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr(this->fd, TCSANOW, &tty) != 0)
	{
		fprintf(stderr, "error %d setting term attributes", errno);
	}
}