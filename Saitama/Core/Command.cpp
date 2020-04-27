#include "Command.h"

using namespace std;
using namespace OnePunchMan;

string Command::Execute(const std::string& cmd)
{
	char line[1024] = { 0 };
	string lines;
	FILE *ptr;
	if ((ptr = popen(cmd.c_str(), "r")) != NULL)
	{
		while (fgets(line, 1024, ptr) != NULL)
		{
			lines.append(string(line));
		}
		pclose(ptr);
	}
	return lines;
}

string Command::ReplacePattern(const std::string& cmd)
{
	string result(cmd);
	for (unsigned int i = 0; i < result.size(); ++i)
	{
		if (result[i] == ' ')
		{
			result.erase(i, 1);
			result.insert(i, "\\s");
		}
	}
	return result;
}

string Command::RepleaceEscape(const std::string& cmd)
{
	string result(cmd);
	for (unsigned int i = 0; i < result.size(); ++i)
	{
		if (result[i] == '\\')
		{
			result.erase(i, 1);
			result.insert(i, "\\\\");
			++i;
		}
	}
	return result;
}
