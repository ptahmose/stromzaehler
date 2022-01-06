#pragma once

#include <string>

class CAzureIot
{
private:
	char device_id[257];
	std::string connectionstring;
public:
	CAzureIot(const std::string& connectionString);

	void Run();
};