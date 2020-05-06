#pragma once
#include <vector>

#include "opencv2/opencv.hpp"
#include "turbojpeg.h"

#include "SeemmoSDK.h"
#include "FFmpegChannel.h"
#include "JsonFormatter.h"
#include "MqttChannel.h"
#include "TrafficData.h"

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
		*/
		ChannelDetector(int width, int height, MqttChannel* mqtt);

		/**
		* @brief: 析构函数
		*/
		virtual ~ChannelDetector();

		/**
		* @brief: 获取通道地址
		* @return 通道地址
		*/
		std::string ChannelUrl() const;

		/**
		* @brief: 处理检测数据
		* @param: detectJson 检测json数据
		* @param: params 检测参数
		* @param: iveBuffer 图片字节流
		* @param: packetIndex 帧序号
		* @return: 识别数据集合
		*/
		std::vector<RecognItem> HandleDetect(const std::string& detectJson, std::string* param, const unsigned char* iveBuffer, long long packetIndex);

		/**
		* @brief: 处理识别数据
		* @param: item 识别数据项
		* @param: iveBuffer 图片字节流
		* @param: recognJson 识别json数据
		*/
		virtual void HandleRecognize(const RecognItem& item, const unsigned char* iveBuffer, const std::string& recognJson)=0;

	protected:
		/**
		* @brief: 供子类实现的处理检测数据
		* @param: items 检测数据项集合
		* @param: timeStamp 时间戳
		*/
		virtual void HandleDetectCore(std::map<std::string, DetectItem> detectItems, long long timeStamp, const unsigned char* iveBuffer, long long packetIndex)=0;


		/**
		* @brief: hisi ive_8uc3转bgr
		* @param: iveBuffer ive字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: bgrBuffer 写入的bgr字节流
		*/
		void IveToBgr(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer);
		
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
		//车道算法筛选区域参数
		std::string _param;
		//是否设置过车道参数
		bool _setParam;
		//bgr字节流
		unsigned char* _bgrBuffer;

	private:
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
	};

}

