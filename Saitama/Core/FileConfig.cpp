#include "FileConfig.h"

using namespace std;
using namespace OnePunchMan;

FileConfig::FileConfig()
	:FileConfig("appsettings.json")
{

}

FileConfig::FileConfig(const string& filePath)
{
	string json;
	ifstream file;
	file.open(filePath);
	if (file.good())
	{
		string line;
		while (getline(file, line))
		{
			json.append(line);
		}
	}
	file.close();
	_jd.Deserialize(json);
}






