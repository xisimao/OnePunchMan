#include "JsonFormatter.h"

using namespace std;
using namespace Saitama;

JsonDeserialization::JsonDeserialization()
{

}

JsonDeserialization::JsonDeserialization(const string& json)
{
	Deserialize(json);
}

tuple<size_t, string> JsonDeserialization::CutString(const string& json, size_t offset)
{
	size_t startIndex = offset;
	size_t endIndex = startIndex;
	while (true)
	{
		endIndex = json.find('"', endIndex + 1);
		if (endIndex == string::npos)
		{
			return tuple<size_t, string>(0, string());
		}
		else
		{
			if (json[endIndex - 1] == '\\')
			{
				startIndex = endIndex - 1;
			}
			else
			{
				break;
			}
		}
	}

	size_t tailIndex = json.find_first_of(":,}", endIndex + 1);
	if (tailIndex == string::npos)
	{
		return tuple<size_t, string>(0, string());
	}
	else
	{
		return tuple<size_t, string>(tailIndex - offset + 1, json.substr(startIndex + 1, endIndex - startIndex - 1));
	}
}

tuple<size_t, string> JsonDeserialization::CutInteger(const string& json, size_t offset)
{
	size_t endIndex = json.find_first_of(",}", offset);
	if (endIndex == string::npos)
	{
		return tuple<size_t, string>(0, string());
	}
	else
	{
		return tuple<size_t, string>(endIndex - offset + 1, json.substr(offset, endIndex - offset));
	}
}

tuple<size_t, string> JsonDeserialization::CutByTag(const string& json, size_t offset, char head, char tail)
{
	size_t startIndex = json.find(head, offset);
	if (startIndex == string::npos)
	{
		return tuple<size_t, string>(0, string());
	}
	else
	{
		int headCount = 0;
		int tailCount = 0;
		for (string::const_iterator it = json.begin() + startIndex; it < json.end(); ++it)
		{
			if (*it == head)
			{
				headCount += 1;
			}
			else if (*it == tail)
			{
				tailCount += 1;
				if (headCount == tailCount)
				{
					size_t tailIndex = json.find_first_of(",]}", it - json.begin() + 1);
					if (tailIndex == string::npos)
					{
						return tuple<size_t, string>(0, string());
					}
					else
					{
						return tuple<size_t, string>(tailIndex - offset + 1, json.substr(startIndex, it - json.begin() - startIndex + 1));
					}
				}
			}
		}
		return tuple<size_t, string>(0, string());
	}
}

void JsonDeserialization::Deserialize(const string& json, const string& prefix)
{
	size_t offset = 0;
	while (offset < json.size())
	{
		size_t keyStartIndex = json.find('"', offset);
		if (keyStartIndex == string::npos)
		{
			break;
		}
		tuple<size_t, string> keyTuple = CutString(json, keyStartIndex);
		if (get<0>(keyTuple) == 0)
		{
			break;
		}
		string key = get<1>(keyTuple);
		if (!prefix.empty())
		{
			key = StringEx::Combine(prefix, ":", key);
		}
		size_t valueStartIndex = keyStartIndex+get<0>(keyTuple);

		valueStartIndex = json.find_first_not_of(" \t\n\r", valueStartIndex);

		if (valueStartIndex == string::npos)
		{
			break;
		}
		tuple<size_t, string> valueTuple;
		if (json[valueStartIndex] == '"')
		{
			valueTuple = CutString(json, valueStartIndex);
			_values.insert(pair<string, string>(key, get<1>(valueTuple)));
		}
		else if (json[valueStartIndex] == '{')
		{
			valueTuple = CutByTag(json, valueStartIndex, '{', '}');
			Deserialize(get<1>(valueTuple), key);
		}
		else if (json[valueStartIndex] == '[')
		{
			valueTuple = CutByTag(json, valueStartIndex, '[', ']');
			DeserializeArray(get<1>(valueTuple), key);
		}
		else
		{
			valueTuple = CutInteger(json, valueStartIndex);
			_values.insert(pair<string, string>(key, get<1>(valueTuple)));
		}
		offset += valueStartIndex-1 -offset+ get<0>(valueTuple);
	}
}

void JsonDeserialization::DeserializeArray(const string& json, const string& prefix)
{
	size_t jsonStartIndex = json.find_first_not_of("[ \t\n\r");
	if (jsonStartIndex == string::npos)
	{
		return;
	}
	else
	{
		if (json[jsonStartIndex] == '{')
		{
			size_t offset = 1;
			int index = 0;
			while (true)
			{
				tuple<size_t, string> t = CutByTag(json, offset, '{', '}');
				if (get<0>(t) == 0)
				{
					break;
				}
				else
				{
					offset += get<0>(t);
					Deserialize(get<1>(t), StringEx::Combine(prefix, ":", index));
					index += 1;
				}
			}
		}
		else
		{
			_values.insert(pair<string, string>(prefix, json));
		}
	}
}
