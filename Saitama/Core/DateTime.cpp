#include "DateTime.h"

using namespace std;
using namespace OnePunchMan;

DateTime::DateTime()
	:DateTime(0,0,0)
{

}

DateTime::DateTime(int year, int month, int day)
	:DateTime(year,month,day,0,0,0)
{

}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second)
	: DateTime(year, month, day, hour,minute,second,0)
{

}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond)
{
	if (month == 0)
	{
		month = 1;
	}
	if (day == 0)
	{
		day = 1;
	}

	_year = year;
	_month = month;
	_day = day;
	_hour = hour;
	_minute = minute;
	_second = second;
	_millisecond = millisecond;


	if (year == 0)
	{
		_utcTimeStamp = hour * 24 * 60 * 60 * 1000 + minute * 60 * 1000 + second * 1000 + millisecond;
	}
	else
	{
		struct tm timeinfo;
		timeinfo.tm_year = _year - 1900;
		timeinfo.tm_mon = _month - 1;
		timeinfo.tm_mday = _day;
		timeinfo.tm_hour = _hour;
		timeinfo.tm_min = _minute;
		timeinfo.tm_sec = _second;
		time_t t = mktime(&timeinfo);
		_utcTimeStamp = t * 1000 + millisecond;
	}
}

int DateTime::Year() const
{
	return _year;
}

int DateTime::Month() const
{
	return _month;
}

int DateTime::Day() const
{
	return _day;
}

int DateTime::Hour() const
{
	return _hour;
}

int DateTime::Minute() const
{
	return _minute;
}

int DateTime::Second() const
{
	return _second;
}

int DateTime::Millisecond() const
{
	return _millisecond;
}

long long DateTime::UtcTimeStamp() const
{
	return _utcTimeStamp;
}

DateTime DateTime::AddMonth(int month)
{
	int tempMonth = _month + month;
	if (tempMonth > 12)
	{
		return DateTime(_year+ tempMonth / 12, tempMonth % 12, _day, _hour, _minute, _second, _millisecond);
	}
	else
	{
		return DateTime(_year, tempMonth, _day, _hour, _minute, _second, _millisecond);
	}
}

bool DateTime::Empty() const
{
	return _year == 0 && _month == 0 && _day == 0 && _hour == 0 && _minute == 0 && _second == 0;
}

bool DateTime::operator == (const DateTime &right) const
{
	return _year == right.Year()&&
		_month == right.Month()&&
		_day==right.Day()&&
		_hour==right.Hour()&&
		_minute==right.Minute()&&
		_second==right.Second()&&
		_millisecond==right.Millisecond();
}

bool DateTime::operator < (const DateTime &right) const
{
	if (_year < right.Year())
	{
		return true;
	}
	else if (_year > right.Year())
	{
		return false;
	}

	if (_month < right.Month())
	{
		return true;
	}
	else if (_month > right.Month())
	{
		return false;
	}

	if (_day < right.Day())
	{
		return true;
	}
	else if (_day > right.Day())
	{
		return false;
	}

	if (_hour < right.Hour())
	{
		return true;
	}
	else if (_hour > right.Hour())
	{
		return false;
	}

	if (_minute < right.Minute())
	{
		return true;
	}
	else if (_minute > right.Minute())
	{
		return false;
	}

	if (_second < right.Second())
	{
		return true;
	}
	else if (_second > right.Second())
	{
		return false;
	}

	if (_millisecond < right.Millisecond())
	{
		return true;
	}
	else if (_millisecond > right.Millisecond())
	{
		return false;
	}

	return false;
}

bool DateTime::operator != (const DateTime &right) const
{
	return !(*this == right);
}

bool DateTime::operator > (const DateTime &right) const
{
	return !(*this == right) && !(*this < right);
}

bool DateTime::operator >= (const DateTime &right) const
{
	return !(*this < right);
}

bool DateTime::operator <= (const DateTime &right) const
{
	return (*this == right) || (*this < right);
}

long long DateTime::operator - (const DateTime &right) const
{
	return this->UtcTimeStamp() - right.UtcTimeStamp();
}

string DateTime::ToString() const
{
	stringstream ss;
	ss << ToString("%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << _millisecond;
	return ss.str();
}

string DateTime::ToString(const string& format) const
{
	time_t time = _utcTimeStamp / 1000;
	struct tm timeinfo;
	localtime_s(&timeinfo, &time);
	char buffer[80] = { 0 };
	strftime(buffer, 80, format.c_str(), &timeinfo);
	return string(buffer);
}

DateTime DateTime::ParseString(const string& format, const string& value)
{
	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int minute = 0;
	int second = 0;
	int millisecond = 0;
	sscanf(value.c_str(), format.c_str(), &year, &month, &day, &hour, &minute, &second, &millisecond);
	return DateTime(year, month, day, hour, minute, second, millisecond);
}

long long DateTime::UtcNowTimeStamp()
{
	struct timeb tb;
	ftime(&tb);
	return tb.time * 1000 + tb.millitm;
}

DateTime DateTime::Now()
{
	struct timeb tb;
	ftime(&tb);
	tm timeinfo;
	localtime_s(&timeinfo,&tb.time);
	return DateTime(timeinfo.tm_year+1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, tb.millitm);
}

DateTime DateTime::Today()
{
	struct timeb tb;
	ftime(&tb);
	tm timeinfo;
	localtime_s(&timeinfo,&tb.time);
	return DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,0,0,0,0);
}

DateTime DateTime::Time()
{
	struct timeb tb;
	ftime(&tb);
	tm timeinfo;
	localtime_s(&timeinfo, &tb.time);
	return DateTime(0,0,0,timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, tb.millitm);
}

DateTime DateTime::UtcNow()
{
	struct timeb tb;
	ftime(&tb);
	tm timeinfo;
	gmtime_s(&timeinfo, &tb.time);
	return DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, tb.millitm);
}
