#pragma once
#include <algorithm>

namespace Saitama
{
	//点
	class Point
	{
	public:

		/**
		* @brief: 构造函数
		*/
		Point()
			:Point(0, 0)
		{

		}

		/**
		* @brief: 构造函数
		* @param: x 点的x值
		* @param: y 点的y值
		*/
		Point(int x, int y)
			:X(x), Y(y)
		{

		}

		//点的x值
		int X;
		//点的Y值
		int Y;

		/**
		* @brief: 是否为空值
		* @return: 返回ture表示空值
		*/
		bool Empty()
		{
			return X == 0 && Y == 0;
		}

		/**
		* @brief: 计算改点到点的距离
		* @param: p 另外一个点
		* @return: 两点的距离
		*/
		double Distance(const Point& point) const
		{
			int x = X - point.X;
			int y = Y - point.Y;
			return sqrt(static_cast<double>(x)* static_cast<double>(x) + static_cast<double>(y)* static_cast<double>(y));
		}

	};

	//线段
	class Line
	{
	public:

		/**
		* @brief: 构造函数
		*/
		Line()
			:Line(Point(),Point())
		{

		}

		/**
		* @brief: 构造函数
		* @param: point1 第一个点
		* @param: point2 第二个点
		*/
		Line(const Point& point1, const Point& point2)
			:Point1(point1),Point2(point2)
		{

		}

		//第一个点
		Point Point1;
		//第二个点
		Point Point2;

		/**
		* @brief: 是否为空值
		* @return: 返回ture表示空值
		*/
		bool Empty()
		{
			return Point1.Empty() && Point2.Empty();
		}

		/**
		* @brief: 计算和另外一条线的相交点
		* @param: line 另外一条线
		* @return: 相交点
		*/
		Point Intersect(const Line& line)
		{
			int s02_x, s02_y, s10_x, s10_y, s32_x, s32_y;
			s10_x = Point2.X - Point1.X;
			s10_y = Point2.Y - Point1.Y;
			s32_x = line.Point2.X - line.Point1.X;
			s32_y = line.Point2.Y - line.Point1.Y;

			double denom = static_cast<double>(s10_x * s32_y - s32_x * s10_y);
			if (denom == 0)
				return Point(); 
			bool denomPositive = denom > 0;

			s02_x = Point1.X - line.Point1.X;
			s02_y = Point1.Y - line.Point1.Y;
			double s_numer = static_cast<double>(s10_x * s02_y) - static_cast<double>(s10_y * s02_x);
			if ((s_numer < 0) == denomPositive)
				return Point();

			double t_numer = static_cast<double>(s32_x * s02_y) - static_cast<double>(s32_y * s02_x);
			if ((t_numer < 0) == denomPositive)
				return Point(); 

			if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
				return Point(); 

			double t = t_numer / denom;
			return Point(Point1.X + static_cast<int>(t * s10_x), Point1.Y + static_cast<int>(t * s10_y));
		}

		/**
		* @brief: 计算线段的中点
		* @return: 中点
		*/
		Point Middle()
		{
			return Point((Point1.X + Point2.X) / 2, (Point1.Y + Point2.Y) / 2);
		}

	};

	//矩形
	class Rectangle
	{
	public:

		/**
		* @brief: 构造函数
		*/
		Rectangle()
			:Rectangle(Point(), 0, 0)
		{

		}

		/**
		* @brief: 构造函数
		* @param: x 矩形左上角点的x值
		* @param: y 矩形左上角点的y值
		* @param: width 矩形的宽度
		* @param: height 矩形的高度
		*/
		Rectangle(int x,int y, int width, int height)
			:Rectangle(Point(x,y), width,height)
		{

		}

		/**
		* @brief: 构造函数
		* @param: top 矩形左上角的点
		* @param: width 矩形的宽度
		* @param: height 矩形的高度
		*/
		Rectangle(const Point& top, int width, int height)
			:Top(top), Width(width), Height(height)
		{

		}

		//矩形左上角的点
		Point Top;
		//矩形的宽度
		int Width;
		//矩形的高度
		int Height;

	};

	//多边形
	class Polygon
	{
	public:

		/**
		* @brief: 构造函数
		*/
		Polygon()
			:Polygon(std::vector<Point>())
		{

		}

		/**
		* @brief: 构造函数
		* @param: points 点集合
		*/
		Polygon(const std::vector<Point>& points)
			:_points(points)
		{
			
		}

		/**
		* @brief: 测试点是否包含在多边形
		* @param: point 点
		* @return: 点在多边形内返回true，否则返回false
		*/
		bool Contains(const Point& point)
		{
			if (_points.empty())
			{
				return false;
			}
			int intersectCount= 0;
			Point p1= _points[0];
			for (int i = 1; i <= _points.size(); i++) {
				Point p2 = _points[i % _points.size()];
				if ((point.X - p1.X) * (p2.Y - p1.Y) == (p2.X - p1.X) * (point.Y - p1.Y)
					&& point.X >= std::min(p1.X, p2.X)
					&& point.X <= std::max(p1.X, p2.X)
					&& point.Y >= std::min(p1.Y, p2.Y)
					&& point.Y <= std::max(p1.Y, p2.Y)) {
					return true;
				}
				if (point.Y > std::min(p1.Y, p2.Y)
					&& point.Y <= std::max(p1.Y, p2.Y)
					&& point.X <= std::max(p1.X, p2.X)
					&& p1.Y != p2.Y
					&& (p1.X == p2.X || point.X <= (point.Y - p1.Y) * (p2.X - p1.X) / (p2.Y - p1.Y) + p1.X)) 
				{
					intersectCount++;
					
				}
				p1 = p2;
			}
			return intersectCount % 2 == 1;
		}

	private:

		//点集合
		std::vector<Point> _points;
	};
}



