#pragma once
#include <string>
#include <vector>
#include <set>
#include <string.h>
#include <algorithm>

namespace Saitama
{
	//字符串容器二进制序列化
	class StringFormatter
	{
	public:

		/**
		* @brief: 递归序列化可变参数。
		* @param: buffer 格式化程序在其中放置序列化数据的字符串。
		* @param: t 当前递归到的序列化的第一项
		* @param: ...u 序列化的其他项
		*/
		template<typename T, typename ...U>
		static void Serialize(std::string* buffer, const T& t, U ...u)
		{
			Serialize(buffer, t);
			Serialize(buffer, u...);
		}

		/**
		* @brief: 布尔类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字符串。
		* @param: value 布尔类型
		*/
		template<>
		static void Serialize(std::string* buffer, const bool& value);

		/**
		* @brief: 数字类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字符串。
		* @param: value 数字。
		*/
		template<typename T>
		static void Serialize(std::string* buffer, const T& value)
		{
			for (int i = 0; i < static_cast<int>(sizeof(T)); ++i)
			{
				buffer->push_back(static_cast<char>(value >> 8 * (sizeof(T) - i - 1)));
			}
		}

		/**
		* @brief: 单字节浮点类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字符串。
		* @param: value 单字节浮点类型
		*/
		template<>
		static void Serialize(std::string* buffer, const float& value);

		/**
		* @brief: 双字节浮点类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字符串。
		* @param: value 双字节浮点类型
		*/
		template<>
		static void Serialize(std::string* buffer, const double& value);

		/**
		* @brief: 字符串类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字符串。
		* @param: value 字符串
		*/
		template<>
		static void Serialize(std::string* buffer, const std::string& value);

		/**
		* @brief: 列表类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字符串。
		* @param: values 列表
		*/
		template<typename T>
		static void Serialize(std::string* buffer, const std::vector<T>& values)
		{
			Serialize(buffer, static_cast<unsigned short>(values.size()));
			for (unsigned int i = 0; i < values.size(); ++i)
			{
				Serialize(buffer, values.at(i));
			}
		}

		/**
		* @brief: 集合类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字符串。
		* @param: values 集合
		*/
		template<typename T>
		static void Serialize(std::string* buffer, const std::set<T>& values)
		{

			Serialize(buffer, static_cast<unsigned short>(values.size()));
			for (typename std::set<T>::const_iterator it = values.begin(); it != values.end(); ++it)
			{
				Serialize(buffer, *it);
			}
		}

		/**
		* @brief: 递归反序列化可变参数。
		* @param: buffer 包含要反序列化的数据的字节流缓冲
		* @param: size 节流的长度。
		* @param: t 当前递归到的反序列化的第一项
		* @param: ...u 反序列化的其他项
		* @return: 如果反序列化成功返回使用的字节流长度，如果失败返回0。
		*/
		template<typename T, typename ...U>
		static unsigned int Deserialize(std::string::const_iterator begin, std::string::const_iterator end, T* t, U... u)
		{
			unsigned int dSize = Deserialize(begin, end, t);
			if (dSize == 0)
			{
				return 0;
			}
			else
			{
				unsigned int temp = Deserialize(begin + dSize,end, u...);
				if (temp == 0)
				{
					return 0;
				}
				else
				{
					return dSize + temp;
				}
			}
		}

		/**
		* @brief: 布尔类型的反序列化。
		* @param: buffer 包含要反序列化的数据的字节流缓冲。
		* @param: size 字节流长度。
		* @param: value 布尔类型指针。
		* @return: 如果反序列化成功返回使用的字节流长度，如果失败返回0。
		*/
		template<>
		static unsigned int Deserialize(std::string::const_iterator begin, std::string::const_iterator end, bool* value);

