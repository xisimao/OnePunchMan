#pragma once
#include <algorithm>
#include <math.h>
#include "StringEx.h"
#include "JsonFormatter.h"

namespace OnePunchMan
{
	//点
	class Point
	{
	public:

		/**
		* 构造函数
		*/
		Point()
			:Point(0, 0)
		{

		}

		/**
		* 构造函数
		* @param x 点的x值
		* @param y 点的y值
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
		* 是否为空值
		* @return 返回ture表示空值
		*/
		bool Empty()
		{
			return X == 0 && Y == 0;
		}

		/**
		* 计算改点到点的距离
		* @param point 另外一个点
		* @return 两点的距离
		*/
		double Distance(const Point& point) const
		{
			int x = X - point.X;
			int y = Y - point.Y;
			return sqrt(static_cast<double>(x)* static_cast<double>(x) + static_cast<double>(y)* static_cast<double>(y));
		}

		/**
		* 获取点的json数据
		* @return 点的json数据
		*/
		std::string ToJson() const
		{
			return StringEx::Combine("[", X, ",", Y, "]");
		}
	};

	//线段
	class Line
	{
	public:

		/**
		* 构造函数
		*/
		Line()
			:Line(Point(),Point())
		{

		}

		/**
		* 构造函数
		* @param point1 第一个点
		* @param point2 第二个点
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
		* 是否为空值
		* @return 返回ture表示空值
		*/
		bool Empty()
		{
			return Point1.Empty() && Point2.Empty();
		}

