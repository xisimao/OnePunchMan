#pragma once
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <numeric>
#include <map>
#include <iomanip>

namespace Saitama
{
	//文本操作类
	class StringEx
	{
	public:

		/**
		* @brief: 将数字转换为十六进制字符串
		* @param: value 数字
		* @return: 十六进制字符串
		*/
		static std::string ToHex(int value);

		/**
		* @brief: 将字节流转换为十六进制字符串
		* @param: buffer 字节流
		* @param: size 字节流长度
		* @return: 十六进制字符串
		*/
		static std::string ToHex(const char* buffer, unsigned int size);

		/**
		* @brief: 将字节流转换为十六进制字符串
		* @param: begin 字节流开始
		* @param: end 字节流结束
		* @return: 十六进制字符串
		*/
		static std::string ToHex(std::string::const_iterator begin, std::string::const_iterator end);

		/**
		* @brief: 将十六进制字符串转换为字节流
		* @param: value 字符串
		* @param: separator 分隔符
		* @return: 返回转换后的字节流
		*/
		static std::string ToBytes(const std::string& value, char separator);

		/**
		* @brief: 异或校验
		* @param: buffer 字节流指针
		* @param: size 字节流长度
		* @return: 返回异或校验结果
		*/
		static char Xor(const char* buffer, unsigned int size);

		/**
		* @brief: 将字符串变为大写
		* @param: value 字符串
		*/
		static std::string ToUpper(const std::string& value);

		/**
		* @brief: 移除字符串前后的空白字符
		* @param: value 字符串
		*/
		static std::string Trim(const std::string& value);

		/**
		* @brief: 分割字符串
		* @param: value 字符串
		* @param: separator 分割字符串
		* @param: filterEmpty 是否过滤掉空白
		*/
		static std::vector<std::string> Split(const std::string& value, const std::string& separator, bool filterEmpty = false);

		/**
		* @brief: 将字符串转换为集合类型
		* @param: value 字符串
		* @return: 转换成功返回转换结果否则返回空集合
		*/
		template<typename T>
		static std::vector<T> ConvertToArray(const std::string& value)
		{
			std::vector<T> v;
			size_t startIndex = 0;
			while (startIndex<value.size())
			{
				T t;
				size_t result = FindItem(value, &t, startIndex);
				if (result == 0)
				{
					break;
				}
				else
				{
					startIndex += result;
					v.push_back(t);
				}
			}
			return v;
		}

		/**
		* @brief: 将字符串转换为数字或者布尔类型
		* @param: value 字符串
		* @return: 转换成功返回转换结果否则返回0
		*/
		template<typename T>
		static T Convert(const std::string& value)
		{
			return Convert<T>(value, 0);
		}

		/**
		* @brief: 将字符串转换为数字或者布尔类型
		* @param: value 字符串
		* @param: defaultValue 转换失败返回的默认值
		* @return: 转换成功返回转换结果否则返回默认值
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
		* @brief: 将字符串转换为数字类型
		* @param: value 字符串
		* @param: t 数字或布尔类型指针
		* @return: 转换成功返回true
		*/
		template<typename T>
		static bool TryConvert(const std::string& value, T* t)
		{
			std::stringstream ss;
			ss << value;
			ss >> *t;
			return !ss.fail();
		}

#ifdef _WIN32
		template<>
		static std::string Convert<std::string>(const std::string& value)
		{
			return Convert<std::string>(value, std::string());
		}

		template<>
		static bool TryConvert<bool>(const std::string& value, bool* t)
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

		template<>
		static bool TryConvert<std::string>(const std::string& value, std::string* t)
		{
			*t = value;
			return true;
		}

#endif

		/**
		* @brief: 将数字转换为字符串
		* @param: value 数字
		* @return: 第一个参数表示转换是否成功，如果为true，第二个参数表示转换结果
		*/
		template<typename T>
		static std::string ToString(T value)
		{
			std::stringstream ss;
			ss << value;
			return std::string(ss.str());
		}

		/**
		* @brief: 四舍五入浮点数
		* @param: value 源浮点数
		* @param: precision 精确位数
		* @return: 四舍五入后的字符串
		*/
		static std::string Rounding(float value, int precision);

		/**
		* @brief: 将多个元素组合成一个字符串。
		* @param: t 由字符串的各部分构成的元素
		* @param: u... 由字符串的各部分构成的元素
		* @return: 已组合的字符串。
		*/
		template<typename T, typename ...U>
		static std::string Combine(const T& t, const U& ...u)
		{
			std::stringstream ss;
			Combine(&ss, t, u...);
			return ss.str();
		}

		/**
		* @brief: 将多个元素组合成一个字符串。
		* @param: t 由字符串的各部分构成的元素。
		* @param: u... 由字符串的各部分构成的元素。
		*/
		template<typename T, typename ...U>
		static void Combine(std::stringstream* ss, const T& t, const U& ...u)
		{
			(*ss) << t;
			Combine(ss, u...);
		}

		/**
		* @brief: 将最后一个元素拼接到字符串上。
		* @param: t 组成字符串的最后一个的元素。
		*/
		template<typename T>
		static void Combine(std::stringstream* ss, const T& t)
		{
			(*ss) << t;
		}

	private:

		/**
		* @brief: 从字符串中截取数组或布尔数组中的一项。
		* @param: json json字符串
		* @param: t 字段值的指针
		* @param: offset 从开头略过的字符数
		* @return: 解析成功返回解析的总字符数，否则返回0
		*/
		template<typename T>
		static size_t FindItem(const std::string& json, T* t, size_t offset)
		{
			size_t endIndex = json.find(',', offset);
			size_t size;
			size_t hasTail;
			if (endIndex == std::string::npos)
			{
				size = json.size() - offset;
				hasTail = false;
			}
			else
			{
				size = endIndex - offset;
				hasTail = true;
			}
			if (StringEx::TryConvert(json.substr(offset, size), t))
			{
				return hasTail ? size + 1 : size;
			}
			else
			{
				return 0;
			}
		}

		/**
		* @brief: 从字符串中截取字符串数组中的一项。。
		* @param: json json字符串
		* @param: t 字段值的指针
		* @param: offset 从开头略过的字符数
		* @return: 解析成功返回解析的总字符数，否则返回0
		*/
		static size_t FindItem(const std::string& json, std::string* t, size_t offset)
		{
			size_t startIndex = json.find('"', offset);
			if (startIndex == std::string::npos)
			{
				return 0;
			}
			else
			{
				size_t endIndex = startIndex;
				while (true)
				{
					endIndex = json.find('"', endIndex + 1);
					if (endIndex == std::string::npos)
					{
						return 0;
					}
					else
					{
						if (json[endIndex - 1] == '\\')
						{
							startIndex = endIndex - 1;
						}
						else
						{
							break;
						}
					}
				}
				t->assign(json.substr(startIndex + 1, endIndex - startIndex - 1));
				endIndex = json.find(',', endIndex);
				if (endIndex == std::string::npos)
				{
					return json.size() - startIndex;
				}
				else
				{
					return endIndex - startIndex + 1;
				}
			}
		}
	};



}

