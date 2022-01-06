#pragma once

#include <string>
#include <functional>

class CAzureIot
{
private:
	char device_id[257];
	std::string connectionstring;
public:
	CAzureIot(const std::string& connectionString);

	void Run(std::function<bool(int, std::string&)> getMessage);
};
