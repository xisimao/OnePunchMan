#include "StringFormatter.h"

using namespace std;
using namespace Saitama;

void StringFormatter::Serialize(string* buffer, const bool& value)
{
	buffer->push_back(value);
}

void StringFormatter::Serialize(string* buffer, const float& value)
{
	Float_Convert fc;
	fc.F = value;
	Serialize(buffer, fc.I);
}

void StringFormatter::Serialize(std::string* buffer, const double& value)
{
	Double_Convert dc;
	dc.D = value;
	Serialize(buffer, dc.L);
}

void StringFormatter::Serialize(std::string* buffer, const std::string& value)
{
	buffer->append(value);
	buffer->push_back(0);
}

unsigned int StringFormatter::Deserialize(std::string::const_iterator begin, std::string::const_iterator end, bool* value)
{
	if (static_cast<unsigned int>((end-begin)) < sizeof(bool))
	{
		return 0;
	}
	*value = *begin != 0;
	return sizeof(bool);
}

unsigned int StringFormatter::Deserialize(std::string::const_iterator begin, std::string::const_iterator end, float* value)
{
	Float_Convert fc;
	unsigned int result = Deserialize(begin, end, &fc.I);
	*value = fc.F;
	return result;
}

unsigned int StringFormatter::Deserialize(std::string::const_iterator begin, std::string::const_iterator end, double* value)
{
	Double_Convert dc;
	unsigned int result = Deserialize(begin, end, &dc.L);
	*value = dc.D;
	return result;
}

unsigned int StringFormatter::Deserialize(std::string::const_iterator begin, std::string::const_iterator end, std::string* value)
{
	if ((end - begin) < static_cast<int>(value->length()))
	{
		return 0;
	}
	value->assign(begin, find(begin, end, 0));
	return static_cast<unsigned int>(value->length() + 1);
}