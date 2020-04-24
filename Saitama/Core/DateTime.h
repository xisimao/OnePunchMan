#pragma once
#include <sys/timeb.h>
#include <time.h>
#include <string>
#include <sstream>
#include <iomanip>

#ifndef _WIN32
#define localtime_s(timeinfo,t) localtime_r(t,timeinfo)
#define gmtime_s(timeinfo,t) gmtime_r(t,timeinfo)
#endif 


namespace Saitama
{
	//日期时间类
	class DateTime
	{
	public:

		/**
		* @brief: 构造函数
		*/
		DateTime();

		/**
		* @brief: 构造函数
		* @param: year 年
		* @param: month 月
		* @param: day 日
		*/
		DateTime(int year, int month, int day);

		/**
		* @brief: 构造函数
		* @param: year 年
		* @param: month 月
		* @param: day 日
		* @param: hour 时
		* @param: minute 分
		* @param: second 秒
		*/
		DateTime(int year, int month, int day, int hour, int minute, int second);

		/**
		* @brief: 构造函数
		* @param: year 年
		* @param: month 月
		* @param: day 日
		* @param: hour 时
		* @param: minute 分
		* @param: second 秒
		* @param: millisecond 毫秒
		*/
		DateTime(int year,int month,int day,int hour,int minute,int second,int millisecond);

		/**
		* @brief: 年
		* @return: 年
		*/
		int Year() const;
		/**
		* @brief: 月
		* @return: 月
		*/
		int Month() const;
		/**
		* @brief: 日
		* @return: 日
		*/
		int Day() const;
		/**
		* @brief: 时
		* @return: 时
		*/
		int Hour() const;
		/**
		* @brief: 分
		* @return: 分
		*/
		int Minute() const;
		/**
		* @brief: 秒
		* @return: 秒 
		*/
		int Second() const;
		/**
		* @brief: 毫秒
		* @return: 毫秒
		*/
		int Millisecond() const;

		/**
		* @brief: 返回表示时间的总毫秒数
		* @return: 表示时间的总毫秒数
		*/
		long long TimeStamp() const;

		/**
		* @brief: 增加月份
		* @param: month 增加的月份
		* @return: 增加后的日期时间
		*/
		DateTime AddMonth(int month);

		/**
		* @brief: 表示日期类型是否为空
		* @return: 返回true表示日期类型为空
		*/
		bool Empty() const;

		/**
		* @brief: 根据指定格式返回日期时间字符串，例如：%Y-%m-%d %H:%M:%S
		* @return: 日期时间字符串
		*/
		std::string ToString(const std::string& format) const;

		/**
		* @brief: 返回日期时间字符串，例如2017-01-01 00:00:00.001
		* @return: 日期时间字符串
		*/
		std::string ToString() const;

		bool operator == (const DateTime &right) const;

		bool operator < (const DateTime &right) const;

		bool operator != (const DateTime &right) const;

		bool operator > (const DateTime &right) const;

		bool operator >= (const DateTime &right) const;

		bool operator <= (const DateTime &right) const;

		long long operator - (const DateTime &right) const;

		/**
		* @brief: 从字符串解析日期时间
		* @param: format 时间字符串格式 例如%d-%d-%d %d:%d:%d.%d，%4d%2d%2d
		* @param: value 表示时间的字符串
		*/
		static DateTime ParseString(const std::string& format, const std::string& value);

		/**
		* @brief: 返回当前的日期时间
		* @return: 当前的日期时间
		*/
		static DateTime Now();

		/**
		* @brief: 返回当前的日期
		* @return: 当前的日期
		*/
		static DateTime Today();

		/**
		* @brief: 返回当前的时间
		* @return: 当前的时间
		*/
		static DateTime Time();


		/**
		* @brief: 返回当前的UTC时间戳
		* @return: 当前的UTC时间戳
		*/
		static long long UtcTimeStamp();

		/**
		* @brief: 返回当前的UTC日期时间
		* @return: 当前的UTC日期时间
		*/
		static DateTime UtcNow();

	private:

		/**
		* @brief: 构造函数
		* @param: year 年
		* @param: month 月
		* @param: day 日
		* @param: hour 时
		* @param: minute 分
		* @param: second 秒
		* @param: millisecond 毫秒
		* @param: milliseconds 时间戳
		*/
		DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond,long long milliseconds);

		//年
		int _year;
		//月
		int _month;
		//日
		int _day;
		//时
		int _hour;
		//分
		int _minute;
		//秒
		int _second;
		//毫秒
		int _millisecond;
		//时间戳
		long long _timeStamp;
	};
}
