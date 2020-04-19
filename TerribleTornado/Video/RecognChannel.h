#pragma once
#include <queue>
#include <mutex>

#include "SeemmoSDK.h"
#include "Thread.h"
#include "LaneDetector.h"
#include "JsonFormatter.h"
#include "MqttChannel.h"

namespace TerribleTornado
{
	class RecognChannel :public Saitama::ThreadObject
	{
	public:
		RecognChannel(int channelIndex, int bgrWidth, int bgrHeight, MqttChannel* mqtt,std::vector<LaneDetector*> lanes);

		void PushGuids(const std::vector<std::string>& guids);

		bool Inited();

		static const int ItemCount;

	protected:
		void StartCore();

	private:
		static const int MaxCacheCount;
		static const int SleepTime;
		static const std::string VideoStructTopic;

		void HandleRecognize(const std::string& json);

		int _inited;
		int _channelIndex;
		MqttChannel* _mqtt;

		std::mutex _laneMutex;
		std::vector<LaneDetector*> _lanes;

		std::vector<uint8_t> _recognData;
		std::vector<uint8_t*> _recognDataValue;
		std::vector<const char*> _guidValues;
		std::string _recognParam;
		std::vector<char> _result;

		std::mutex _mutex;
		std::queue<std::string> _guids;
	};
}