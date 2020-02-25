#pragma once
#include <math.h>

namespace Saitama
{
	//��
	class Point
	{
	public:

		/**
		* @brief: ���캯��
		*/
		Point()
			:Point(0, 0)
		{

		}

		/**
		* @brief: ���캯��
		* @param: x ���xֵ
		* @param: y ���yֵ
		*/
		Point(int x, int y)
			:_x(x), _y(y)
		{

		}

		/**
		* @brief: ����ĵ㵽��ľ���
		* @param: p ����һ����
		* @return: ����ľ���
		*/
		double Distance(const Point& p) const
		{
			int x = _x - p._x;
			int y = _y - p._y;
			return sqrt(static_cast<double>(x)* static_cast<double>(x) + static_cast<double>(y)* static_cast<double>(y));
		}

	private:

		//���xֵ
		int _x;
		//���yֵ
		int _y;
	};

	//����
	class Rectangle
	{
	public:

		/**
		* @brief: ���캯��
		*/
		Rectangle()
			:Rectangle(0, 0, 0, 0)
		{

		}

		/**
		* @brief: ���캯��
		* @param: x �������Ͻǵ�xֵ
		* @param: y �������Ͻǵ�yֵ
		* @param: width ���εĿ��
		* @param: height ���εĸ߶�
		*/
		Rectangle(int x, int y, int width, int height)
			:_top(x, y), _width(width), _height(height)
		{

		}

		/**
		* @brief: ��ȡ�������Ͻǵĵ�
		* @return: �������Ͻǵĵ�
		*/
		const Point& Top() const
		{
			return _top;
		}

	private:

		//�������Ͻǵĵ�
		Point _top;
		//���εĿ��
		int _width;
		//���εĸ߶�
		int _height;
	};
}



