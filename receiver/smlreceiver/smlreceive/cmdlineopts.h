#pragma once

#include <vector>
#include <string>

class CCmdLineOpts
{
private:
	std::string filetowrite;
	std::vector<std::string> restHttpUrls;
	std::vector<std::string> users;
	std::vector<std::string> passwords;
	bool	verbose;

public:
	CCmdLineOpts();

	bool Parse(int argc, char** argv);

	const std::string& GetFileToWrite() const { return this->filetowrite; }
	const std::vector<std::string>& GetRestHttpsUrls() const { return this->restHttpUrls; }
	const std::vector<std::string>& GetRestHttpsUsers() const { return this->users; }
	const std::vector<std::string>& GetRestHttpsPasswords() const { return this->passwords; }
	bool IsVerbose() const { return this->verbose; }
};