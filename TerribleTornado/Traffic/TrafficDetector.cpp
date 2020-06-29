#include "TrafficDetector.h"

using namespace std;
using namespace OnePunchMan;

TrafficDetector::TrafficDetector(int width, int height, MqttChannel* mqtt)
	:_channelIndex(0), _channelUrl(), _width(width), _height(height), _mqtt(mqtt)
	, _lanesInited(false), _param(), _setParam(true),_writeBmp(false)
	, _bgrSize(width* height * 3), _jpgSize(static_cast<int>(tjBufSize(width, height, TJSAMP_422)))
	, _iveHandler(-1)
{

}

bool TrafficDetector::LanesInited() const
{
	return _lanesInited;
}

void TrafficDetector::WriteBmp()
{
	_writeBmp = true;
}

string TrafficDetector::GetDetectParam()
{
	return GetDetectParam("[]");
}

string TrafficDetector::GetDetectParam(const std::string& regions)
{
	return StringEx::Combine("{\"Detect\":{\"DetectRegion\":", regions, ",\"IsDet\":true,\"MaxCarWidth\":10,\"MinCarWidth\":10,\"Mode\":0,\"Threshold\":20,\"Version\":1001}}");
}
