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


namespace OnePunchMan
{
	//日期时间类
	class DateTime
	{
	public:

		/**
		* 构造函数
		*/
		DateTime();

		/**
		* 构造函数
		* @param year 年
		* @param month 月
		* @param day 日
		*/
		DateTime(int year, int month, int day);

		/**
		* 构造函数
		* @param year 年
		* @param month 月
		* @param day 日
		* @param hour 时
		* @param minute 分
		* @param second 秒
		*/
		DateTime(int year, int month, int day, int hour, int minute, int second);

		/**
		* 构造函数
		* @param year 年
		* @param month 月
		* @param day 日
		* @param hour 时
		* @param minute 分
		* @param second 秒
		* @param millisecond 毫秒
		*/
		DateTime(int year,int month,int day,int hour,int minute,int second,int millisecond);

		/**
		* 年
		* @return 年
		*/
		int Year() const;
		/**
		* 月
		* @return 月
		*/
		int Month() const;
		/**
		* 日
		* @return 日
		*/
		int Day() const;
		/**
		* 时
		* @return 时
		*/
		int Hour() const;
		/**
		* 分
		* @return 分
		*/
		int Minute() const;
		/**
		* 秒
		* @return 秒 
		*/
		int Second() const;
		/**
		* 毫秒
		* @return 毫秒
		*/
		int Millisecond() const;

		/**
		* 返回表示时间的utc时间戳
		* @return 表示时间的utc时间戳
		*/
		long long UtcTimeStamp() const;

		/**
		* 增加月份
		* @param month 增加的月份
		* @return 增加后的日期时间
		*/
		DateTime AddMonth(int month);

		/**
		* 表示日期类型是否为空
		* @return 返回true表示日期类型为空
		*/
		bool Empty() const;

		/**
		* 根据指定格式返回日期时间字符串,例如:%Y-%m-%d %H:%M:%S
		* @return 日期时间字符串
		*/
		std::string ToString(const std::string& format) const;

		/**
		* 返回日期时间字符串,例如2017-01-01 00:00:00.001
		* @return 日期时间字符串
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
		* 从字符串解析日期时间
		* @param format 时间字符串格式 例如%d-%d-%d %d:%d:%d.%d,%4d%2d%2d
		* @param value 表示时间的字符串
		* @return 解析出的时间
		*/
		static DateTime ParseString(const std::string& format, const std::string& value);

		/**
		* 从时间戳解析日期时间
		* @param utcTimeStamp utc时间戳
		* @return 解析出的时间
		*/
		static DateTime  ParseTimeStamp(long long utcTimeStamp);

		/**
		* 返回当前的日期时间
		* @return 当前的日期时间
		*/
		static DateTime Now();

		/**
		* 返回当前的日期
		* @return 当前的日期
		*/
		static DateTime Today();

		/**
		* 返回当前的时间
		* @return 当前的时间
		*/
		static DateTime Time();

		/**
		* 返回当前的UTC时间戳
		* @return 当前的UTC时间戳
		*/
		static long long UtcNowTimeStamp();

		/**
		* 返回当前的UTC日期时间
		* @return 当前的UTC日期时间
		*/
		static DateTime UtcNow();

	private:

		/**
		* 构造函数
		* @param year 年
		* @param month 月
		* @param day 日
		* @param hour 时
		* @param minute 分
		* @param second 秒
		* @param millisecond 毫秒
		* @param milliseconds 时间戳
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
		//utc时间戳
		long long _utcTimeStamp;
	};
}
