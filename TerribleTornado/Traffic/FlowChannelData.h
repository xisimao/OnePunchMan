#pragma once
#include <vector>
#include <string>

#include "Sqlite.h"
#include "StringEx.h"
#include "Shape.h"

namespace Saitama
{
	//车道
	class Lane
	{
	public:
		int ChannelIndex;
		std::string LaneId;
		std::string LaneName;
		int LaneIndex;
		int LaneType;
		int Direction;
		int FlowDirection;
		int Length;
		std::string IOIp;
		int IOPort;
		int IOIndex;	
		std::string DetectLine;
		std::string StopLine;
		std::string LaneLine1;
		std::string LaneLine2;

		double GetMeterPerPixel()
		{
			Line detectLine = GetLine(DetectLine);
			Line stopLine = GetLine(StopLine);
			Line laneLine1 = GetLine(LaneLine1);
			Line laneLine2 = GetLine(LaneLine2);
			if (detectLine.Empty()||
				stopLine.Empty()||
				laneLine1.Empty()||
				laneLine2.Empty())
			{
				return 0;
			}
			else
			{
				Point point1 = detectLine.Intersect(laneLine1);
				Point point2 = detectLine.Intersect(laneLine2);
				Line line1(point1, point2);
				Point point3 = stopLine.Intersect(laneLine1);
				Point point4 = stopLine.Intersect(laneLine2);
				Line line2(point3, point4);
				if (line1.Empty() || line2.Empty())
				{
					return 0;
				}
				double pixels = line1.Middle().Distance(line2.Middle());
				return Length / pixels;
			}
		}

		Polygon GetRegion()
		{
			Line detectLine = GetLine(DetectLine);
			Line stopLine = GetLine(StopLine);
			Line laneLine1 = GetLine(LaneLine1);
			Line laneLine2 = GetLine(LaneLine2);
			if (detectLine.Empty() ||
				stopLine.Empty() ||
				laneLine1.Empty() ||
				laneLine2.Empty())
			{
				return Polygon();
			}
			else
			{
				Point point1 = detectLine.Intersect(laneLine1);
				Point point2 = detectLine.Intersect(laneLine2);
				Point point3 = stopLine.Intersect(laneLine1);
				Point point4 = stopLine.Intersect(laneLine2);
				if (point1.Empty()||
					point2.Empty()||
					point3.Empty()||
					point4.Empty())
				{
					return Polygon();
				}
				std::vector<Point> points;
				points.push_back(point1);
				points.push_back(point2);
				points.push_back(point3);
				points.push_back(point4);
				return Polygon(points);				
			}
		}

	private:
		Line GetLine(std::string line)
		{
			if (line.size() > 2)
			{
				std::vector<Point> points;
				std::vector<std::string> coordinates = StringEx::Split(line.substr(1, line.size() - 2), ",", true);
				int x, y = 0;
				for (int i = 0; i < coordinates.size(); ++i)
				{
					if (coordinates[i].size() > 2)
					{
						if (i % 2 == 0)
						{
							x=StringEx::Convert<int>(coordinates[i].substr(1, coordinates[i].size() - 1));
						}
						else
						{
							y = StringEx::Convert<int>(coordinates[i].substr(0, coordinates[i].size() - 1));
							points.push_back(Point(x, y));
						}
					}
				}
				if (points.size() >= 2)
				{
					return Line(points[0], points[1]);
				}
			}
			return Line();
		}
	};

	//流量视频通道
	class FlowChannel
	{
	public:
		int ChannelIndex;
		std::string ChannelName;
		std::string ChannelUrl;
		int ChannelType;
		std::string RtspUser;
		std::string RtspPassword;
		int RtspProtocol;
		bool IsLoop;

		std::vector<Lane> Lanes;
	};

	//流量视频通道数据库操作
	class FlowChannelData
	{
	public:

		std::vector<FlowChannel> GetList();

		FlowChannel Get(int channelIndex);

		bool Set(const FlowChannel& channel);

		void SetList(const std::vector<FlowChannel>& channels);

		int Delete(int channelIndex);

	private:

		FlowChannel GetChannel(const SqliteReader& sqlite);

		bool Insert(const FlowChannel& channel);

		SqliteWriter _sqlite;
	};
}


