﻿// Middleware.cpp : Defines the entry point for the application.
//

#include "Middleware.h"
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h>
#include <string.h>

using namespace std;


void
set_blocking(int fd, bool should_block)
{
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if (tcgetattr(fd, &tty) != 0)
	{
		fprintf(stderr, "error %d from tggetattr", errno);
		return;
	}

	tty.c_cc[VMIN] = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr(fd, TCSANOW, &tty) != 0)
	{
		fprintf(stderr, "error %d setting term attributes", errno);
	}
}

int main()
{
	//FILE* fp = fopen("/dev/ttyACM0", "rb");
	int fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_SYNC);
	set_blocking(fd, true);
	for (;;)
	{
		char c;
		int n = read(fd, &c, 1);
		if (n > 0)
		{
			fputc(c, stdout);
		}
	}


	return 0;
}
