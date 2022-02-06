// Middleware.cpp : Defines the entry point for the application.
//

#include "smlreceive.h"
#include "ReadMessage.h"
#include "utils.h"
#include <sys/file.h>
#include <unistd.h>
#include "azureiothub.h"
#include <sstream>
#include <iomanip> 
#include <chrono>
#include "cmdlineopts.h"

using namespace std;
using namespace std::chrono;

static void WriteValues(const string& filename, double power, double totalenergy)
{
	// Note: We use 'open' (instead of fopen etc.) here on purpose, because we 
	//       try to be cautious that we first get the lock, and then modify the file. To my
	//       understanding, with fopen and "w" it will be truncated (before getting the lock!).
	int fd = open(filename.c_str(), O_WRONLY | O_CREAT);
	if (fd >= 0)
	{
		stringstream ss;
		ss << std::setprecision(std::numeric_limits<long double>::digits10-2) << "{\"WP_Pges\":" << power << ",\"WP_Wges\":" << (totalenergy / 1000.0) << "}";
		string string = ss.str();
		auto size_string = string.size();
		flock(fd, LOCK_EX);
		lseek(fd, 0, SEEK_SET);
		write(fd, string.c_str(), size_string);
		ftruncate(fd, size_string);
		flock(fd, LOCK_UN);
		close(fd);
	}
}

static string GenerateShellCommand(const std::string& url, const std::string& user, const std::string& password, long long timestamp, double value)
{
	stringstream ss;
	ss << "wget -O - -q ";
	if (!user.empty())
	{
		// TODO: we'd need to escape " in the user/password
		ss << "--user \"" << user << "\" --password \"" << password << "\" ";
	}

	ss << "\"" << url << ".json?operation=add&ts=" << timestamp << "&value=" << value << "\"";

	return ss.str();
}

static void SendToVolkszaehler(
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
		//fprintf(stdout, "%s\n", ss.str().c_str());
		system(ss.str().c_str());
	}
	
}

int main(int argc, char** argv)
{
	CCmdLineOpts opts;
	opts.Parse(argc, argv);

	/*
	for (auto s : opts.GetRestHttpsUrls())
	{
		fprintf(stdout, "URL: %s\n", s.c_str());
	}

	for (auto s : opts.GetRestHttpsUsers())
	{
		fprintf(stdout, "USER: %s\n", s.c_str());
	}

	for (auto s : opts.GetRestHttpsPasswords())
	{
		fprintf(stdout, "Password: %s\n", s.c_str());
	}
	*/

	CReadMessage readMsg("/dev/ttyACM1");

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
					SendToVolkszaehler(opts.GetRestHttpsUrls(), opts.GetRestHttpsUsers(), opts.GetRestHttpsPasswords(), power);
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
