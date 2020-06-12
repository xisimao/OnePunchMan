#include "TrafficDetector.h"

using namespace std;
using namespace OnePunchMan;

TrafficDetector::TrafficDetector(int width, int height, MqttChannel* mqtt, bool debug)
	:_channelIndex(0), _channelUrl(), _width(width), _height(height), _mqtt(mqtt)
	, _lanesInited(false), _param(), _setParam(true)
	, _bgrSize(0), _bgrBuffer(NULL)
	, _jpgSize(0), _jpgBuffer(NULL)
	, _debug(debug), _jpgHandler(-1)
{
	_bgrSize = _width * _height * 3;
	_bgrBuffer = new unsigned char[_bgrSize];

	_jpgSize = static_cast<int>(tjBufSize(1920, 1080, TJSAMP_422));
	_jpgBuffer = tjAlloc(_jpgSize);

}

TrafficDetector::~TrafficDetector()
{
	tjFree(_jpgBuffer);
	delete[] _bgrBuffer;
}

bool TrafficDetector::LanesInited() const
{
	return _lanesInited;
}
