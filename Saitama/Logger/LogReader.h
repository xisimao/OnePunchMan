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
	//��־��ȡ
	class LogReader
	{
	public:

		/**
		* @brief: ��ҳ��ȡ��־
		* @param: logDIrectory ��־Ŀ¼
		* @param: logName ��־����
		* @param: logDate ��־����
		* @param: logLevel ��־����Ϊ0ʱ��ѯ����
		* @param: logEvent ��־�¼���Ϊ0ʱ��ѯ����
		* @param: pageNum ��ҳҳ��
		* @param: pageSize ��ҳ����
		* @param: hasTotal �Ƿ��ѯ����
		* @return: ��һ��������ʾ��־��ѯ������ϣ��ڶ���������ʾ���������hasTotalΪfalse�򷵻�0
		*/
		static std::tuple<std::vector<LogItem>,int> ReadLogs(const std::string logDirectory,const std::string& logName, const DateTime& logDate, int logLevel, int logEvent, int pageNum, int pageSize,bool hasTotal);

	};
}