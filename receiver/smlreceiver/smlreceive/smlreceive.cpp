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
#include <unistd.h>
#include "azureiothub.h"
#include <sstream>
#include <chrono>
#include "cmdlineopts.h"

using namespace std;
using namespace std::chrono;

static void WriteValues(const string& filename, double power, double totalenergy)
{
	//FILE* fp = fopen("/tmp/stromzaehler.txt", "wb");
	//FILE* fp = fopen("/mnt/RAMDisk/zaehlerdata/stromzaehler.txt", "wb");
	FILE* fp = fopen(filename.c_str(), "ab");
	if (fp != NULL)
	{
		// Note: We open the file in "append"-mode (not "truncate", which would be 'w'), we 
		//        try to be cautious that we first get the lock, and then modify the file. To my
		//        understanding, with "w" it will be truncated (before getting the lock!).
		int r = flock(fileno(fp), LOCK_EX);
		fseek(fp,0,SEEK_SET);
		fprintf(fp, "{\"WP_Pges\": %lf,\"WP_Wges\": %lf}", power, totalenergy / 1000);
		fflush(fp);
		auto length = ftell(fp);
		off_t offset = length;
		ftruncate(fileno(fp), offset);
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

/*
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
}*/

static string GenerateShellCommand(const std::string& url, const std::string& user, const std::string& password, long long timestamp, double value)
{
	stringstream ss;
	ss << "wget - O - -q ";
	if (!user.empty())
	{
		ss << "--user " << user << " --password " << password << " ";
	}

	ss << "\""<<url<<".json?operation=add&ts="<<timestamp<< "&value=" << value << "\"";

	return ss.str();
}

void SendToVolkszaehler(
		const std::vector<std::string>& urls,
		const std::vector<std::string>& users,
		const std::vector<std::string>& passwords,
		double d)
{
	milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

	stringstream ss;
	for (size_t i = 0; i < urls.size(); ++i)
	{
		string cmd = GenerateShellCommand(
			urls[i],
			i < users.size() ? users[i] : "",
			i < passwords.size() ? passwords[i] : "",
			ms.count(),
			d);

		if (i > 0)
		{
			ss << " ; ";
		}

		ss << cmd;
	}

	if (!ss.str().empty())
	{
		fprintf(stdout, "%s\n", ss.str().c_str());
		//system(ss.str().c_str());
	}

/*	milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	stringstream ss;
	ss << "wget -O - -q \"https://demo.volkszaehler.org/middleware/data/";
	ss << "67165e80-6fa9-11ec-9134-017bf40382af";
	ss << ".json?operation=add&ts=" << ms.count();
	ss << "&value=" << d << "\"";


	ss << "; wget -O - -q \"https://vz.nanox.de/middleware/data/";
	ss << "6a74cb20-6faa-11ec-a96c-1f3bc4c22881";
	ss << ".json?operation=add&ts=" << ms.count();
	ss << "&value=" << d << "\"";

	fprintf(stdout, "%s\n", ss.str().c_str());
	system(ss.str().c_str());*/
}

int main(int argc, char** argv)
{
	CCmdLineOpts opts;
	opts.Parse(argc, argv);

	for (auto s : opts.GetRestHttpsUrls())
	{
		fprintf(stdout,"URL: %s\n",s.c_str());
	}

	for (auto s : opts.GetRestHttpsUsers())
	{
		fprintf(stdout,"USER: %s\n",s.c_str());
	}

	for (auto s : opts.GetRestHttpsPasswords())
	{
		fprintf(stdout,"Password: %s\n",s.c_str());
	}


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

		if (opts.IsVerbose())
		{
			printf("crc: %04X (from msg: %04X) : %s\n", crc, msg.GetCrc(), (crc == msg.GetCrc()) ? "OK" : "FAIL");
		}

		bool crcCorrect = crc == msg.GetCrc();
		if (!crcCorrect)
		{
			*msg.data = 0x1b;
			crc = calculate_crc(msg);
			if (crc == msg.GetCrc())
			{
				crcCorrect = true;
				if (opts.IsVerbose())
				{
					printf("After correction -> OK\n");
				}
			}
		}

		if (crcCorrect)
		{
			double power;
			bool bPowerOk = msg.TryGetEffectivePowerInWatts(&power);
			if (opts.IsVerbose())
			{
				if (!bPowerOk)
				{
					printf("Effective Power : <not present>\n");
				}
				else
				{
					printf("Effective Power : %.1lf Watt\n", power);
				}
			}

			double totalenergy;
			bool bTotalEnergyOk = msg.TryGetTotalEnergyInWattHours(&totalenergy);
			if (opts.IsVerbose())
			{
				if (!bTotalEnergyOk)
				{
					printf("Total Energy: <not present>\n");
				}
				else
				{
					printf("Total Energy: %.1lf Watt*hour\n", totalenergy);
				}
			}

			if (bTotalEnergyOk == true && bPowerOk)
			{
				if (!opts.GetFileToWrite().empty())
				{
					WriteValues(opts.GetFileToWrite(), power, totalenergy);
				}
			}

			if (bPowerOk)
			{
				if (!opts.GetRestHttpsUrls().empty())
				{
					SendToVolkszaehler(opts.GetRestHttpsUrls(),opts.GetRestHttpsUsers(),opts.GetRestHttpsPasswords(), power);
				}
			}
		}

		/*for (size_t i = 0; i < msg.size; ++i)
		{
			printf("0x%02X", msg.data[i]);
		}*/

		printf("\n");
	}

	return 0;
}
