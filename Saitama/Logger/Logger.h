#pragma once
#include <string>
#include <thread>
#include <ostream>
#include <mutex>

#include "DateTime.h"

namespace OnePunchMan
{
	//日志类型
	enum class LogType
	{
		None = 0,
		Console = 1,
		File = 2
	};

	//日志级别
	enum class LogLevel : int
	{
		None = 0,
		Debug = 1,
		Information = 2,
		Warning = 3,
		Error = 4
	};

	//日志事件
	enum class LogEvent :int
	{
		None = 0,
		System = 1,
		Socket = 2,
		Mqtt = 3,
		Sqlite = 4,
		Encode = 5,
		Decode = 6,
		Detect = 7,
		Recogn = 8,
		Flow = 9,
		Event = 10,
		Adapter = 11,
		Http = 12,
		Monitor = 13
	};
	
	// 日志类
	class Logger
	{
	public:

		/**
		* @brief: 构造函数
		* @param: minLevel 日志最小级别
		* @param: maxLevel 日志最大级别
		*/
		Logger(LogLevel minLevel, LogLevel maxLevel)
		{
			_minLevel = minLevel;
			_maxLevel = maxLevel;
		}

		virtual ~Logger()
		{
		}

		/**
		* @brief: 写日志
		* @param: logLevel 日志级别
		* @param: logEvent 日志事件
		* @param: time 日志事件
		* @param: content 日志内容
		*/
		void Log(LogLevel logLevel, LogEvent logEvent,const DateTime& time,const std::string& content)
		{
			if ((int)logLevel >= (int)_minLevel && (int)logLevel <= (int)_maxLevel)
			{
				LogCore(logLevel,logEvent,time,content);
			}
		}

	protected:

		/**
		* @brief: 供子类实现的写日志
		* @param: logLevel 日志级别
		* @param: logEvent 日志事件
		* @param: time 日志事件
		* @param: content 日志内容
		*/
		virtual void LogCore(LogLevel logLevel, LogEvent logEvent, const DateTime& time, const std::string& content)=0;

	private:

		//日志最小级别
		LogLevel _minLevel;
		//日志最大级别
		LogLevel _maxLevel;
	};
}