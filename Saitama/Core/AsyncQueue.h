#pragma once
#include <mutex>
#include <string.h>
#include <vector>
#include <set>
#include <queue>
#include <iostream> //for test

namespace Saitama
{

	//异步队列
	class AsyncQueue
	{
	public:

		/**
		* @brief: 构造函数，默认容量
		*/
		AsyncQueue();

		/**
		* @brief: 析构函数
		*/
		~AsyncQueue();

		
		/**
		* @brief: 存入字节流
		* @param: buffer 字节流
		* @param: size 字节流长度
		* @return: true表示获取成功，false有两种可能，size为0或者容量已满
		*/
		bool Push(const char* buffer, unsigned int size);

		/**
		* @brief: 读取最后一项字节流
		* @parameter: key 用于读取键
		* @parameter: buffer 用于读取字节流
		* @parameter: capacity 字节流缓冲的容量
		* @return: 返回0表示读取失败表示没有可读项，返回-1表示缓冲区容量不足，否则返回读取长度
		*/
		int Pop(char* buffer, unsigned int capacity);
		

		/**
		* @brief: 获取当前缓冲区的字节流总长度
		* @return: 当前缓冲区的字节流总长度，以字节为单位
		*/
		unsigned int Size();

		/**
		* @brief: 获取当前缓冲区是否为空
		* @return: 返回true表示为空
		*/
		bool Empty();

		//测试用
		void Show();

	private:

		/**
		* @brief: 数字类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字节流。
		* @param: start 表示从第几个字节开始序列化
		* @param: capacity 字节流容量。
		* @param: value 数字。
		* @return: 如果序列化成功返回字节流长度，如果失败返回0。
		*/
		static void PushSize(char* buffer, unsigned int start, unsigned int capacity, unsigned int value)
		{
			for (unsigned int i = 0; i < capacity; ++i)
			{
				buffer[i] = static_cast<char>(value >> 8 * (i + start));
			}
		}
		/**
		* @brief: 数字类型的反序列化。
		* @param: buffer 包含要反序列化的数据的字节流。
		* @param: start 表示从第几个字节开始反序列化
		* @param: size 字节流长度。
		* @param: value 数字指针
		* @return: 如果反序列化成功返回使用的字节流长度，如果失败返回0。
		*/
		static void PopSize(const char* buffer, unsigned int start, unsigned int size, unsigned int* value)
		{
			for (unsigned int i = 0; i < size; ++i)
			{
				*value = *value | static_cast<unsigned char>(buffer[i]) << (8 * (i + start));
			}
		}

		/**
		* @brief: 判断当前剩余的缓冲区能否容纳字节流
		* @parameter: index 当前基准的序号
		* @parameter: size 需要复制的长度
		* @return: 返回true表示可以复制到尾，否则需要截断后从头复制
		*/
		bool NeedTurn(unsigned int index, unsigned int size);

		/**
		* @brief: 获取复制后的新基准序号
		* @parameter: index 当前基准的序号
		* @parameter: size 需要复制的长度
		* @return: 复制后的新基准序号
		*/
		unsigned int NewIndex(unsigned int index, unsigned int size);

		//缓冲区
		char* _buffer;
		//队列的最小容量
		static const int Capacity;
		//保存长度需要的字节数
		static const int HeadSize;

		//当前存入时用到的序号，表示当前可用字节流的结束序号后一位
		unsigned int _pushIndex;
		//存入同步标识
		std::mutex _pushMutex;

		//当前读取时用到的序号，表示当前可用字节流的起始序号
		unsigned int _popIndex;
		//读取同步标识
		std::mutex _popMutex;

	};
}

