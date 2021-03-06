#pragma once
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <numeric>
#include <map>
#include <iomanip>
#include <codecvt>


namespace OnePunchMan
{
	//文本操作类
	class StringEx
	{
	public:

		/**
		* 将数字转换为十六进制字符串
		* @param value 数字
		* @return 十六进制字符串
		*/
		static std::string ToHex(int value);

		/**
		* 将字节流转换为十六进制字符串
		* @param buffer 字节流
		* @param size 字节流长度
		* @return 十六进制字符串
		*/
		static std::string ToHex(const char* buffer, unsigned int size, const std::string& separator=" ");

		/**
		* 将字节流转换为十六进制字符串
		* @param begin 字节流开始
		* @param end 字节流结束
		* @return 十六进制字符串
		*/
		static std::string ToHex(std::string::const_iterator begin, std::string::const_iterator end, const std::string& separator = " ");

		/**
		* 将十六进制字符串转换为字节流
		* @param value 字符串
		* @param separator 分隔符
		* @return 返回转换后的字节流
		*/
		static std::string ToBytes(const std::string& value, const char separator);

		/**
		* 异或校验
		* @param buffer 字节流指针
		* @param size 字节流长度
		* @return 返回异或校验结果
		*/
		static char Xor(const char* buffer, unsigned int size);

		/**
		* base64算法加密
		* @param buffer 需要加密的字节流
		* @param size 需要加密的字节流长度
		* @return 加密后的字符串
		*/
		static void ToBase64String(const unsigned char* buffer, unsigned int size, std::string* base64);

		/**
		* base64算法解密
		* @param value 需要解密的字符串
		* @return 解密后的字符串
		*/
		static std::string FromBase64String(const std::string& value);

		/**
		* 将字符串变为大写
		* @param value 字符串
		*/
		static std::string ToUpper(const std::string& value);

		/**
		* 将字符串变为小写
		* @param value 字符串
		*/
		static std::string ToLower(const std::string& value);

		/**
		* 移除字符串前后的空白字符
		* @param value 字符串
		* @return 移除空白字符后的字符串
		*/
		static std::string Trim(const std::string& value);

		/**
		* 分割字符串
		* @param value 字符串
		* @param separator 分割字符串
		* @param filterEmpty 是否过滤掉空白
		* @return 分割后的字符串集合
		*/
		static std::vector<std::string> Split(const std::string& value, const std::string& separator, bool filterEmpty = false);

		/**
		* 替换字符串
		* @param value 字符串
		* @param oldValue 需要替换的值
		* @param newValue 替换成的值
		* @return 替换后的字符串
		*/
		static std::string Replace(const std::string& value, const std::string& oldValue, const std::string& newValue);
		
		/**
		* 替换url的转义字符
		* @param value 字符串
		* @return 替换后的字符串
		*/
		static std::string UnescapeUriDataString(const std::string& value);

		/**
		* 将字符串转换为数字或者布尔类型
		* @param value 字符串
		* @return 转换成功返回转换结果否则返回0
		*/
		template<typename T>
		static T Convert(const std::string& value)
		{
			return Convert<T>(value, 0);
		}

		/**
		* 将字符串转换为数字或者布尔类型
		* @param value 字符串
		* @param defaultValue 转换失败返回的默认值
		* @return 转换成功返回转换结果否则返回默认值
		*/
		template<typename T>
		static T Convert(const std::string& value,T defaultValue)
		{
			T t;
			if (TryConvert(value, &t))
			{
				return t;
			}
			else
			{
				return defaultValue;
			}
		}

		/**
		* 将字符串转换为数字类型
		* @param value 字符串
		* @param t 数字或布尔类型指针
		* @return 转换成功返回true
		*/
		template<typename T>
		static bool TryConvert(const std::string& value, T* t)
		{
			std::stringstream ss;
			ss << value;
			ss >> *t;
			return !ss.fail();
		}

		/**
		* 将字符串转换为布尔类型
		* @param value 字符串
		* @param t 数字或布尔类型指针
		* @return 转换成功返回true
		*/
		static bool TryConvert(const std::string& value, bool* t)
		{
			if (ToUpper(value).compare("TRUE") == 0)
			{
				*t = true;
				return true;
			}
			else if (ToUpper(value).compare("FALSE") == 0)
			{
				*t = false;
				return true;
			}
			else
			{
				return false;
			}
		}

