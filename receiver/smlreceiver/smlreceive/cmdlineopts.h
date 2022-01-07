#pragma once

#include <vector>
#include <string>

class CCmdLineOpts
{
private:
	std::string filetowrite;
	std::vector<std::string> restHttpUrls;
	bool	verbose;

public:
	CCmdLineOpts();

	bool Parse(int argc, char** argv);

	const std::string& GetFileToWrite() const { return this->filetowrite; }
	const std::vector<std::string>& GetRestHttpsUrls() const { return this->restHttpUrls; }
	bool IsVerbose() const { return this->verbose; }
};