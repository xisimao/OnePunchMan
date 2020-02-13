#include "Path.h"

using namespace std;
using namespace Saitama;

#ifdef _WIN32 
const char Path::Separator = '\\';
#else
const char Path::Separator = '/';
#endif

string Path::GetCurrentPath()
{
	char buffer[1024] = { 0 };
	getcwd(buffer, 1024);
	return string(buffer);
}

string Path::GetExtension(const string& path)
{
	size_t index;
	if ((index = path.find_last_of('.')) == string::npos)
	{
		return string();
	}
	return path.substr(index + 1, path.length() - index - 1);
}

string Path::GetFileName(const string& path)
{
	if (path.empty())
	{
		return string();
	}
	else
	{
		size_t start = path.find_last_of(Separator);
		if (start == string::npos)
		{
			start = 0;
		}
		size_t end = path.find_last_of('.');
		return path.substr(start, end);
	}
}