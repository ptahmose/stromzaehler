// Middleware.cpp : Defines the entry point for the application.
//

#include "Middleware.h"

using namespace std;

int main()
{
	FILE* fp = fopen("/dev/ttyACM0", "rb");

	for (;;)
	{
		int c = fgetc(fp);
		if (c != EOF)
		{
			fputc(c, stdout);
		}
	}

	return 0;
}
