#pragma once
#include "SeemmoSDK.h"
#include "MqttChannel.h"
#include "DecodeChannel.h"
#include "RecognChannel.h"
#include "TrafficDetector.h"

namespace OnePunchMan
{
	//检测线程
	class DetectChannel :public ThreadObject
	{
	public:
		/**
		* @brief: 构造函数
		* @param: detectIndex 检测序号
		* @param: width 视频解码后宽度
		* @param: height 视频解码后高度
		*/
		DetectChannel(int detectIndex,int width, int height);

		/**
		* @brief: 当前检测线程是否未初始化
		* @return: 如果未初始化或者当前线程正在进行检测返回true，否则返回false
		*/
		bool Inited();

		/**
		* @brief: 添加需要检测的通道序号
		* @param: channelIndex 通道序号
		*/
		void SetRecogn(RecognChannel* recogn);

		/**
		* @brief: 添加需要检测的通道序号
		* @param: channelIndex 通道序号
		*/
		void AddChannel(int channelIndex,DecodeChannel* decode,TrafficDetector* detector);

	protected:
		void StartCore();

	private:

		//视频帧数据
		class ChannelItem
		{
		public:
			ChannelItem()
				: ChannelIndex(0), Param(), Decode(NULL),Detector(NULL)
			{

			}
			//通道序号
			int ChannelIndex;
			//车道参数
			std::string Param;
			//解码
			DecodeChannel* Decode;
			//检测
			TrafficDetector* Detector;
		};

	private:
		/**
		* @brief: 从json数据中获取检测项集合
		* @param: items 检测项集合
		* @param: jd json解析
		* @param: key 检测类型，机动车，非机动和和行人
		*/
		void GetDetecItems(std::map<std::string, DetectItem>* items, const JsonDeserialization& jd, const std::string& key);

		/**
		* @brief: 从json数据中获取识别项集合
		* @param: items 识别项集合
		* @param: jd json解析
		* @param: key 检测类型，机动车，非机动和和行人
		* @param: channelIndex 通道序号
		*/
		void GetRecognItems(std::vector<RecognItem>* items, const JsonDeserialization& jd, const std::string& key,int channelIndex);

		//轮询中睡眠时间(毫秒)
		static const int SleepTime;
		//检测主题
		static const std::string DetectTopic;

		//线程是否初始化完成
		bool _inited;
		//检测序号
		int _detectIndex;
		//图片宽度
		int _width;
		//图片高度
		int _height;
		//视频帧数据
		std::vector<ChannelItem> _channelItems;
		//检测线程
		RecognChannel* _recogn;

		//detect
		std::vector<uint8_t*> _ives;
		std::vector<int> _indexes;
		std::vector<uint64_t> _timeStamps;
		std::vector<uint32_t> _widths;
		std::vector<uint32_t> _heights;
		std::vector<const char*> _params;
		std::vector<char> _result;

	};

}

