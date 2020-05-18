#include "TrafficDetector.h"

using namespace std;
using namespace OnePunchMan;

TrafficDetector::TrafficDetector(int width, int height, MqttChannel* mqtt, bool debug)
	:_channelIndex(0), _channelUrl(), _width(width), _height(height), _mqtt(mqtt)
	, _lanesInited(false), _param(), _setParam(true), _bgrBuffer(new unsigned char[width * height * 3]), _jpgBuffer(new unsigned char[width * height])
	, _debug(debug), _jpgHandler(-1)
{
}

TrafficDetector::~TrafficDetector()
{
	delete[] _jpgBuffer;
	delete[] _bgrBuffer;
}

bool TrafficDetector::LanesInited() const
{
	return _lanesInited;
}

void TrafficDetector::IveToBgr(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer)
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

int TrafficDetector::BgrToJpg(const unsigned char* bgrBuffer, int width, int height, unsigned char** jpgBuffer)
{
	tjhandle handle = tjInitCompress();
	unsigned long jpgSize=0;
	tjCompress2(handle, bgrBuffer, width, 0, height, TJPF_BGR, jpgBuffer, &jpgSize, TJSAMP_422, 10, 0);
	tjDestroy(handle);
	return static_cast<int>(jpgSize);
}

void TrafficDetector::JpgToBase64(string* base64,const unsigned char* jpgBuffer, int jpgSize)
{
	base64->assign("data:image/jpg;base64,");
	StringEx::ToBase64String(jpgBuffer, jpgSize, base64);
}

void TrafficDetector::DrawPolygon(cv::Mat* image, const Polygon& polygon, const cv::Scalar& scalar)
{
	vector<vector<cv::Point>> polygons;
	vector<cv::Point> points;
	for (unsigned int j = 0; j < polygon.Points().size(); ++j)
	{
		points.push_back(cv::Point(polygon.Points()[j].X, polygon.Points()[j].Y));
	}
	polygons.push_back(points);
	cv::polylines(*image, polygons, true, scalar, 3);
}

void TrafficDetector::DrawPoint(cv::Mat* image, const Point& point,const cv::Scalar& scalar)
{
	cv::circle(*image, cv::Point(point.X, point.Y), 10, scalar, -1);
}