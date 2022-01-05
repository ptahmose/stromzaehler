// Middleware.cpp : Defines the entry point for the application.
//

#include "Middleware.h"
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h>

using namespace std;

int main()
{
	//FILE* fp = fopen("/dev/ttyACM0", "rb");
	int fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_SYNC);

	for (;;)
	{
		char c;
		int n = read(fd, &c, 1);
		if (n > 0)
		{
			printf("%i\n", c);
		}
	}


	return 0;
}
