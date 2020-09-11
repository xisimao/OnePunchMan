#include "FileLogger.h"

#include <iostream>
using namespace std;
using namespace OnePunchMan;

FileLogger::FileLogger(LogLevel minLevel, LogLevel maxLevel, const string& name,const string& directory,int holdDays)
	:Logger(minLevel,maxLevel),_name(name), _date(DateTime::Today()),_directory(directory),_holdDays(holdDays)
{
	//创建日志目录
	Path::CreatePath(_directory);

	//删除日志
	DeleteLog(_directory, _holdDays);

	Open();
}

FileLogger::~FileLogger()
{
	Close();
}

string FileLogger::GetLogFileName(const string& logName, const DateTime& logDate)
{
	return StringEx::Combine(logName, "_", logDate.ToString("%Y%m%d"), ".log");
}

void FileLogger::DeleteLog(const std::string& directory, unsigned int holdDays)
{
	long long holdMilliseconds = holdDays * 24 * 60 * 60 * 1000;
	DateTime today = DateTime::Today();
#ifdef _WIN32 
	string filter = Path::Combine(directory, "*");
	_finddata_t fileInfo;
	intptr_t handle = _findfirst(filter.c_str(), &fileInfo);
	if (handle == -1L)
	{
		return;
	}
	do
	{
		if (strcmp(fileInfo.name, ".") == 0 || strcmp(fileInfo.name, "..") == 0)
		{
			continue;
		}
		string fileName(fileInfo.name);
#else
	DIR* dir = opendir(directory.c_str());
	if (dir == NULL)
	{
		return;
	}

	struct dirent* file;

	while ((file = readdir(dir)) != NULL)
	{
		if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
		{
			continue;
		}
		string fileName(file->d_name);
#endif
		string filePath = Path::Combine(directory, fileName);
		if (fileName.find(_name) == 0)
		{
			vector<string> datas = StringEx::Split(fileName, "_");
			if (datas.size() >= 2)
			{
				DateTime date = DateTime::ParseString("%4d%2d%2d", Path::GetFileName(datas[datas.size() - 1]));
				long long l = today - date;
				if (!date.Empty() && l >= holdMilliseconds)
				{
					remove(filePath.c_str());
				}
			}
		}

#ifdef _WIN32
	} while (_findnext(handle, &fileInfo) == 0);
	_findclose(handle);
#else
	}
	closedir(dir);
#endif
}

void FileLogger::Open()
{
	string filePath = Path::Combine(_directory, GetLogFileName(_name,_date));
	_file.open(filePath, ofstream::out | ofstream::app);
}

void FileLogger::Close()
{
	if (_file.is_open())
	{
		_file.close();
	}
}

void FileLogger::LogCore(LogLevel logLevel, LogEvent logEvent, const DateTime& time, const string& content)
{
	DateTime today = DateTime::Today();
	if (_date != today)
	{
		_date = today;
		Close();
		DeleteLog(_directory, _holdDays);
		Open();
	}

	_file << content;
	_file.flush();
}