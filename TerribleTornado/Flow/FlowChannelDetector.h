#pragma once
#include <vector>

#include "opencv2/opencv.hpp"
#include "turbojpeg.h"

#include "ChannelDetector.h"
#include "LaneDetector.h"
#include "BGR24Handler.h"
#include "JPGHandler.h"

namespace OnePunchMan
{
	//通道检测
	class FlowChannelDetector:public ChannelDetector
	{
	public:
		/**
		* @brief: 构造函数
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: mqtt mqtt
		* @param: debug 是否处于调试模式，处于调试模式则输出画线后的bmp
		*/
		FlowChannelDetector(int width, int height, MqttChannel* mqtt,bool debug=false);

		/**
		* @brief: 析构函数
		*/
		~FlowChannelDetector();

		/**
		* @brief: 获取车道是否初始化成功
		* @return 车道是否初始化成功
		*/
		bool LanesInited() const;

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
		* @brief: 处理识别数据
		* @param: item 识别数据项
		* @param: iveBuffer 图片字节流
		* @param: recognJson 识别json数据
		*/
		void HandleRecognize(const RecognItem& item, const unsigned char* iveBuffer,const std::string& recognJson);
	
	protected:
		/**
		* @brief: 供子类实现的处理检测数据
		* @param: items 检测数据项集合
		* @param: timeStamp 时间戳
		*/
		void HandleDetectCore(std::map<std::string, DetectItem> detectItems, long long timeStamp, const unsigned char* iveBuffer, long long packetIndex);

	private:
		//IO mqtt主题
		static const std::string IOTopic;
		//视频结构化mqtt主题
		static const std::string VideoStructTopic;

		/**
		* @brief: 绘制检测区域
		* @param: detectItems 检测项集合
		* @param: iveBuffer ive字节流
		* @param: packetIndex 帧序号
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, long long packetIndex);

		//车道集合同步锁
		std::mutex _laneMutex;
		//车道集合
		std::vector<LaneDetector*> _lanes;
		//车道初始化是否成功
		bool _lanesInited;
		//是否处于调试模式
		bool _debug;
		//调试用bgr字节流
		unsigned char* _debugBgrBuffer;
		//调试时写bmp
		BGR24Handler _bgrHandler;
		//调试时写jpg
		JPGHandler _jpgHandler;
	};

}

