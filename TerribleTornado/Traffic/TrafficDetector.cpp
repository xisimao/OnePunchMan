#include "TrafficDetector.h"

using namespace std;
using namespace OnePunchMan;

TrafficDetector::TrafficDetector(int width, int height, MqttChannel* mqtt)
	:_channelIndex(0), _channelUrl(), _width(width), _height(height), _mqtt(mqtt)
	, _lanesInited(false)
	, _bgrSize(width* height * 3), _jpgSize(static_cast<int>(tjBufSize(width, height, TJSAMP_422)))
{

}

bool TrafficDetector::LanesInited() const
{
	return _lanesInited;
}