#pragma once
#include "Seemmo_SDK.h"
#include "Seemmo_DecodeChannel.h"
#include "Seemmo_RecognChannel.h"
#include "FlowDetector.h"
#include "EventDetector.h"

namespace OnePunchMan
{
	//检测线程
	class Seemmo_DetectChannel :public ThreadObject
	{
	public:
		/**
		* 构造函数
		* @param detectIndex 检测序号
		* @param width 视频解码后宽度
		* @param height 视频解码后高度
		*/
		Seemmo_DetectChannel(int detectIndex,int width, int height);

		/**
		* 当前检测线程是否未初始化
		* @return 如果未初始化或者当前线程正在进行检测返回true,否则返回false
		*/
		bool Inited();

		/**
		* 添加需要检测的通道序号
		* @param channelIndex 通道序号
		*/
		void SetRecogn(Seemmo_RecognChannel* recogn);

		/**
		* 添加需要检测的通道序号
		* @param channelIndex 通道序号
		* @param decode 解码线程
		* @param flowDetector 流量检测
		* @param eventDetector 事件检测
		*/
		void AddChannel(int channelIndex,Seemmo_DecodeChannel* decode, FlowDetector* flowDetector,EventDetector* eventDetector);

		/**
		* 更新通道
		* @param channel 通道
		*/
		void UpdateChannel(const TrafficChannel& channel);

		/**
		* 将下一帧视频写入到bmp
		* * @param channelIndex 通道序号
		*/
		void Screenshot(int channelIndex);

	protected:
		void StartCore();

	private:
		//视频帧数据
		class ChannelItem
		{
		public:
			ChannelItem()
				: ChannelIndex(0), Param(), WriteBmp(false), Decode(NULL), Flow(NULL), Event(NULL), OutputDetect(false), OutputImage(false)
			{

			}
			//通道序号
			int ChannelIndex;
			//车道参数
			std::string Param;
			//是否需要截取bmp图片
			bool WriteBmp;
			//解码
			Seemmo_DecodeChannel* Decode;
			//流量检测
			FlowDetector* Flow;
			//事件检测
			EventDetector* Event;
			//是否输出检测数据
			bool OutputDetect;
			//是否输出图片
			bool OutputImage;
		};

		/**
		* 从json数据中获取检测项集合
		* @param items 检测项集合
		* @param jd json解析
		* @param key 检测类型,机动车,非机动和和行人
		*/
		void GetDetecItems(std::map<std::string, DetectItem>* items, const JsonDeserialization& jd, const std::string& key);

		/**
		* 从json数据中获取识别项集合
		* @param items 识别项集合
		* @param jd json解析
		* @param key 检测类型,机动车,非机动和和行人
		* @param channelIndex 通道序号
		* @param frameIndex 帧序号
		* @param frameSpan 帧间隔时间(毫秒)
		* @param taskId 任务号
		*/
		void GetRecognItems(std::vector<RecognItem>* items, const JsonDeserialization& jd, const std::string& key,int channelIndex, int frameIndex,unsigned char frameSpan,unsigned char taskId);

		//轮询中睡眠时间(毫秒)
		static const int SleepTime;

		//线程是否初始化完成
		bool _inited;
		//检测序号
		int _detectIndex;
		//图片宽度
		int _width;
		//图片高度
		int _height;

		//视频检测数据同步锁
		std::mutex _channelMutex;
		//视频帧数据
		std::map<int,ChannelItem> _channelItems;
		//检测线程
		Seemmo_RecognChannel* _recogn;
		//图像转换
		ImageConvert _image;

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

