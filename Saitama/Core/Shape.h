#pragma once
#include <math.h>

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
			:_top(x, y), _width(width), _height(height)
		{

		}

		/**
		* @brief: 获取矩形左上角的点
		* @return: 矩形左上角的点
		*/
		const Point& Top() const
		{
			return _top;
		}

	private:

		//矩形左上角的点
		Point _top;
		//矩形的宽度
		int _width;
		//矩形的高度
		int _height;
	};
}



