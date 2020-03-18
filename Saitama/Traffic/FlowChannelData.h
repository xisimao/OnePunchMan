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
		std::string Region;
	};

	//������Ƶͨ��
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

	//������Ƶͨ�����ݿ����
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


