#pragma once
#include <string>
#include <fstream>
#include <sstream>

#ifdef _WIN32 
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h> 
#include <string.h>
#endif

#include "FileConfig.h"
#include "Path.h"
#include "Logger.h"
#include "DateTime.h"

namespace Saitama
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
		FileLogger(const std::string& name,LogLevel minLevel,LogLevel maxLevel);

		/**
		* @brief: 析构函数
		*/
		~FileLogger();

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

		void CreateDirectory(const std::string& directory);

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