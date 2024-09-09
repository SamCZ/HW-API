#include <iostream>
#include <cstdarg>
#include <string>
#include <fstream>
#include <memory>
#include <cstdio>
#include <regex>

#include "json.hpp"
#include "Socket.hpp"

using Json = nlohmann::json;

std::string exec(const char* cmd) {
	std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
	if (!pipe) return "ERROR";
	char buffer[128];
	std::string result = "";
	while (!feof(pipe.get())) {
		if (fgets(buffer, 128, pipe.get()) != NULL)
			result += buffer;
	}
	return result;
}

Json GetDiskUsage()
{
	std::stringstream stream(exec("df -h | grep /dev/xvd | awk '{print $1,$5}'"));

	Json json;

	std::string line;
	while (std::getline(stream, line))
	{
		auto space = line.find(' ');

		if (space == std::string::npos)
			continue;

		json[line.substr(0, space)] = line.substr(space + 1, line.length() - space - 2);
	}

	return json;
}

bool BothAreSpaces(char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); }

std::vector<std::string> SplitString(const std::string& str, char delimiter)
{
	std::vector<std::string> vec;

	std::stringstream stream(str);
	std::string line;
	while (getline(stream, line, delimiter))
	{
		vec.emplace_back(line);
	}

	return vec;
}

Json GetMemoryUsage()
{
	std::stringstream stream(exec("free -h"));
	Json json;

	std::string line;
	std::getline(stream, line);

	bool isMem = true;
	while (std::getline(stream, line))
	{
		std::string::iterator new_end = std::unique(line.begin(), line.end(), BothAreSpaces);
		line.erase(new_end, line.end());

		std::vector<std::string> vals = SplitString(line, ' ');

		if (isMem)
		{
			isMem = false;
			json["mem"]["total"] = vals[1];
			json["mem"]["used"] = vals[2];
			json["mem"]["free"] = vals[3];
			json["mem"]["shared"] = vals[4];
			json["mem"]["buff/cache"] = vals[5];
			json["mem"]["available"] = vals[6];
		} else
		{
			json["swap"]["total"] = vals[1];
			json["swap"]["used"] = vals[2];
			json["swap"]["free"] = vals[3];
		}
	}

	return json;
}

int GetNumberOfCpus()
{
	std::string input = exec("cat /proc/cpuinfo | grep processor | wc -l");
	std::string output = std::regex_replace(input, std::regex("[^0-9]*([0-9]+).*"), std::string("$1"));
	return std::stoi(output, nullptr);
}

double GetCpuLoad()
{
	return std::stod(exec("uptime | awk '{print $12}' "), nullptr);
}

int main(int argc, char* argv[])
{
	uint16_t port = 8283;

	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-p") == 0)
		{
			if ((i + 1) < argc - 1)
			{
				port = atoi(argv[i + 1]);
			}
		}
	}

	std::cout << "Starting server monitor on port " << port << std::endl;

	TCPServer server("0.0.0.0", port);
	if (server.Start())
	{
		server.Listen([]() -> std::string {
			Json json;
			json["cpu_count"] = GetNumberOfCpus();
			json["cpu_load"] = GetCpuLoad();
			json["memory"] = GetMemoryUsage();
			json["disks"] = GetDiskUsage();

			return json.dump(4);
		});
	}

	return 0;
}
