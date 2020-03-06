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
			:_x(x), _y(y)
		{

		}

		int X() const
		{
			return _x;
		}

		int Y() const
		{
			return _y;
		}

		/**
		* @brief: 是否为空值
		* @return: 返回ture表示空值
		*/
		bool Empty()
		{
			return _x == 0 && _y == 0;
		}

		/**
		* @brief: 计算改点到点的距离
		* @param: p 另外一个点
		* @return: 两点的距离
		*/
		double Distance(const Point& point) const
		{
			int x = _x - point._x;
			int y = _y - point._y;
			return sqrt(static_cast<double>(x)* static_cast<double>(x) + static_cast<double>(y)* static_cast<double>(y));
		}

	private:

		//点的x值
		int _x;
		//点的y值
		int _y;
	};

	class Line
	{
	public:
		Line()
			:Line(Point(),Point())
		{

		}

		Line(const Point& point1, const Point& point2)
			:_point1(point1),_point2(point2)
		{

		}

		const Point& Point1() const
		{
			return _point1;
		}

		const Point& Point2() const
		{
			return _point1;
		}

		Point Intersect(const Line& line)
		{
			int s02_x, s02_y, s10_x, s10_y, s32_x, s32_y, s_numer, t_numer, denom, t;
			s10_x = _point2.X() - _point1.X();
			s10_y = _point2.Y() - _point1.Y();
			s32_x = line._point2.X() - line._point1.X();
			s32_y = line._point2.Y() - line._point1.Y();

			denom = s10_x * s32_y - s32_x * s10_y;
			if (denom == 0)
				return Point(); // Collinear
			bool denomPositive = denom > 0;

			s02_x = _point1.X() - line._point1.X();
			s02_y = _point1.Y() - line._point1.Y();
			s_numer = s10_x * s02_y - s10_y * s02_x;
			if ((s_numer < 0) == denomPositive)
				return Point(); // No collision

			t_numer = s32_x * s02_y - s32_y * s02_x;
			if ((t_numer < 0) == denomPositive)
				return Point(); // No collision

			if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
				return Point(); // No collision
			// Collision detected
			t = t_numer / denom;

			return Point(_point1.X() + (t * s10_x), _point1.Y() + (t * s10_y));
		}

	private:

		Point _point1;
		Point _point2;
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
		* @param: top 矩形左上角的点
		* @param: width 矩形的宽度
		* @param: height 矩形的高度
		*/
		Rectangle(const Point& top, int width, int height)
			:_top(top), _width(width), _height(height), _hitPoint(top.X()+width/2,top.Y())
		{

		}

		/**
		* @brief: 获取矩形左上角的点
		* @return: 矩形左上角的点
		*/
		const Point& HitPoint() const
		{
			return _hitPoint;
		}

	private:

		//矩形左上角的点
		Point _top;
		//矩形的宽度
		int _width;
		//矩形的高度
		int _height;

		//碰撞测试点
		Point _hitPoint;
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
			int intersectCount= 0;
			Point p1= _points[0];
			for (int i = 1; i <= _points.size(); i++) {
				Point p2 = _points[i % _points.size()];
				if ((point.X() - p1.X()) * (p2.Y() - p1.Y()) == (p2.X() - p1.X()) * (point.Y() - p1.Y())
					&& point.X() >= std::min(p1.X(), p2.X())
					&& point.X() <= std::max(p1.X(), p2.X())
					&& point.Y() >= std::min(p1.Y(), p2.Y())
					&& point.Y() <= std::max(p1.Y(), p2.Y())) {
					return true;
				}
				if (point.Y() > std::min(p1.Y(), p2.Y())
					&& point.Y() <= std::max(p1.Y(), p2.Y())
					&& point.X() <= std::max(p1.X(), p2.X())
					&& p1.Y() != p2.Y()
					&& (p1.X() == p2.X() || point.X() <= (point.Y() - p1.Y()) * (p2.X() - p1.X()) / (p2.Y() - p1.Y()) + p1.X())) 
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



