#pragma once
#include "Logger.h"
#include "Sqlite.h"

namespace OnePunchMan
{
	//日志数据
	class LogData
	{
	public:
		LogData()
			:Id(0), LogLevel(0), LogEvent(0),Time(),Content()
		{

		}
		int Id;
		int LogLevel;
		int LogEvent;
		std::string Time;
		std::string Content;
	};

	class SqliteLogger:public Logger
	{
	public:
		SqliteLogger(const std::string& dbName="flow.db");

		static std::vector<LogData> GetDatas(int logLevel,int logEvent,const std::string& startTime,const std::string& endTime,int pageNum,int pageSize,int* total);

		static void RemoveDatas(const DateTime& time);

	protected:
		void LogCore(LogLevel logLevel, LogEvent logEvent, const DateTime& time, const std::string& content);

	private:
		static std::string _dbName;

		SqliteWriter _writter;

	};
}

