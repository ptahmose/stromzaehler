#include "ReadMessage.h"
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>

using namespace std;

CReadMessage::CReadMessage(const char* devName)
{
	this->fd = open(devName, O_RDWR | O_NOCTTY | O_SYNC);
	this->set_blocking(true);
	this->buffer = unique_ptr<uint8_t, decltype(free)>(malloc(8192), free);
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