#include "ChannelDetector.h"

using namespace std;
using namespace OnePunchMan;

ChannelDetector::ChannelDetector(int width, int height, MqttChannel* mqtt)
	:_channelIndex(0), _channelUrl(), _width(width), _height(height), _mqtt(mqtt)
	, _param(), _setParam(true), _bgrBuffer(new unsigned char[width * height * 3])
{
}

ChannelDetector::~ChannelDetector()
{
	delete[] _bgrBuffer;
}

string ChannelDetector::ChannelUrl() const
{
	return _channelUrl;
}

void ChannelDetector::GetDetecItems(map<string, DetectItem>* items, const JsonDeserialization& jd, const string& key)
{
	int itemIndex = 0;
	while (true)
	{
		string id = jd.Get<string>(StringEx::Combine("ImageResults:0:", key, ":", itemIndex, ":GUID"));
		if (id.empty())
		{
			break;
		}
		int type = jd.Get<int>(StringEx::Combine("ImageResults:0:", key, ":", itemIndex, ":Type"));
		vector<int> rect = jd.GetArray<int>(StringEx::Combine("ImageResults:0:", key, ":", itemIndex, ":Detect:Body:Rect"));
		if (rect.size() >= 4)
		{
			DetectItem item;
			item.Region = Rectangle(Point(rect[0], rect[1]), rect[2], rect[3]);
			item.Type = static_cast<DetectType>(type);
			item.Status = DetectStatus::Out;
			item.Distance = 0.0;
			items->insert(pair<string, DetectItem>(id, item));
		}
		itemIndex += 1;
	}
}

void ChannelDetector::GetRecognItems(vector<RecognItem>* items, const JsonDeserialization& jd, const string& key)
{
	int itemIndex = 0;
	while (true)
	{
		string id = jd.Get<string>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":GUID"));
		if (id.empty())
		{
			break;
		}
		vector<int> rect = jd.GetArray<int>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":Detect:Body:Rect"));
		if (rect.size() >= 4)
		{
			RecognItem item;
			item.ChannelIndex = _channelIndex;
			item.Guid = id;
			item.Type = jd.Get<int>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":Type"));
			item.Width = jd.Get<int>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":Detect:Body:Width"));
			item.Height = jd.Get<int>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":Detect:Body:Height"));
			item.Region = Rectangle(Point(rect[0], rect[1]), rect[2], rect[3]);
			items->push_back(item);
		}
		itemIndex += 1;
	}
}

void ChannelDetector::IveToBgr(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer)
{
	const unsigned char* b = iveBuffer;
	const unsigned char* g = b + width * height;
	const unsigned char* r = g + width * height;
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			bgrBuffer[(j * width + i) * 3 + 0] = b[j * width + i];
			bgrBuffer[(j * width + i) * 3 + 1] = g[j * width + i];
			bgrBuffer[(j * width + i) * 3 + 2] = r[j * width + i];
		}
	}
}

vector<RecognItem> ChannelDetector::HandleDetect(const string& detectJson, string* param, const unsigned char* iveBuffer, long long packetIndex)
{
	if (!_setParam)
	{
		param->assign(_param);
		_setParam = true;
	}

	long long timeStamp = DateTime::UtcNowTimeStamp();
	JsonDeserialization detectJd(detectJson);
	map<string, DetectItem> detectItems;
	GetDetecItems(&detectItems, detectJd, "Vehicles");
	GetDetecItems(&detectItems, detectJd, "Bikes");
	GetDetecItems(&detectItems, detectJd, "Pedestrains");

	HandleDetectCore(detectItems, timeStamp,iveBuffer,packetIndex);

	vector<RecognItem> items;
	GetRecognItems(&items, detectJd, "Vehicles");
	GetRecognItems(&items, detectJd, "Bikes");
	GetRecognItems(&items, detectJd, "Pedestrains");
	return items;
}
