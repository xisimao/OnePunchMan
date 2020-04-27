#include "ByteFormatter.h"

using namespace std;
using namespace OnePunchMan;

unsigned int ByteFormatter::Serialize(char* buffer, unsigned int capacity, const bool& value)
{
	if (capacity < sizeof(bool))
	{
		return 0;
	}
	buffer[0] = value;
	return sizeof(bool);
}

unsigned int ByteFormatter::Serialize(char* buffer, unsigned int capacity, const float& value)
{
	Float_Convert fc;
	fc.F = value;
	return Serialize(buffer, capacity, fc.I);
}

unsigned int ByteFormatter::Serialize(char* buffer, unsigned int capacity, const double& value)
{
	Double_Convert dc;
	dc.D = value;
	return Serialize(buffer, capacity, dc.L);
}

unsigned int ByteFormatter::Serialize(char* buffer, unsigned int capacity, const std::string& value)
{
	if (capacity < value.length()+1)
	{
		return 0;
	}
	memcpy(buffer, value.c_str(), value.length());
	buffer[value.length()] = 0;
	return static_cast<unsigned int>(value.length() + 1);
}

unsigned int ByteFormatter::Deserialize(const char* buffer, unsigned int size, bool* value)
{
	if (size < sizeof(bool))
	{
		return 0;
	}
	*value = buffer[0] != 0;
	return sizeof(bool);
}

unsigned int ByteFormatter::Deserialize(const char* buffer, unsigned int size, float* value)
{
	Float_Convert fc;
	unsigned int result=Deserialize(buffer, size, &fc.I);
	*value = fc.F;
	return result;
}

unsigned int ByteFormatter::Deserialize(const char* buffer, unsigned int size, double* value)
{

	Double_Convert dc;
	unsigned int result = Deserialize(buffer, size, &dc.L);
	*value = dc.D;
	return result;
}

unsigned int ByteFormatter::Deserialize(const char* buffer, unsigned int size, std::string* value)
{
	if (size < value->length())
	{
		return 0;
	}
	value->assign(buffer);
	return static_cast<unsigned int>(value->length()+1);
}