		/**
		* 将字符串转换为字符串类型
		* @param value 字符串
		* @param t 数字或布尔类型指针
		* @return 转换成功返回true
		*/
		static bool TryConvert(const std::string& value, std::string* t)
		{
			*t = value;
			return true;
		}

		/**
		* 将数字转换为字符串
		* @param value 数字
		* @return 转换结果
		*/
		template<typename T>
		static std::string ToString(T value)
		{
			std::stringstream ss;
			ss << value;
			return std::string(ss.str());
		}

		/**
		* 将布尔转换为字符串
		* @param value 布尔类型
		* @return 转换结果
		*/
		static std::string ToString(bool value)
		{
			std::stringstream ss;
			ss << (value?"true":"false");
			return std::string(ss.str());
		}
		
		/**
		* 四舍五入浮点数
		* @param value 源浮点数
		* @param precision 精确位数
		* @return 四舍五入后的字符串
		*/
		template<typename T>
		static T Rounding(T value, int precision)
		{
			std::stringstream ss;
			ss << std::setiosflags(std::ios::fixed) << std::setprecision(precision);
			ss << value;
			T t = 0.0;
			ss >> t;
			return t;
		}

		/**
		* 将多个元素组合成一个字符串。
		* @param t 由字符串的各部分构成的元素
		* @param u... 由字符串的各部分构成的元素
		* @return 已组合的字符串。
		*/
		template<typename T, typename ...U>
		static std::string Combine(const T& t, const U& ...u)
		{
			std::stringstream ss;
			Combine(&ss, t, u...);
			return ss.str();
		}

		/**
		* 将多个元素组合成一个字符串。
		* @param t 由字符串的各部分构成的元素。
		* @param u... 由字符串的各部分构成的元素。
		*/
		template<typename T, typename ...U>
		static void Combine(std::stringstream* ss, const T& t, const U& ...u)
		{
			(*ss) << t;
			Combine(ss, u...);
		}

		/**
		* 将最后一个元素拼接到字符串上。
		* @param t 组成字符串的最后一个的元素。
		*/
		template<typename T>
		static void Combine(std::stringstream* ss, const T& t)
		{
			(*ss) << t;
		}
		private:

		/**
		* 判断字符是否是base64字符串范围内
		* @param value 字符串
		* @return 转换成功返回转换结果否则返回空字符串
		*/
		static inline bool IsBase64(unsigned char c);

		//base64字符集合
		static const std::string Base64Chars;

	};

	/**
	* 将字符串转换为字符串类型
	* @param value 字符串
	* @return 转换成功返回转换结果否则返回空字符串
	*/
	template<>
	inline std::string StringEx::Convert<std::string>(const std::string& value)
	{
		return Convert(value, std::string());
	}


	//文本操作类
	class WStringEx
	{
	public:
		/**
		* 将多个元素组合成一个字符串。
		* @param t 由字符串的各部分构成的元素
		* @param u... 由字符串的各部分构成的元素
		* @return 已组合的字符串。
		*/
		template<typename T, typename ...U>
		static std::string Combine(const T& t, const U& ...u)
		{
			std::wstringstream ss;
			Combine(&ss, t, u...);
			return ToString(ss.str());
		}

		/**
		* 将多个元素组合成一个字符串。
		* @param t 由字符串的各部分构成的元素。
		* @param u... 由字符串的各部分构成的元素。
		*/
		template<typename T, typename ...U>
		static void Combine(std::wstringstream* ss, const T& t, const U& ...u)
		{
			(*ss) << t;
			Combine(ss, u...);
		}

		/**
		* 将最后一个元素拼接到字符串上。
		* @param t 组成字符串的最后一个的元素。
		*/
		template<typename T>
		static void Combine(std::wstringstream* ss, const T& t)
		{
			(*ss) << t;
		}

		/**
		* wstring转string
		* @param value wstring
		* @return string
		*/
		static std::string ToString(const std::wstring& value)
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> cv;
			return cv.to_bytes(value);
		}
	};
}

