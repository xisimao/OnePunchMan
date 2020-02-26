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
		* @brief: 计算改点到点的距离
		* @param: p 另外一个点
		* @return: 两点的距离
		*/
		double Distance(const Point& p) const
		{
			int x = _x - p._x;
			int y = _y - p._y;
			return sqrt(static_cast<double>(x)* static_cast<double>(x) + static_cast<double>(y)* static_cast<double>(y));
		}

	private:

		//点的x值
		int _x;
		//点的y值
		int _y;
	};

	//矩形
	class Rectangle
	{
	public:

		/**
		* @brief: 构造函数
		*/
		Rectangle()
			:Rectangle(0, 0, 0, 0)
		{

		}

		/**
		* @brief: 构造函数
		* @param: x 矩形左上角的x值
		* @param: y 矩形左上角的y值
		* @param: width 矩形的宽度
		* @param: height 矩形的高度
		*/
		Rectangle(int x, int y, int width, int height)
			:_top(x, y), _width(width), _height(height), _hitPoint(x+width/2,y)
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



