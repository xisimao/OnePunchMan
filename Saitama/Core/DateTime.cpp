#include "DateTime.h"

using namespace std;
using namespace OnePunchMan;

DateTime::DateTime()
	:DateTime(0, 0, 0)
{

}

DateTime::DateTime(int year, int month, int day)
	: DateTime(year, month, day, 0, 0, 0)
{

}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second)
	: DateTime(year, month, day, hour, minute, second, 0)
{

}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond)
{
	_year = year;
	_month = month;
	_day = day;
	_hour = hour;
	_minute = minute;
	_second = second;
	_millisecond = millisecond;

	if (!Empty())
	{
		if (year == 0)
		{
			_timeStamp = hour * 24 * 60 * 60 * 1000 + minute * 60 * 1000 + second * 1000 + millisecond;
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

			timeinfo.tm_isdst = 0;
			timeinfo.tm_wday = 0;
			timeinfo.tm_yday = 0;
			time_t t = mktime(&timeinfo);
			if (t == -1)
			{
				_timeStamp = 0;
			}
			else
			{
				_timeStamp = t * 1000 + millisecond;
			}
		}
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

long long DateTime::TimeStamp() const
{
	return _timeStamp;
}

bool DateTime::operator == (const DateTime& right) const
{
	return _year == right.Year() &&
		_month == right.Month() &&
		_day == right.Day() &&
		_hour == right.Hour() &&
		_minute == right.Minute() &&
		_second == right.Second() &&
		_millisecond == right.Millisecond();
}

bool DateTime::operator < (const DateTime& right) const
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

bool DateTime::operator != (const DateTime& right) const
{
	return !(*this == right);
}

bool DateTime::operator > (const DateTime& right) const
{
	return !(*this == right) && !(*this < right);
}

bool DateTime::operator >= (const DateTime& right) const
{
	return !(*this < right);
}

bool DateTime::operator <= (const DateTime& right) const
{
	return (*this == right) || (*this < right);
}

long long DateTime::operator - (const DateTime& right) const
{
	return this->TimeStamp() - right.TimeStamp();
}

DateTime DateTime::AddMonth(int month)
{
	int tempMonth = _month + month;
	if (tempMonth > 12)
	{
		return DateTime(_year + tempMonth / 12, tempMonth % 12, _day, _hour, _minute, _second, _millisecond);
	}
	else
	{
		return DateTime(_year, tempMonth, _day, _hour, _minute, _second, _millisecond);
	}
}

bool DateTime::Empty() const
{
	return _year == 0 && _month == 0 && _day == 0 && _hour == 0 && _minute == 0 && _second == 0 && _millisecond == 0;
}

DateTime DateTime::ToUtcTime() const
{
	time_t seconds = _timeStamp / 1000;
	tm utcTime;
	gmtime_s(&utcTime, &seconds);
	return DateTime(utcTime.tm_year + 1900, utcTime.tm_mon + 1, utcTime.tm_mday, utcTime.tm_hour, utcTime.tm_min, utcTime.tm_sec, static_cast<int>(_timeStamp % 1000));
}

string DateTime::ToString() const
{
	stringstream ss;
	ss << ToString("%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << _millisecond;
	return ss.str();
}

string DateTime::ToString(const string& format) const
{
	struct tm timeinfo;
	timeinfo.tm_year = _year - 1900;
	timeinfo.tm_mon = _month - 1;
	timeinfo.tm_mday = _day;
	timeinfo.tm_hour = _hour;
	timeinfo.tm_min = _minute;
	timeinfo.tm_sec = _second;

	timeinfo.tm_isdst = 0;
	timeinfo.tm_wday = 0;
	timeinfo.tm_yday = 0;
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

DateTime DateTime::ParseTimeStamp(long long timeStamp)
{
	time_t seconds = timeStamp / 1000;
	tm localTime;
	localtime_s(&localTime, &seconds);
	return DateTime(localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday, localTime.tm_hour, localTime.tm_min, localTime.tm_sec, static_cast<int>(timeStamp % 1000));
	/*long long seconds = timeStamp / 1000;
	static const int hoursInDay = 24;
	static const int minutesInHour = 60;
	static const int daysFromUnixTime = 2472632;
	static const int daysFromYear = 153;
	static const int magicUnknownFirst = 146097;
	static const int magicUnknownSecond = 1461;
	int second = seconds % minutesInHour;
	int i =static_cast<int>(seconds / minutesInHour);
	int minute = i % minutesInHour;
	i /= minutesInHour;
	int hour = i % hoursInDay;
	int day = i / hoursInDay;
	int a = day + daysFromUnixTime;
	int b = (a * 4 + 3) / magicUnknownFirst;
	int c = (-b * magicUnknownFirst) / 4 + a;
	int d = ((c * 4 + 3) / magicUnknownSecond);
	int e = -d * magicUnknownSecond;
	e = e / 4 + c;
	int m = (5 * e + 2) / daysFromYear;
	day = -(daysFromYear * m + 2) / 5 + e + 1;
	int month = (-m / 10) * 12 + m + 2;
	int year = b * 100 + d - 6700 + (m / 10);
	return DateTime(year + 1900, month + 1, day, hour, minute, second, static_cast<int>(timeStamp % 1000));*/
}

DateTime DateTime::Now()
{
	return ParseTimeStamp(NowTimeStamp());
}

long long DateTime::NowTimeStamp()
{
	struct timeb tb;
	ftime(&tb);
	return tb.time * 1000 + tb.millitm;
}

DateTime DateTime::Today()
{
	DateTime d = DateTime::Now();
	return DateTime(d.Year(), d.Month(), d.Day());
}

DateTime DateTime::Time()
{
	DateTime d = DateTime::Now();
	return DateTime(0, 0, 0, d.Hour(), d.Minute(), d.Second());
}

int DateTime::TimeZone()
{
	struct timeb tb;
	ftime(&tb);

	tm localTime;
	localtime_s(&localTime, &tb.time);
	tm utcTime;
	gmtime_s(&utcTime, &tb.time);

	time_t localTimeStamp = mktime(&localTime);
	time_t utcTimeStamp = mktime(&utcTime);

	return static_cast<int>((localTimeStamp - utcTimeStamp) / 3600);
}