		/**
		* 计算和另外一条线的相交点
		* @param line 另外一条线
		* @return 相交点
		*/
		Point Intersect(const Line& line)
		{
			int x1 = Point1.X, x2 = Point2.X, x3 = line.Point1.X, x4 = line.Point2.X;
			int y1 = Point1.Y, y2 = Point2.Y, y3 = line.Point1.Y, y4 = line.Point2.Y;

			float d = static_cast<float>((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
			// If d is zero, there is no intersection
			if (d == 0) return Point();

			// Get the x and y
			int pre = (x1 * y2 - y1 * x2), post = (x3 * y4 - y3 * x4);
			int x = static_cast<int>(round(static_cast<float>(pre * (x3 - x4) - (x1 - x2) * post) / d));
			int y = static_cast<int>(round(static_cast<float>(pre * (y3 - y4) - (y1 - y2) * post) / d));

			// Check if the x and y coordinates are within both lines
			if (x < (std::min)(x1, x2) || x > (std::max)(x1, x2) ||
				x < (std::min)(x3, x4) || x > (std::max)(x3, x4)) return Point();
			if (y < (std::min)(y1, y2) || y > (std::max)(y1, y2) ||
				y < (std::min)(y3, y4) || y > (std::max)(y3, y4)) return Point();

			return Point(x, y);
		}

		/**
		* 计算线段的中点
		* @return 中点
		*/
		Point Middle()
		{
			return Point((Point1.X + Point2.X) / 2, (Point1.Y + Point2.Y) / 2);
		}

		/**
		* json转线段
		* @param json 线段json数据[[1,1],[2,2]]
		* @return 转换成功返回线段,否则返回空线段
		*/
		static Line FromJson(const std::string& json)
		{
			std::vector<Point> points;
			std::vector<std::string> pointJsons = JsonDeserialization::ConvertToItems(json);
			for (unsigned int j = 0; j < pointJsons.size(); ++j)
			{
				std::vector<int> coordinates = JsonDeserialization::ConvertToArray<int>(pointJsons[j]);
				if (coordinates.size() == 2)
				{
					points.push_back(Point(coordinates[0], coordinates[1]));
				}
			}
			if (points.size() == 2)
			{
				return Line(points[0],points[1]);
			}
			else
			{
				return Line();
			}
		}

	};

	//折线
	class BrokeLine
	{
	public:

		/**
		* 构造函数
		*/
		BrokeLine()
			:BrokeLine(std::vector<Point>())
		{

		}

		/**
		* 构造函数
		* @param points 点集合
		*/
		BrokeLine(const std::vector<Point>& points)
			:_points(points)
		{

		}

		/**
		* 添加点
		* @param point 点
		*/
		void AddPoint(const Point& point)
		{
			_points.push_back(point);
		}

		/**
		* 获取多边形的点集合
		* @return 多边形的点集合
		*/
		const std::vector<Point>& Points() const
		{
			return _points;
		}

		/**
		* 是否为空值
		* @return 返回ture表示空值
		*/
		bool Empty()
		{
			return _points.empty();
		}

		/**
		* 计算和另外一条线的相交点
		* @param line 另外一条线
		* @return 相交点
		*/
		Point Intersect(const Line& line)
		{
			for (unsigned int i = 1; i < Points().size(); ++i)
			{
				Line tempLine(Points()[i - 1], Points()[i]);
				Point intersectPoint = tempLine.Intersect(line);
				if (!intersectPoint.Empty())
				{
					return intersectPoint;
				}
			}
			return Point();
		}

		/**
		* 获取多边形的json数据
		* @return 多边形的json数据
		*/
		std::string ToJson() const
		{
			std::string json;
			json.append("[");
			for (unsigned int i = 0; i < _points.size(); ++i)
			{
				json.append(_points[i].ToJson());
				json.append(",");
			}
			if (json.size() == 1)
			{
				json.append("]");
			}
			else
			{
				json[json.size() - 1] = ']';
			}
			return json;
		}

		/**
		* json转多边形
		* @param json 多边形json数据[[1,1],[2,2],[3,3],[4,4],[5,5]]
		* @return 转换成功返回多边形,否则返回空
		*/
		static BrokeLine FromJson(const std::string& json)
		{
			std::vector<Point> points;
			std::vector<std::string> pointJsons = JsonDeserialization::ConvertToItems(json);
			for (unsigned int j = 0; j < pointJsons.size(); ++j)
			{
				std::vector<int> coordinates = JsonDeserialization::ConvertToArray<int>(pointJsons[j]);
				if (coordinates.size() == 2)
				{
					points.push_back(Point(coordinates[0], coordinates[1]));
				}
			}
			return BrokeLine(points);
		}


	private:

		//点集合
		std::vector<Point> _points;
	};

	//矩形
	class Rectangle
	{
	public:

		/**
		* 构造函数
		*/
		Rectangle()
			:Rectangle(Point(), 0, 0)
		{

		}

		/**
		* 构造函数
		* @param x 矩形左上角点的x值
		* @param y 矩形左上角点的y值
		* @param width 矩形的宽度
		* @param height 矩形的高度
		*/
		Rectangle(int x,int y, int width, int height)
			:Rectangle(Point(x,y), width,height)
		{

		}

		/**
		* 构造函数
		* @param top 矩形左上角的点
		* @param width 矩形的宽度
		* @param height 矩形的高度
		*/
		Rectangle(const Point& top, int width, int height)
			:_top(top),_width(width),_height(height), _hitPoint(top.X + width / 2, top.Y + height)
		{
			_points.push_back(top);
			_points.push_back(Point(top.X+width,top.Y));
			_points.push_back(Point(top.X+width,top.Y+height));
			_points.push_back(Point(top.X,top.Y+height));
		}

		/**
		* 获取矩形宽度
		* @return 矩形宽度
		*/
		Point Top() const
		{
			return _top;
		}

		/**
		* 获取矩形宽度
		* @return 矩形宽度
		*/
		int Width() const
		{
			return _width;
		}

		/**
		* 获取矩形高度
		* @return 矩形高度
		*/
		int Height() const
		{
			return _height;
		}

		/**
		* 获取检测点
		* @return 检测点
		*/
		const Point& HitPoint() const
		{
			return _hitPoint;
		}

		/**
		* 获取矩形的点集合
		* @return 矩形的点集合
		*/
		const std::vector<Point>& Points() const
		{
			return _points;
		}

	private:
		//矩形顶点
		Point _top;
		//矩形宽度
		int _width;
		//矩形高度
		int _height;
		//测试点
		Point _hitPoint;
		//点集合
		std::vector<Point> _points;
	};

	//多边形
	class Polygon
	{
	public:

		/**
		* 构造函数
		*/
		Polygon()
			:Polygon(std::vector<Point>())
		{

		}

		/**
		* 构造函数
		* @param points 点集合
		*/
		Polygon(const std::vector<Point>& points)
			:_points(points)
		{
			
		}

		/**
		* 获取多边形的点集合
		* @return 多边形的点集合
		*/
		const std::vector<Point>& Points() const
		{
			return _points;
		}

		/**
		* 是否为空值
		* @return 返回ture表示空值
		*/
		bool Empty()
		{
			return _points.empty();
		}

		/**
		* 测试点是否包含在多边形
		* @param point 点
		* @return 点在多边形内返回true,否则返回false
		*/
		bool Contains(const Point& point)
		{
			if (_points.empty())
			{
				return false;
			}
			int intersectCount= 0;
			Point p1= _points[0];
			for (unsigned int i = 1; i <= _points.size(); i++) {
				Point p2 = _points[i % _points.size()];
				if ((point.X - p1.X) * (p2.Y - p1.Y) == (p2.X - p1.X) * (point.Y - p1.Y)
					&& point.X >= (std::min)(p1.X, p2.X)
					&& point.X <= (std::max)(p1.X, p2.X)
					&& point.Y >= (std::min)(p1.Y, p2.Y)
					&& point.Y <= (std::max)(p1.Y, p2.Y)) {
					return true;
				}
				if (point.Y > (std::min)(p1.Y, p2.Y)
					&& point.Y <= (std::max)(p1.Y, p2.Y)
					&& point.X <= (std::max)(p1.X, p2.X)
					&& p1.Y != p2.Y
					&& (p1.X == p2.X || point.X <= (point.Y - p1.Y) * (p2.X - p1.X) / (p2.Y - p1.Y) + p1.X)) 
				{
					intersectCount++;
					
				}
				p1 = p2;
			}
			return intersectCount % 2 == 1;
		}

		static Polygon Build(const BrokeLine& brokeLines1, const BrokeLine& brokeLines2, const Line& line1, const Line& line2)
		{
			std::vector<Point> points;
			Point point1, point2, point3, point4;
			int index1 = 0, index2 = 0, index3 = 0, index4 = 0;

			bool hasIntersect=false;
			for (unsigned int i = 1; i < brokeLines1.Points().size(); ++i)
			{
				Line tempLine(brokeLines1.Points()[i - 1], brokeLines1.Points()[i]);
				Point intersectPoint = tempLine.Intersect(line1);
				if (!intersectPoint.Empty())
				{
					point1 = intersectPoint;
					index1 = i;
					points.push_back(intersectPoint);
					hasIntersect = true;
					break;
				}
			}
			if (!hasIntersect)
			{
				return Polygon();
			}

			hasIntersect = false;
			for (unsigned int i = 1; i < brokeLines1.Points().size(); ++i)
			{
				Line tempLine(brokeLines1.Points()[i - 1], brokeLines1.Points()[i]);
				Point intersectPoint = tempLine.Intersect(line2);
				if (!intersectPoint.Empty())
				{
					point2 = intersectPoint;
					index2 = i;
					if (index2 > index1)
					{
						for (int j = index1; j< index2; ++j)
						{
							points.push_back((brokeLines1.Points()[j]));
						}
					}
					else
					{
						for (int j = index1-1; j >= index2; --j)
						{
							points.push_back((brokeLines1.Points()[j]));
						}
					}
					points.push_back(intersectPoint);
					hasIntersect = true;
					break;
				}
			}
			if (!hasIntersect)
			{
				return Polygon();
			}

			hasIntersect = false;
			for (unsigned int i = 1; i < brokeLines2.Points().size(); ++i)
			{
				Line tempLine(brokeLines2.Points()[i - 1], brokeLines2.Points()[i]);
				Point intersectPoint = tempLine.Intersect(line2);
				if (!intersectPoint.Empty())
				{
					point3 = intersectPoint;
					index3 = i;
					points.push_back(intersectPoint);
					hasIntersect = true;
					break;
				}
			}
			if (!hasIntersect)
			{
				return Polygon();
			}

			hasIntersect = false;
			for (unsigned int i = 1; i < brokeLines2.Points().size(); ++i)
			{
				Line tempLine(brokeLines2.Points()[i - 1], brokeLines2.Points()[i]);
				Point intersectPoint = tempLine.Intersect(line1);
				if (!intersectPoint.Empty())
				{
					point4 = intersectPoint;
					index4 = i;
					if (index4 > index3)
					{
						for (int j = index3; j < index4; ++j)
						{
							points.push_back((brokeLines2.Points()[j]));
						}
					}
					else
					{
						for (int j = index3 - 1; j >= index4; --j)
						{
							points.push_back((brokeLines2.Points()[j]));
						}
					}
					points.push_back(intersectPoint);
					hasIntersect = true;
					break;
				}
			}
			if (!hasIntersect)
			{
				return Polygon();
			}
			else
			{
				return Polygon(points);
			}
		}
		
		/**
		* 获取多边形的json数据
		* @return 多边形的json数据
		*/
		std::string ToJson() const
		{
			std::string json;
			json.append("[");
			for (unsigned int i = 0; i < _points.size(); ++i)
			{
				json.append(_points[i].ToJson());
				json.append(",");
			}
			if (json.size() == 1)
			{
				json.append("]");
			}
			else
			{
				json[json.size() - 1] = ']';
			}
			return json;
		}

		/**
		* json转多边形
		* @param json 多边形json数据[[1,1],[2,2],[3,3],[4,4],[5,5]]
		* @return 转换成功返回多边形,否则返回空
		*/
		static Polygon FromJson(const std::string& json)
		{
			std::vector<Point> points;
			std::vector<std::string> pointJsons = JsonDeserialization::ConvertToItems(json);
			for (unsigned int j = 0; j < pointJsons.size(); ++j)
			{
				std::vector<int> coordinates = JsonDeserialization::ConvertToArray<int>(pointJsons[j]);
				if (coordinates.size() == 2)
				{
					points.push_back(Point(coordinates[0], coordinates[1]));
				}
			}
			return Polygon(points);
		}

		/**
		* json转多边形集合
		* @param json 多边形json数据[[[1,1],[2,2],[3,3],[4,4],[5,5]],[[1,1],[2,2],[3,3],[4,4],[5,5]]]
		* @return 转换成功返回多边形集合,否则返回空集合
		*/
		static std::vector<Polygon> FromJsonArray(const std::string& json)
		{
			std::vector<Polygon> polygons;
			std::vector<std::string> polygonJsons = JsonDeserialization::ConvertToItems(json);
			for (unsigned int i = 0; i < polygonJsons.size(); ++i)
			{
				polygons.push_back(FromJson(polygonJsons[i]));
			}
			return polygons;
		}

	private:

		//点集合
		std::vector<Point> _points;
	};
}



