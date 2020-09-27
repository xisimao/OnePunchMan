#include "StringEx.h"

using namespace std;
using namespace OnePunchMan;

const string StringEx::Base64Chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

string StringEx::ToHex(int value)
{
	char temp[9] = { 0 };
	sprintf(temp, "%x", value);
	return std::string(temp);
}

string StringEx::ToHex(string::const_iterator begin, string::const_iterator end, const string& separator)
{
	std::string str;
	char temp[3];
	for (string::const_iterator it = begin; it < end; ++it)
	{
		sprintf(temp, "%.2x", static_cast<unsigned char>(*it));
		str.append(temp, 2);
		str.append(separator);
	}
	return str;
}

string StringEx::ToHex(const char* buffer, unsigned int size, const string& separator)
{
	std::string str;
	char temp[3];
	for (unsigned int i = 0; i < size; ++i)
	{
		sprintf(temp, "%.2x", static_cast<unsigned char>(buffer[i]));
		str.append(temp, 2);
		str.append(separator);
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

void StringEx::ToBase64String(const unsigned char* buffer, unsigned int size, string* base64) {


	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (size--) {
		char_array_3[i++] = *(buffer++);
		if (i == 3) {
			char_array_4[0] = static_cast<unsigned char>((char_array_3[0] & 0xfc) >> 2);
			char_array_4[1] = static_cast<unsigned char>(((char_array_3[0] & 0x03) << 4) +
				((char_array_3[1] & 0xf0) >> 4));
			char_array_4[2] = static_cast<unsigned char>(((char_array_3[1] & 0x0f) << 2) +
				((char_array_3[2] & 0xc0) >> 6));
			char_array_4[3] = static_cast<unsigned char>(char_array_3[2] & 0x3f);

			for (i = 0; (i < 4); i++) {
				base64->push_back(Base64Chars[char_array_4[i]]);
			}
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 3; j++) {
			char_array_3[j] = '\0';
		}

		char_array_4[0] = static_cast<unsigned char>((char_array_3[0] & 0xfc) >> 2);
		char_array_4[1] = static_cast<unsigned char>(((char_array_3[0] & 0x03) << 4) +
			((char_array_3[1] & 0xf0) >> 4));
		char_array_4[2] = static_cast<unsigned char>(((char_array_3[1] & 0x0f) << 2) +
			((char_array_3[2] & 0xc0) >> 6));
		char_array_4[3] = static_cast<unsigned char>(char_array_3[2] & 0x3f);

		for (j = 0; (j < i + 1); j++) {
			base64->push_back(Base64Chars[char_array_4[j]]);
		}

		while ((i++ < 3)) {
			base64->push_back('=');
		}
	}
}

string StringEx::FromBase64String(std::string const& value) {
	size_t in_len = value.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (value[in_] != '=') && IsBase64(value[in_])) {
		char_array_4[i++] = value[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++) {
				char_array_4[i] = static_cast<unsigned char>(Base64Chars.find(char_array_4[i]));
			}

			char_array_3[0] = static_cast<unsigned char>((char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4));
			char_array_3[1] = static_cast<unsigned char>(((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2));
			char_array_3[2] = static_cast<unsigned char>(((char_array_4[2] & 0x3) << 6) + char_array_4[3]);

			for (i = 0; (i < 3); i++) {
				ret += char_array_3[i];
			}
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = static_cast<unsigned char>(Base64Chars.find(char_array_4[j]));

		char_array_3[0] = static_cast<unsigned char>((char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4));
		char_array_3[1] = static_cast<unsigned char>(((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2));
		char_array_3[2] = static_cast<unsigned char>(((char_array_4[2] & 0x3) << 6) + char_array_4[3]);

		for (j = 0; (j < i - 1); j++) {
			ret += static_cast<std::string::value_type>(char_array_3[j]);
		}
	}

	return ret;
}

inline bool StringEx::IsBase64(unsigned char c) {
	return (c == 43 || // +
		(c >= 47 && c <= 57) || // /-9
		(c >= 65 && c <= 90) || // A-Z
		(c >= 97 && c <= 122)); // a-z
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

string StringEx::Replace(const string& value, const string& oldValue, const string& newValue)
{
	string temp(value);
	size_t index = temp.find(oldValue);
	while (index != string::npos)
	{
		temp = temp.replace(index, oldValue.size(), newValue);
		index = temp.find(oldValue);
	}
	return temp;
}

string StringEx::Rounding(float value, int precision)
{
	stringstream ss;
	ss << setiosflags(ios::fixed) << setprecision(precision);
	ss << value;
	return ss.str();
}

string StringEx::Rounding(double value, int precision)
{
	stringstream ss;
	ss << setiosflags(ios::fixed) << setprecision(precision);
	ss << value;
	return ss.str();
}