#include <cxxopts.hpp>
#include "cmdlineopts.h"

bool CCmdLineOpts::Parse(int argc, char** argv)
{
	cxxopts::Options options("smreceive", "get data from power-meter and distribute it");

	options.add_options()
		("f,write-to-file", "Write out the current values to the specified file (with flock)", cxxopts::value<std::string>())
		("r,rest-http-url", "URL for http-call (can be given multiple times)", cxxopts::value<std::vector<std::string>>())
		("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))
		("h,help", "Print usage")
		;

	auto result = options.parse(argc, argv);

	if (result.count("help"))
	{
		std::cout << options.help() << std::endl;
		exit(0);
	}

	if (result.count("verbose"))
	{
		this->verbose = result["verbose"].as<bool>();
	}

	if (result.count("rest-http-url"))
	{
		this->restHttpUrls = result["rest-http-url"].as<std::vector<std::string>>();
	}

	if (result.count("write-to-file"))
	{
		this->filetowrite = result["write-to-file"].as<std::string>();
	}

	return true;
}

