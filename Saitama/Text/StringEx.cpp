#include "StringEx.h"

using namespace std;
using namespace Saitama;

string StringEx::ToHex(int value)
{
	char temp[9] = { 0 };
	sprintf(temp, "%x", value);
	return std::string(temp);
}

string StringEx::ToHex(string::const_iterator begin, string::const_iterator end)
{
	std::string str;
	char temp[3];
	for (string::const_iterator it = begin; it < end; ++it)
	{
		sprintf(temp, "%.2x", static_cast<unsigned char>(*it));
		str.append(temp, 2);
		str.append(" ");
	}
	return str;
}

string StringEx::ToHex(const char* buffer, unsigned int size)
{
	std::string str;
	char temp[3];
	for (unsigned int i = 0; i < size; ++i)
	{
		sprintf(temp, "%.2x", static_cast<unsigned char>(buffer[i]));
		str.append(temp, 2);
		str.append(" ");
	}
	return str;
}

string StringEx::ToBytes(const std::string& value, char separator)
{
	string buffer;
	size_t start = 0;
	size_t end = 0;
	do
	{
		end = value.find(separator, start);
		char c = static_cast<char>(strtol(value.substr(start, end - start).c_str(), NULL, 16));
		buffer.push_back(c);
		start = end + 1;
		if (start == value.length())
		{
			break;
		}
	} while (end != string::npos);
	return buffer;
}

char StringEx::Xor(const char* buffer, unsigned int size)
{
	return (char)accumulate(buffer, buffer + size, 0, [](char a, char b) {return a ^ b; });
}

string StringEx::ToUpper(const string& str)
{
	string result(str);
	transform(result.begin(), result.end(), result.begin(), ::toupper);
	return result;
}

string StringEx::Trim(const string& str)
{
	string result(str);
	if (result.empty())
	{
		return result;
	}

	const string WhiteSpace = " \t\n\r";
	if (WhiteSpace.find(result[0]) == string::npos &&
		WhiteSpace.find(result[result.length() - 1]) == string::npos)
	{
		return result;
	}

	string::size_type start = result.find_first_not_of(WhiteSpace);
	string::size_type end = result.find_last_not_of(WhiteSpace);
	result.erase(end + 1, result.length() - end - 1);
	result.erase(0, start);
	return result;
}

vector<string> StringEx::Split(const string& value, const string& separator, bool filterEmpty)
{
	vector<string> result;
	size_t end = 0;
	while (true)
	{
		size_t start = end;
		end = value.find(separator, start);
		string temp;
		if (end == string::npos)
		{
			temp.assign(value.substr(start, value.length() - start));
		}
		else if (start == end)
		{
			temp.assign("");
		}
		else
		{
			temp.assign(value.substr(start, end - start));
		}

		if (temp.empty())
		{
			if (!filterEmpty)
			{
				result.push_back(temp);
			}
		}
		else
		{
			result.push_back(temp);
		}

		if (end == string::npos)
		{
			break;
		}
		else
		{
			end += separator.size();
		}
	}
	return result;
}

string StringEx::Rounding(float value, int precision)
{
	stringstream ss;
	ss << setiosflags(ios::fixed) << setprecision(1);
	ss << value;
	return ss.str();
}