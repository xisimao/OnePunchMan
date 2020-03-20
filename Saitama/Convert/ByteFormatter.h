#pragma once
#include <string>
#include <vector>
#include <set>
#include <string.h>

namespace Saitama
{
	//字符数组二进制序列化
	class ByteFormatter
	{
	public:

		/**
		* @brief: 递归序列化可变参数。
		* @param: buffer 格式化程序在其中放置序列化数据的字节流。
		* @param: capacity 字节流容量。
		* @param: t 当前递归到的序列化的第一项
		* @param: ...u 序列化的其他项
		* @return: 如果序列化成功返回字节流长度，如果失败返回0
		*/
		template<typename T, typename ...U>
		static unsigned int Serialize(char* buffer, unsigned int capacity, const T& t, U ...u)
		{
			unsigned int size = Serialize(buffer, capacity, t);
			if (size == 0)
			{
				return 0;
			}
			else
			{
				unsigned int temp=Serialize(buffer + size, capacity - size, u...);
				if (temp == 0)
				{
					return 0;
				}
				else
				{
					return size + temp;
				}
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
		static unsigned int Deserialize(const char* buffer, unsigned int size, T* t, U... u)
		{
			unsigned int dSize = Deserialize(buffer, size, t);
			if (dSize == 0)
			{
				return 0;
			}
			else
			{
				unsigned int temp=Deserialize(buffer + dSize, size - dSize, u...);
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
		* @brief: 布尔类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字节流。
		* @param: capacity 字节流容量。
		* @param: value 布尔类型
		* @return: 如果序列化成功返回字节流长度，如果失败返回0。
		*/
		static unsigned int Serialize(char* buffer, unsigned int capacity, const bool& value);

		/**
		* @brief: 数字类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字节流。
		* @param: capacity 字节流容量。
		* @param: value 数字。
		* @return: 如果序列化成功返回字节流长度，如果失败返回0。
		*/
		template<typename T>
		static unsigned int Serialize(char* buffer, unsigned int capacity, const T& value)
		{
			if (capacity < sizeof(T))
			{
				return 0;
			}
			for (int i = 0; i <static_cast<int>(sizeof(T)); ++i)
			{
				buffer[i] = static_cast<char>(value >> 8 * (sizeof(T) - i - 1));
			}
			return sizeof(T);
		}

		/**
		* @brief: 单字节浮点类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字节流。
		* @param: capacity 字节流容量。
		* @param: value 单字节浮点类型
		* @return: 如果序列化成功返回字节流长度，如果失败返回0。
		*/
		static unsigned int Serialize(char* buffer, unsigned int capacity, const float& value);

		/**
		* @brief: 双字节浮点类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字节流。
		* @param: capacity 字节流容量。
		* @param: value 双字节浮点类型
		* @return: 如果序列化成功返回字节流长度，如果失败返回0。
		*/
		static unsigned int Serialize(char* buffer, unsigned int capacity, const double& value);

		/**
		* @brief: 字符串类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字节流。
		* @param: capacity 字节流容量。
		* @param: value 字符串
		* @return: 如果序列化成功返回字节流长度，如果失败返回0。
		*/
		static unsigned int Serialize(char* buffer, unsigned int capacity, const std::string& value);

		/**
		* @brief: 列表类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字节流。
		* @param: capacity 字节流容量
		* @param: values 列表
		* @return: 如果序列化成功返回字节流长度，如果失败返回0。
		*/
		template<typename T>
		static unsigned int Serialize(char* buffer, unsigned int capacity, const std::vector<T>& values)
		{
			unsigned int offset = sizeof(unsigned short);
			if (Serialize(buffer, capacity, static_cast<unsigned short>(values.size())) == 0)
			{
				return 0;
			}

			for (unsigned int i = 0; i < values.size(); ++i)
			{
				unsigned int size=Serialize(buffer + offset, capacity - offset, values.at(i));
				if (size == 0)
				{
					return 0;
				}
				else
				{
					offset += size;
				}
			}
			return offset;
		}

		/**
		* @brief: 集合类型的序列化。
		* @param: buffer 格式化程序在其中放置序列化数据的字节流。
		* @param: capacity 字节流容量
		* @param: values 集合
		* @return: 如果序列化成功返回字节流长度，如果失败返回0。
		*/
		template<typename T>
		static unsigned int Serialize(char* buffer, unsigned int capacity, const std::set<T>& values)
		{
			unsigned int offset = sizeof(unsigned short);
			if (Serialize(buffer, capacity, static_cast<unsigned short>(values.size())) == 0)
			{
				return 0;
			}
			for(typename std::set<T>::const_iterator it=values.begin();it!=values.end();++it)
			{
				unsigned int size=Serialize(buffer + offset, capacity - offset, *it);
				if (size == 0)
				{
					return 0;
				}
				else
				{
					offset += size;
				}
			}
			return offset;
		}

		/**
		* @brief: 布尔类型的反序列化。
		* @param: buffer 包含要反序列化的数据的字节流缓冲。
		* @param: size 字节流长度。
		* @param: value 布尔类型指针。
		* @return: 如果反序列化成功返回使用的字节流长度，如果失败返回0。
		*/
		static unsigned int Deserialize(const char* buffer, unsigned int size, bool* value);

		/**
		* @brief: 数字类型的反序列化。
		* @param: buffer 包含要反序列化的数据的字节流缓冲。
		* @param: size 字节流长度。
		* @param: value 数字指针
		* @return: 如果反序列化成功返回使用的字节流长度，如果失败返回0。
		*/
		template<typename T>
		static unsigned int Deserialize(const char* buffer, unsigned int size, T* value)
		{
			if (size < sizeof(T))
			{
				return 0;
			}

			*value = 0;

			*value = (T)(*value | static_cast<unsigned char>(buffer[0]));

			for (int i = 1; i < static_cast<int>(sizeof(T)); ++i)
			{
				*value = (T)(*value << 8);
				*value = (T)(*value | static_cast<unsigned char>(buffer[i]));
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
		static unsigned int Deserialize(const char* buffer, unsigned int size, float* value);

		/**
		* @brief: 双字节浮点类型的反序列化。
		* @param: buffer 包含要反序列化的数据的字节流缓冲。
		* @param: size 字节流长度。
		* @param: value 双字节浮点类型指针。
		* @return: 如果反序列化成功返回使用的字节流长度，如果失败返回0。
		*/
		static unsigned int Deserialize(const char* buffer, unsigned int size, double* value);

		/**
		* @brief: 字符串浮点类型的反序列化。
		* @param: buffer 包含要反序列化的数据的字节流缓冲。
		* @param: size 字节流长度。
		* @param: value 字符串浮点类型指针。
		* @return: 如果反序列化成功返回使用的字节流长度，如果失败返回0。
		*/
		static unsigned int Deserialize(const char* buffer, unsigned int size, std::string* value);

		/**
		* @brief: 列表类型的反序列化。
		* @param: buffer 包含要反序列化的数据的字节流缓冲。
		* @param: size 字节流长度
		* @param: values 列表指针
		* @return: 如果反序列化成功返回使用的使用的字节流长度，如果失败返回0。
		*/
		template<typename T>
		static unsigned int Deserialize(const char* buffer, unsigned int size, std::vector<T>* values)
		{
			unsigned short vSize = 0;
			if (Deserialize(buffer, size, &vSize) == 0)
			{
				return 0;
			}

			unsigned int offset = sizeof(unsigned short);
			for (unsigned int i = 0; i < vSize; ++i)
			{
				T item;
				unsigned int temp= Deserialize(buffer + offset, size - offset, &item);
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
		static unsigned int Deserialize(const char* buffer, unsigned int size, std::set<T>* values)
		{
			unsigned short vSize = 0;
			if (Deserialize(buffer, size, &vSize) == 0)
			{
				return 0;
			}

			unsigned int offset = sizeof(unsigned short);
			for (unsigned int i = 0; i < vSize; ++i)
			{
				T item;
				unsigned int temp= Deserialize(buffer + offset, size - offset, &item);
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

