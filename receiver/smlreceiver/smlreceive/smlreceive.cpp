// Middleware.cpp : Defines the entry point for the application.
//

#include "smlreceive.h"
//#include <fcntl.h> 
//#include <unistd.h>
//#include <termios.h>
//#include <string.h>
#include "ReadMessage.h"
#include "utils.h"
#include <sys/file.h>
#include "azureiothub.h"
#include <sstream>
#include <chrono>

using namespace std;
using namespace std::chrono;

static void WriteValues(double power, double totalenergy)
{
	//FILE* fp = fopen("/tmp/stromzaehler.txt", "wb");
	FILE* fp = fopen("/mnt/RAMDisk/zaehlerdata/stromzaehler.txt", "wb");
	if (fp != NULL)
	{
		int r = flock(fileno(fp), LOCK_EX);
		fprintf(fp, "{\"WP_Pges\": %lf,\"WP_Wges\": %lf}", power, totalenergy / 1000);
		flock(fileno(fp), LOCK_UN);
		fclose(fp);
	}
}


//void
//set_blocking(int fd, bool should_block)
//{
//	struct termios tty;
//	memset(&tty, 0, sizeof tty);
//	if (tcgetattr(fd, &tty) != 0)
//	{
//		fprintf(stderr, "error %d from tggetattr", errno);
//		return;
//	}
//
//	tty.c_cc[VMIN] = should_block ? 1 : 0;
//	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout
//
//	if (tcsetattr(fd, TCSANOW, &tty) != 0)
//	{
//		fprintf(stderr, "error %d setting term attributes", errno);
//	}
//}

static void run_iot(const char* connectionString)
{
	CReadMessage readMsg("/dev/ttyACM0");

	CAzureIot azureIot(connectionString);
	azureIot.Run(
		[&](int count, string& data)->bool
		{
			Message msg;
			int r = readMsg.ReadMessage(msg);
			uint16_t crc = calculate_crc(msg);

			printf("crc: %04X (from msg: %04X) : %s\n", crc, msg.GetCrc(), (crc == msg.GetCrc()) ? "OK" : "FAIL");
			if (crc != msg.GetCrc())
			{
				*msg.data = 0x1b;
				crc = calculate_crc(msg);
				if (crc == msg.GetCrc())
				{
					printf("After correction -> OK\n");
				}
			}

			if (crc == msg.GetCrc())
			{
				double power;
				bool bPowerOk = msg.TryGetEffectivePowerInWatts(&power);
				if (!bPowerOk)
				{
					printf("Effective Power : <not present>");
				}
				else
				{
					printf("Effective Power : %.1lf Watt\n", power);
				}

				double totalenergy;
				bool bTotalEnergyOk = msg.TryGetTotalEnergyInWattHours(&totalenergy);
				if (!bTotalEnergyOk)
				{
					printf("Total Energy: <not present>");
				}
				else
				{
					printf("Total Energy: %.1lf Watt*hour\n", totalenergy);
				}

				if (bTotalEnergyOk == true && bPowerOk)
				{
					WriteValues(power, totalenergy);
				}

				if (bPowerOk)
				{
					stringstream str{};
					str << "{ \"Power\": " << power << " }";
					data = str.str();
					return true;
				}
			}

			return false;
		});
}

void SendToVolkszaehler(double d)
{
	milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	stringstream ss;
	ss << "wget - O - -q \"https://demo.volkszaehler.org/middleware/data/";
	ss << "67165e80-6fa9-11ec-9134-017bf40382af";
	ss << ".json?operation=add&ts=" << ms.count();
	ss << "&value=" << d << "\"";


	ss << "; wget - O - -q \"https://vz.nanox.de/middleware/data/";
	ss << "6a74cb20-6faa-11ec-a96c-1f3bc4c22881";
	ss << ".json?operation=add&ts=" << ms.count();
	ss << "&value=" << d << "\"";

	fprintf(stdout, "%s\n", ss.str().c_str());
	system(ss.str().c_str());
}

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		run_iot(argv[1]);
	}
	else
	{

		printf("Hello World\n");
		/*
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
		*/
		CReadMessage readMsg("/dev/ttyACM0");

		for (;;)
		{
			Message msg;
			int r = readMsg.ReadMessage(msg);
			uint16_t crc = calculate_crc(msg);

			printf("crc: %04X (from msg: %04X) : %s\n", crc, msg.GetCrc(), (crc == msg.GetCrc()) ? "OK" : "FAIL");
			if (crc != msg.GetCrc())
			{
				*msg.data = 0x1b;
				crc = calculate_crc(msg);
				if (crc == msg.GetCrc())
				{
					printf("After correction -> OK\n");
				}
			}

			if (crc == msg.GetCrc())
			{
				double power;
				bool bPowerOk = msg.TryGetEffectivePowerInWatts(&power);
				if (!bPowerOk)
				{
					printf("Effective Power : <not present>");
				}
				else
				{
					printf("Effective Power : %.1lf Watt\n", power);
				}

				double totalenergy;
				bool bTotalEnergyOk = msg.TryGetTotalEnergyInWattHours(&totalenergy);
				if (!bTotalEnergyOk)
				{
					printf("Total Energy: <not present>");
				}
				else
				{
					printf("Total Energy: %.1lf Watt*hour\n", totalenergy);
				}

				if (bTotalEnergyOk == true && bPowerOk)
				{
					WriteValues(power, totalenergy);
				}

				if (bPowerOk)
				{
					SendToVolkszaehler(power);
				}
			}

			for (size_t i = 0; i < msg.size; ++i)
			{
				printf("0x%02X", msg.data[i]);
			}

			printf("\n");
		}
	}

	return 0;
}
