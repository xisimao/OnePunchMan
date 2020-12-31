#include "ImageDrawing.h"

using namespace std;
using namespace OnePunchMan;

void ImageDrawing::DrawPoint(cv::Mat* image, const Point& point, const cv::Scalar& scalar)
{
	cv::circle(*image, cv::Point(point.X, point.Y), 10, scalar, -1);
}

void ImageDrawing::DrawRectangle(cv::Mat* image, const Rectangle& rectangle, const cv::Scalar& scalar)
{
	cv::Rect rect(rectangle.Top().X, rectangle.Top().Y, rectangle.Width(), rectangle.Height());
	cv::rectangle(*image, rect, scalar, 10);
}

void ImageDrawing::DrawPolygon(cv::Mat* image, const Polygon& polygon, const cv::Scalar& scalar)
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

void ImageDrawing::DrawString(cv::Mat* image, const std::string& text, const Point& point, const cv::Scalar& scalar)
{
	cv::Point cvPoint(point.X, point.Y);
	cv::putText(*image, text, cvPoint, cv::FONT_HERSHEY_COMPLEX, 2, scalar, 2);
}