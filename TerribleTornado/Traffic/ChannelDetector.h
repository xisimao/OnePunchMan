#pragma once
#include <vector>

#include "opencv2/opencv.hpp"
#include "turbojpeg.h"

#include "SeemmoSDK.h"
#include "FFmpegChannel.h"
#include "JsonFormatter.h"
#include "LaneDetector.h"
#include "MqttChannel.h"
#include "BGR24Handler.h"
#include "JPGHandler.h"

namespace OnePunchMan
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
		* @param: debug 是否处于调试模式，处于调试模式则输出画线后的bmp
		*/
		ChannelDetector(int width, int height, MqttChannel* mqtt,bool debug=false);

		/**
		* @brief: 析构函数
		*/
		~ChannelDetector();

		/**
		* @brief: 获取通道地址
		* @return 通道地址
		*/
		std::string ChannelUrl() const;

		/**
		* @brief: 获取车道是否初始化成功
		* @return 车道是否初始化成功
		*/
		bool Inited() const;

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
		* @param: detectJson 检测json数据
		* @param: params 检测参数
		* @param: iveBuffer 图片字节流
		* @param: packetIndex 帧序号
		* @return: 识别数据集合
		*/
		std::vector<RecognItem> HandleDetect(const std::string& detectJson, std::string* param, const unsigned char* iveBuffer,long long packetIndex);
		
		/**
		* @brief: 处理识别数据
		* @param: item 识别数据项
		* @param: iveBuffer 图片字节流
		* @param: recognJson 识别json数据
		*/
		void HandleRecognize(const RecognItem& item, const unsigned char* iveBuffer,const std::string& recognJson);

	private:
		//轮询中睡眠时间(毫秒)
		static const int SleepTime;

		//IO mqtt主题
		static const std::string IOTopic;
		//视频结构化mqtt主题
		static const std::string VideoStructTopic;

		/**
		* @brief: 从json数据中获取检测项集合
		* @param: items 识别项集合
		* @param: jd json解析
		* @param: key 检测类型，机动车，非机动和和行人
		*/
		void GetRecognItems(std::vector<RecognItem>* items, const JsonDeserialization& jd, const std::string& key);

		/**
		* @brief: 从json数据中获取检测项集合
		* @param: items 检测项
		* @param: jd json解析
		* @param: key 检测类型，机动车，非机动和和行人
		*/
		void GetDetecItems(std::map<std::string, DetectItem>* items, const JsonDeserialization& jd, const std::string& key);

		/**
		* @brief: hisi ive_8uc3转bgr
		* @param: iveBuffer ive字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: bgrBuffer 写入的bgr字节流
		*/
		void IveToBgr(const unsigned char* iveBuffer,int width,int height,unsigned char* bgrBuffer);
		
		/**
		* @brief: 绘制检测区域
		* @param: detectItems 检测项集合
		* @param: iveBuffer ive字节流
		* @param: packetIndex 帧序号
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, long long packetIndex);

		//视频序号
		int _channelIndex;
		//视频地址
		std::string _channelUrl;
		//视频宽度
		int _width;
		//视频高度
		int _height;
		//mqtt
		MqttChannel* _mqtt;
		//车道集合同步锁
		std::mutex _laneMutex;
		//车道集合
		std::vector<LaneDetector*> _lanes;
		//车道初始化是否成功
		bool _inited;
		//车道算法筛选区域参数
		std::string _param;
		//是否设置过车道参数
		bool _setParam;
		//bgr字节流
		unsigned char* _bgrBuffer;
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

