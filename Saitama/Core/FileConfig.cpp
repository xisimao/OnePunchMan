#include "FileConfig.h"

using namespace std;
using namespace Saitama;

FileConfig::FileConfig()
	:FileConfig("app.config")
{

}

FileConfig::FileConfig(const string& filePath)
{
	ifstream file;
	file.open(filePath);
	if (file.good())
	{
		string line;
		while (getline(file, line))
		{
			size_t i = line.find('=');
			if (i != string::npos)
			{
				string key = line.substr(0, i);
				key = StringEx::Trim(key);
				if (!key.empty() && key[0] != '#')
				{
					string value = line.substr(i + 1, line.length() - i - 1);
					value = StringEx::Trim(value);
					_configs.insert(pair<string, string>(key, value));
				}
			}
		}
	}
	file.close();
}






