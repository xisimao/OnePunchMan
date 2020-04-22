#pragma once
#include <vector>

#include "SeemmoSDK.h"
#include "FrameChannel.h"
#include "JsonFormatter.h"
#include "LaneDetector.h"
#include "MqttChannel.h"

namespace TerribleTornado
{
	//通道检测
	class ChannelDetector
	{
	public:
		/**
		* @brief: 构造函数
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: mqtt mqtt
		*/
		ChannelDetector(int width, int height, MqttChannel* mqtt);

		/**
		* @brief: 更新通道
		* @param: channel 通道
		*/
		void UpdateChannel(const FlowChannel& channel);
		
		/**
		* @brief: 清空通道
		*/
		void ClearChannel();

		/**
		* @brief: 收集流量
		* @param: flowJson 流量json数据
		* @param: timeStamp 时间戳
		* @return: 流量json数据
		*/
		void CollectFlow(std::string* flowJson, long long timeStamp);

		/**
		* @brief: 处理检测数据
		* @param: detectJson 检测结构
		* @param: timeStamp 时间戳
		* @return: io json数据
		*/
		std::vector<std::string> HandleDetect(const std::string& detectJson, long long timeStamp);
		
		/**
		* @brief: 处理识别数据
		* @param: recognJson 识别数据
		*/
		void HandleRecognize(const std::string& recognJson);

	private:
		//轮询中睡眠时间(毫秒)
		static const int SleepTime;

		//IO mqtt主题
		static const std::string IOTopic;
		//视频结构化mqtt主题
		static const std::string VideoStructTopic;

		/**
		* @brief: 从json数据中获取检测项集合
		* @param: guids guid集合
		* @param: jd json解析
		* @param: key 检测类型，机动车，非机动和和行人
		*/
		static void GetRecognGuids(std::vector<std::string>* guids, const Saitama::JsonDeserialization& jd, const std::string& key);

		/**
		* @brief: 从json数据中获取检测项集合
		* @param: items 检测项
		* @param: jd json解析
		* @param: key 检测类型，机动车，非机动和和行人
		*/
		static void GetDetecItems(std::map<std::string, DetectItem>* items, const Saitama::JsonDeserialization& jd, const std::string& key);

		//视频地址
		std::string _channelUrl;
		//mqtt
		MqttChannel* _mqtt;
		//车道集合同步锁
		std::mutex _laneMutex;
		//车道集合
		std::vector<LaneDetector*> _lanes;
	};

}

