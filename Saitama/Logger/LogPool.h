#pragma once
#include <sstream>
#include <set>
#include <string>

#include "Logger.h"
#include "ConsoleLogger.h"
#include "FileLogger.h"
#include "JsonFormatter.h"

namespace OnePunchMan
{
	//#define Stack(level,...) LogPool::Socket(level ,"[",__FILE__,":" __LINE__," ", __FUNCTION__,"]",##__VA_ARGS__);
	
	// 日志池 
	class LogPool
	{
	public:

		/**
		* @brief: 析构函数
		*/
		~LogPool();

		/**
		* @brief: 初始化日志
		* @param: jd json解析
		*/
		static void Init(const JsonDeserialization& jd);

		/**
		* @brief: 添加日志写入
		* @param: logger 日志写入指针
		*/
		static void AddLogger(Logger* logger);

		/**
		* @brief: 调试日志
		* @param: t 日志的内容
		* @param: ...u 日志的内容
		*/
		template<typename T, typename ...U>
		static void Debug(T t, U ...u)
		{
			Debug(LogEvent::None, t, u...);
		}

		/**
		* @brief: 调试日志
		* @param: logEvent 日志事件
		* @param: t 日志的内容
		* @param: ...u 日志的内容
		*/
		template<typename T, typename ...U>
		static void Debug(LogEvent logEvent, T t, U ...u)
		{
			if (_instance != NULL)
			{
				_instance->Log(LogLevel::Debug, logEvent, t, u...);
			}
		}

		/**
		* @brief: 消息日志
		* @param: t 日志的内容
		* @param: ...u 日志的内容
		*/
		template<typename T, typename ...U>
		static void Information(T t, U ...u)
		{
			Information(LogEvent::None, t, u...);
		}

		/**
		* @brief: 消息日志
		* @param: logEvent 日志事件
		* @param: t 日志的内容
		* @param: ...u 日志的内容
		*/
		template<typename T, typename ...U>
		static void Information(LogEvent logEvent, T t, U ...u)
		{
			if (_instance != NULL)
			{
				_instance->Log(LogLevel::Information, logEvent, t, u...);
			}
		}

		/**
		* @brief: 警告日志
		* @param: t 日志的内容
		* @param: ...u 日志的内容
		*/
		template<typename T, typename ...U>
		static void Warning(T t, U ...u)
		{
			Warning(LogEvent::None, t, u...);
		}

		/**
		* @brief: 警告日志
		* @param: logEvent 日志事件
		* @param: t 日志的内容
		* @param: ...u 日志的内容
		*/
		template<typename T, typename ...U>
		static void Warning(LogEvent logEvent, T t, U ...u)
		{
			if (_instance != NULL)
			{
				_instance->Log(LogLevel::Warning, logEvent, t, u...);
			}
		}

		/**
		* @brief: 错误日志
		* @param: t 日志的内容
		* @param: ...u 日志的内容
		*/
		template<typename T, typename ...U>
		static void Error(T t, U ...u)
		{
			Error(LogEvent::None, t, u...);
		}

		/**
		* @brief: 错误日志
		* @param: logEvent 日志事件
		* @param: t 日志的内容
		* @param: ...u 日志的内容
		*/
		template<typename T, typename ...U>
		static void Error(LogEvent logEvent, T t, U ...u)
		{
			if (_instance != NULL)
			{
				_instance->Log(LogLevel::Error, logEvent, t, u...);
			}	
		}

		/**
		* @brief: 获取文件日志保存目录
		* @return: 文件日志保存目录
		*/
		static std::string Directory();

		/**
		* @brief: 获取文件日志保存天数
		* @return: 文件日志保存天数
		*/
		static int HoldDays();

	private:

		/**
		* @brief: 构造函数
		* @param: jd json解析
		*/
		LogPool(const JsonDeserialization& jd);

		/**
		* @brief: 从配置文件读取日志类型
		* @param: jd json解析
		* @param: key 日志级别对应的配置项key
		* @return: 如果读取成功返回类型,否则返回None
		*/
		LogType ReadType(const JsonDeserialization& jd, const std::string& key);

		/**
		* @brief: 从配置文件读取日志级别
		* @param: jd json解析
		* @param: key 日志级别对应的配置项key
		* @return: 如果读取成功返回级别,否则返回None
		*/
		LogLevel ReadLevel(const JsonDeserialization& jd, const std::string& key);

		/**
		* @brief: 写日志
		* @param: level 日志级别
		* @param: t  日志的内容
		* @param: ...u 日志的内容
		*/
		template<typename T, typename ...U>
		void Log(LogLevel logLevel, LogEvent logEvent, T t, U ...u)
		{
			if (logLevel < _defaultLevel)
			{
				return;
			}
			DateTime time = DateTime::Now();
			std::stringstream ss;
			ss << "[" << time.ToString() << "]";
			ss << "[" << (int)logLevel << "]";
			ss << "[" << (int)logEvent << "] ";
			ContentFormat(&ss, t, u...);
			std::lock_guard<std::mutex> lck(_mutex);
			for (std::set<Logger*>::iterator it = _loggers.begin(); it != _loggers.end(); ++it)
			{
				(*it)->Log(logLevel,logEvent,time, ss.str());
			}
		}

		/**
		* @brief: 日志内容格式
		* @param: t日志内容
		* @param: ...u 日志内容
		*/
		template<typename T, typename ...U>
		void ContentFormat(std::stringstream* ss, T t, U ...u)
		{
			(*ss) << t << " ";
			ContentFormat(ss, u...);
		}

		/**
		* @brief: 日志内容格式
		* @param: t日志内容
		*/
		template<typename T>
		void ContentFormat(std::stringstream* ss, T t)
		{
			(*ss) << t << std::endl;
		}

		// 单例引用
		static LogPool* _instance;

		//文件保存目录 
		std::string _directory;
		//日志保存天数
		unsigned int _holdDays;
		//默认接收的最小日志级别
		LogLevel _defaultLevel;
		std::mutex _mutex;
		//日志集合
		std::set<Logger*> _loggers;
	};
}