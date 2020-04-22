#pragma once
#include <queue>
#include <mutex>

#include "SeemmoSDK.h"
#include "Thread.h"
#include "ChannelDetector.h"

namespace TerribleTornado
{
	//识别线程
	class RecognChannel :public Saitama::ThreadObject
	{
	public:
		/**
		* @brief: 构造函数
		* @param: recognIndex 检测线程序号
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: detectors 通道检测集合
		*/
		RecognChannel(int recognIndex,int width, int height, const std::vector<ChannelDetector*>& detectors);
	
		/**
		* @brief: 析构函数
		*/
		~RecognChannel();

		/**
		* @brief: 推送guid集合
		* @param: channelIndex 通道序号
		* @param: guids guid集合
		*/
		void PushGuids(int channelIndex,const std::vector<std::string>& guids);
		
		/**
		* @brief: 获取是否初始化完成
		* @return: 初始化完成返回ture，否则返回false
		*/
		bool Inited();

		//表示一个识别线程可以接收的通道数量
		static const int ItemCount;

	protected:
		void StartCore();

	private:
		//guid数据项
		class GuidItem
		{
		public:
			//通道序号
			int ChannelIndex;
			//guid
			std::string Guid;
		};

		//最大缓存数量
		static const int MaxCacheCount;
		//线程休眠时间(ms)
		static const int SleepTime;

		//是否初始化完成
		int _inited;
		//检测线程序号
		int _recognIndex;
		//通道检测集合
		std::vector<ChannelDetector*> _detectors;
		//guid数据项集合同步锁
		std::mutex _mutex;
		//guid数据项集合
		std::queue<GuidItem> _guidsCache;

		//recogn
		std::vector<uint8_t*> _bgrs;
		std::vector<const char*> _guids;
		std::string _param;
		std::vector<char> _result;

	};
}