#pragma once
#include <vector>

#include "opencv2/opencv.hpp"
#include "turbojpeg.h"

#include "ChannelDetector.h"
#include "EventDetector.h"

namespace OnePunchMan
{
	//通道检测
	class EventChannelDetector:public ChannelDetector
	{
	public:
		/**
		* @brief: 构造函数
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: mqtt mqtt
		*/
		EventChannelDetector(int width, int height, MqttChannel* mqtt);

		/**
		* @brief: 析构函数
		*/
		~EventChannelDetector();

		/**
		* @brief: 更新通道
		* @param: channel 通道
		*/
		void UpdateChannel(const EventChannel& channel);

		/**
		* @brief: 清空通道
		*/
		void ClearChannel();

	protected:
		/**
		* @brief: 供子类实现的处理检测数据
		* @param: items 检测数据项集合
		* @param: timeStamp 时间戳
		*/
		void HandleDetectCore(std::map<std::string, DetectItem> detectItems,long long timeStamp);

	private:
		//事件 mqtt主题
		static const std::string EventTopic;

		//车道集合同步锁
		std::mutex _laneMutex;
		//车道集合
		std::vector<EventDetector*> _lanes;
		
	};

}

