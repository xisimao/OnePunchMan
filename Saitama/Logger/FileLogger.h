#pragma once
#include <string>
#include <fstream>

#ifdef _WIN32 
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h> 
#include <string.h>
#endif

#include "Path.h"
#include "StringEx.h"
#include "Logger.h"
#include "DateTime.h"

namespace OnePunchMan
{
	//文件日志类 
	class FileLogger : public Logger
	{
	public:

		/**
		* @brief: 构造函数
		* @param: name 日志名称
		* @param: minLevel 日志最小级别
		* @param: maxLevel 日志最大级别
		*/
		FileLogger(LogLevel minLevel, LogLevel maxLevel, const std::string& name, const std::string& directory, int holdDays);

		/**
		* @brief: 析构函数
		*/
		~FileLogger();

		/**
		* @brief: 获取日志文件名
		* @param: name 日志名称
		* @param: DateTime 日志日期
		* @return: 日志文件名
		*/
		static std::string GetLogFileName(const std::string& logName, const DateTime& logDate);

	protected:

		void LogCore(const std::string& log);

	private:

		/**
		* @brief: 打开文件
		*/
		void Open();

		/**
		* @brief: 关闭文件
		*/
		void Close();

		/**
		* @brief: 删除日志
		* @param: directory 目录
		* @param: holdDays 日志保存天数
		*/
		void DeleteLog(const std::string& directory, unsigned int holdDays);

		//文件名 
		std::string _name;
		//日期 
		DateTime _date;
		//文件保存目录 
		std::string _directory;
		//日志保存天数
		unsigned int _holdDays;
		//文件流 
		std::ofstream _file;

	};

}