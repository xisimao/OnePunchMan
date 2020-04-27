#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <tuple>

#include "DateTime.h"
#include "LogPool.h"
#include "StringEx.h"

namespace OnePunchMan
{
	//日志读取
	class LogReader
	{
	public:

		/**
		* @brief: 分页读取日志
		* @param: logDIrectory 日志目录
		* @param: logName 日志名称
		* @param: logDate 日志日期
		* @param: logLevel 日志级别，为0时查询所有
		* @param: logEvent 日志事件，为0时查询所有
		* @param: pageNum 分页页码
		* @param: pageSize 分页数量
		* @param: hasTotal 是否查询总数
		* @return: 第一个参数表示日志查询结果集合，第二个参数表示总数，如果hasTotal为false则返回0
		*/
		static std::tuple<std::vector<LogItem>,int> ReadLogs(const std::string logDirectory,const std::string& logName, const DateTime& logDate, int logLevel, int logEvent, int pageNum, int pageSize,bool hasTotal);

	};
}