#include "SqliteLogger.h"

using namespace std;
using namespace OnePunchMan;

string SqliteLogger::_dbName;

SqliteLogger::SqliteLogger(const string& dbName)
	:Logger(LogLevel::Information,LogLevel::Error) ,_writter(dbName)
{
	_dbName = dbName;
}

vector<LogData> SqliteLogger::GetDatas(int logLevel, int logEvent, const string& startTime, const string& endTime, int pageNum, int pageSize, int* total)
{
	SqliteReader reader(_dbName);
	vector<LogData> datas;
	string whereSql(" Where ");
	whereSql.append(logLevel == static_cast<int>(LogLevel::None) ? "1=1 " : StringEx::Combine("LogLevel=", logLevel, " "));
	whereSql.append(logEvent == static_cast<int>(LogEvent::None) ? "And 1=1 " : StringEx::Combine("And LogEvent=", logEvent, " "));
	whereSql.append(startTime.empty() ? "And 1=1 " : StringEx::Combine("And Time>='", startTime, "' "));
	whereSql.append(endTime.empty() ? "And 1=1 " : StringEx::Combine("And Time<='", endTime, "' "));


	string dataSql = "Select * From System_Log";
	dataSql.append(whereSql);
	dataSql.append(pageSize == 0 ? "" : StringEx::Combine("Order By Time Desc Limit ", (pageNum - 1) * pageSize, ",", pageSize));
	if (reader.BeginQuery(dataSql))
	{
		while (reader.HasRow())
		{
			LogData data;
			data.Id = reader.GetInt(0);
			data.LogLevel = reader.GetInt(1);
			data.LogEvent = reader.GetInt(2);
			data.Time = reader.GetString(3);
			data.Content = reader.GetString(4);
			datas.push_back(data);
		}
		reader.EndQuery();
	}
	string totalSql = "Select Count(*) From System_Log";
	totalSql.append(whereSql);
	*total = reader.ExecuteScalar(totalSql);
	return datas;
}

void SqliteLogger::RemoveDatas(const DateTime& time)
{
	string sql = StringEx::Combine("Delete From System_Log Where Time<'",time.ToString(),"'");
	SqliteWriter writer(_dbName);
	writer.ExecuteRowCount(sql);
}

void SqliteLogger::LogCore(LogLevel logLevel, LogEvent logEvent, const DateTime& time, const std::string& log)
{
	string sql = StringEx::Combine("Insert Into System_Log (Id,LogLevel,LogEvent,Time,Content) Values "
		, "(NULL,"
		, static_cast<int>(logLevel), ","
		, static_cast<int>(logEvent), ","
		, "'", time.ToString(), "',"
		, "'", log.substr(32,log.size()-33), "')");
	_writter.Execute(sql);
}
