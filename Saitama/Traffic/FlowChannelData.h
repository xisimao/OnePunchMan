#pragma once
#include <vector>
#include <string>

#include "Sqlite.h"
#include "StringEx.h"

namespace Saitama
{
	//����
	class Lane
	{
	public:
		std::string ChannelId;
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
	};

	//������Ƶͨ��
	class FlowChannel
	{
	public:

		std::string ChannelId;
		std::string ChannelName;
		int ChannelIndex;
		int ChannelType;
		std::string RtspUser;
		std::string RtspPassword;
		int RtspProtocol;
		bool IsLoop;

		std::vector<Lane> Lanes;
	};

	//������Ƶͨ�����ݿ����
	class FlowChannelData
	{
	public:

		std::vector<FlowChannel> GetList();

		FlowChannel Get(const std::string& channelId);

		bool Set(const FlowChannel& channel);

		int Delete(const std::string& channelId);

	private:

		FlowChannel GetChannel(const SqliteReader& sqlite);

		SqliteWriter _sqlite;
	};
}


