#include "TrafficDetector.h"

using namespace std;
using namespace OnePunchMan;

TrafficDetector::TrafficDetector(int width, int height, MqttChannel* mqtt, bool debug)
	:_channelIndex(0), _channelUrl(), _width(width), _height(height), _mqtt(mqtt)
	, _lanesInited(false), _param(), _setParam(true)
	, _debug(debug)
{

}


bool TrafficDetector::LanesInited() const
{
	return _lanesInited;
}