		/**
		* @brief: 数字类型的反序列化。
		* @param: buffer 包含要反序列化的数据的字节流缓冲。
		* @param: size 字节流长度。
		* @param: value 数字指针
		* @return: 如果反序列化成功返回使用的字节流长度，如果失败返回0。
		*/
		template<typename T>
		static unsigned int Deserialize(std::string::const_iterator begin, std::string::const_iterator end, T* value)
		{
			if (static_cast<unsigned int>((end - begin)) < sizeof(T))
			{
				return 0;
			}

			*value = 0;

			*value = (T)(*value | static_cast<unsigned char>(*begin));

			for (int i = 1; i < static_cast<int>(sizeof(T)); ++i)
			{
				*value = (T)(*value << 8);
				*value = (T)(*value | static_cast<unsigned char>(*(begin+i)));
			}
			return sizeof(T);
		}

		/**
		* @brief: 单字节浮点类型的反序列化。
		* @param: buffer 包含要反序列化的数据的字节流缓冲。
		* @param: size 字节流长度。
		* @param: value 单字节浮点类型指针。
		* @return: 如果反序列化成功返回使用的字节流长度，如果失败返回0。
		*/
		template<>
		static unsigned int Deserialize(std::string::const_iterator begin, std::string::const_iterator end, float* value);

		/**
		* @brief: 双字节浮点类型的反序列化。
		* @param: buffer 包含要反序列化的数据的字节流缓冲。
		* @param: size 字节流长度。
		* @param: value 双字节浮点类型指针。
		* @return: 如果反序列化成功返回使用的字节流长度，如果失败返回0。
		*/
		template<>
		static unsigned int Deserialize(std::string::const_iterator begin, std::string::const_iterator end, double* value);

		/**
		* @brief: 字符串浮点类型的反序列化。
		* @param: buffer 包含要反序列化的数据的字节流缓冲。
		* @param: size 字节流长度。
		* @param: value 字符串浮点类型指针。
		* @return: 如果反序列化成功返回使用的字节流长度，如果失败返回0。
		*/
		template<>
		static unsigned int Deserialize(std::string::const_iterator begin, std::string::const_iterator end, std::string* value);

		/**
		* @brief: 列表类型的反序列化。
		* @param: buffer 包含要反序列化的数据的字节流缓冲。
		* @param: size 字节流长度
		* @param: values 列表指针
		* @return: 如果反序列化成功返回使用的使用的字节流长度，如果失败返回0。
		*/
		template<typename T>
		static unsigned int Deserialize(std::string::const_iterator begin, std::string::const_iterator end, std::vector<T>* values)
		{
			unsigned short vSize = 0;
			if (Deserialize(begin, end, &vSize) == 0)
			{
				return 0;
			}

			unsigned int offset = sizeof(unsigned short);
			for (unsigned int i = 0; i < vSize; ++i)
			{
				T item;
				unsigned int temp = Deserialize(begin + offset, end, &item);
				if (temp == 0)
				{
					return 0;
				}
				else
				{
					offset += temp;
				}
				values->push_back(item);
			}
			return offset;
		}

		/**
		* @brief: 集合类型的反序列化。
		* @param: buffer 包含要反序列化的数据的字节流缓冲。
		* @param: size 字节流长度
		* @param: values 集合指针
		* @return: 如果反序列化成功返回使用的使用的字节流长度，如果失败返回0。
		*/
		template<typename T>
		static unsigned int Deserialize(std::string::const_iterator begin, std::string::const_iterator end, std::set<T>* values)
		{
			unsigned short vSize = 0;
			if (Deserialize(begin, end, &vSize) == 0)
			{
				return 0;
			}

			unsigned int offset = sizeof(unsigned short);
			for (unsigned int i = 0; i < vSize; ++i)
			{
				T item;
				unsigned int temp = Deserialize(begin + offset, end, &item);
				if (temp == 0)
				{
					return 0;
				}
				else
				{
					offset += temp;
				}
				values->insert(item);
			}
			return offset;
		}

	private:

		// 用于转换单精度浮点数的联合体
		union Float_Convert
		{
			float F;
			unsigned int I;
		};

		// 用于转换双精度浮点数的联合体
		union Double_Convert
		{
			double D;
			unsigned long long L;
		};
	};
}

