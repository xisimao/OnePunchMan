#pragma once
#include <queue>
#include <mutex>

#include "Seemmo_SDK.h"
#include "Thread.h"
#include "FlowDetector.h"

namespace OnePunchMan
{
	//识别线程
	class Seemmo_RecognChannel :public ThreadObject
	{
	public:
		/**
		* 构造函数
		* @param recognIndex 检测线程序号
		* @param width 图片宽度
		* @param height 图片高度
		* @param detectors 通道检测集合
		*/
		Seemmo_RecognChannel(int recognIndex,int width, int height, std::vector<FlowDetector*>* detectors);

		/**
		* 析构函数
		*/
		~Seemmo_RecognChannel();
		/**
		* 推送识别数据项集合
		* @param items 识别数据项集合
		*/
		void PushItems(const std::vector<RecognItem>& items);
		
		/**
		* 获取是否初始化完成
		* @return 初始化完成返回ture,否则返回false
		*/
		bool Inited();

		/**
		* 获取当前队列中的数据数量
		* @return 当前队列中的数据数量
		*/
		int Size();

	protected:
		void StartCore();

	private:
		//最大缓存数量
		static const int MaxCacheCount;
		//线程休眠时间(ms)
		static const int SleepTime;
		//识别主题
		static const std::string RecognTopic;

		//是否初始化完成
		int _inited;
		//检测线程序号
		int _recognIndex;
		//通道检测集合
		std::vector<FlowDetector*>* _detectors;
		//guid数据项集合同步锁
		std::mutex _queueMutex;
		//guid数据项集合
		std::queue<RecognItem> _items;

		//recogn
		std::vector<uint8_t*> _bgrs;
		std::vector<const char*> _guids;
		std::string _param;
		std::vector<char> _result;
	};
}