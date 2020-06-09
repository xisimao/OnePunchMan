#include "ImageConvert.h"

using namespace std;
using namespace OnePunchMan;

void ImageConvert::IveToBgr(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer)
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

int ImageConvert::BgrToJpg(const unsigned char* bgrBuffer, int width, int height, unsigned char** jpgBuffer, int jpgSize)
{
	unsigned char* tempJpgBuffer = *jpgBuffer;
	//jpg×ª»»
	tjhandle jpgHandle = tjInitCompress();
	unsigned long tempJpgSize = jpgSize;
	tjCompress2(jpgHandle, bgrBuffer, width, 0, height, TJPF_BGR, jpgBuffer, &tempJpgSize, TJSAMP_422, 10, TJFLAG_NOREALLOC);
	if (tempJpgBuffer != *jpgBuffer)
	{
		LogPool::Error(LogEvent::Detect, "jpg memory leak");
		free(*jpgBuffer);
		*jpgBuffer = tempJpgBuffer;
	}
	tjDestroy(jpgHandle);
	return static_cast<int>(tempJpgSize);
}

void ImageConvert::Yuv420spToYuv420p(const unsigned char* yuv420spBuffer, int width, int height, unsigned char* yuv420pBuffer)
{
	memcpy(yuv420pBuffer, yuv420spBuffer, width * height);
	const unsigned char* uv = yuv420spBuffer + width * height;
	unsigned char* u = yuv420pBuffer + width * height;
	unsigned char* v = u + width * height / 4;
	for (int i = 0; i < width * height / 4; ++i)
	{
		u[i] = uv[i * 2];
		v[i] = uv[i * 2 + 1];
	}
}


void ImageConvert::DrawPolygon(cv::Mat* image, const Polygon& polygon, const cv::Scalar& scalar)
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

void ImageConvert::DrawPoint(cv::Mat* image, const Point& point, const cv::Scalar& scalar)
{
	cv::circle(*image, cv::Point(point.X, point.Y), 10, scalar, -1);
}
