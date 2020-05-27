#pragma once
#include <string>
#include <vector>
#include <map>

#include "StringEx.h"

namespace OnePunchMan
{
	//json序列化
	class JsonSerialization
	{
	public:

		/**
		* @brief: 序列化字段
		* @param: json 用于存放序列化结果的字符串
		* @param: key 字段的键
		* @param: value 字段的值
		*/
		template<typename T>
		static void SerializeValue(std::string* json, const std::string& key, T value)
		{
			if (json->empty())
			{
				json->append("{");
			}
			else
			{
				json->erase(json->end() - 1, json->end());
				json->append(",");
			}
			json->append(StringEx::Combine("\"", key, "\":"));
			ConvertToJson(json, value);
			json->append("}");
		}

		/**
		* @brief: 序列化Json类
		* @param: json 用于存放序列化结果的字符串
		* @param: key 字段的键
		* @param: value 类的json字符串
		*/
		static void SerializeClass(std::string* json, const std::string& key, const std::string& value)
		{
			if (json->empty())
			{
				json->append("{");
			}
			else
			{
				json->erase(json->end() - 1, json->end());
				json->append(",");
			}
			json->append(StringEx::Combine("\"", key, "\":"));
			if (value.empty())
			{
				json->append("{}");
			}
			else
			{
				json->append(value);
			}
			json->append("}");
		}

		/**
		* @brief: 序列化Json数组
		* @param: json 用于存放序列化结果的字符串
		* @param: key 字段的键
		* @param: value 数组的json字符串
		*/
		static void SerializeArray(std::string* json, const std::string& key, const std::string& value)
		{
			if (json->empty())
			{
				json->append("{");
			}
			else
			{
				json->erase(json->end() - 1, json->end());
				json->append(",");
			}
			json->append(StringEx::Combine("\"", key, "\":"));
			if (value.empty())
			{
				json->append("[]");
			}
			else
			{
				json->append(value);
			}
			json->append("}");

		}

		/**
		* @brief: 序列化类数组中的一项
		* @param: json 用于存放序列化结果的字符串
		* @param: value 类的json字符串
		*/
		static void AddClassItem(std::string* json, const std::string& value)
		{
			if (json->empty())
			{
				json->append("[");
			}
			else
			{
				json->erase(json->end() - 1, json->end());
				json->append(",");
			}
			json->append(value);
			json->append("]");
		}

		/**
		* @brief: 序列化值数组中的一项
		* @param: json 用于存放序列化结果的字符串
		* @param: value 值
		*/
		template<typename T>
		static void AddValueItem(std::string* json, T value)
		{
			if (json->empty())
			{
				json->append("[");
			}
			else
			{
				json->erase(json->end() - 1, json->end());
				json->append(",");
			}
			ConvertToJson(json, value);
			json->append("]");
		}
	private:

		/**
		* @brief: 将数字和布尔类转换为json
		* @param: json 用于存放序列化结果的字符串
		* @param: value 字段的值
		*/
		template<typename T>
		static void ConvertToJson(std::string* json, T value)
		{
			json->append(StringEx::ToString(value));
		}

		/**
		* @brief: 将字符串转换为json
		* @param: json 用于存放序列化结果的字符串
		* @param: value 字段的值
		*/
		static void ConvertToJson(std::string* json, const char* value)
		{
			json->append(StringEx::Combine("\"", value, "\""));
		}

		/**
		* @brief: 将字符串转换为json
		* @param: json 用于存放序列化结果的字符串
		* @param: value 字段的值
		*/
		static void ConvertToJson(std::string* json, const std::string& value)
		{
			json->append(StringEx::Combine("\"", value, "\""));
		}

		/**
		* @brief: 将集合转换为json
		* @param: json 用于存放序列化结果的字符串
		* @param: values 字段的值列表
		*/
		template<typename T>
		static void ConvertToJson(std::string* json, const std::vector<T>& values)
		{
			json->append("[");
			for_each(values.begin(), values.end(), [&json](const T& value) {

				ConvertToJson(json, value);
				json->append(",");
				});
			if (!values.empty())
			{
				json->erase(json->end() - 1);
			}
			json->append("]");
		}
	};

	//json反序列化
	class JsonDeserialization
	{
	public:

		/**
		* @brief: 构造函数
		*/
		JsonDeserialization();

		/**
		* @brief: 构造函数
		* @param: json json字符串
		*/
		JsonDeserialization(const std::string& json);

		/**
		* @brief: 反序列化Json
		* @param: json json字符串
		* @param: prefix key前缀
		*/
		void Deserialize(const std::string& json, const std::string& prefix = "");

		/**
		* @brief: 读取json
		* @param: key 键
		* @return: 读取成功返回true
		*/
		template<typename T>
		T Get(const std::string& key) const
		{
			std::string value;
			if (_values.find(key) != _values.end())
			{
				value = _values.at(key);
			}
			return StringEx::Convert<T>(value);
		}

		/**
		* @brief: 读取配置文件
		* @param: key 键
		* @param: t 值
		* @return: 读取成功返回true
		*/
		template<typename T>
		T Get(const std::string& key, T defaultValue) const
		{
			std::string value;
			if (_values.find(key) != _values.end())
			{
				value = _values.at(key);
			}
			return StringEx::Convert<T>(value,defaultValue);
		}
		
		/**
		* @brief: 读取配置文件
		* @param: key 键
		* @param: v 值集合
		*/
		template<typename T>
		std::vector<T> GetArray(const std::string& key) const
		{
			std::string value;
			if (_values.find(key) != _values.end())
			{
				return ConvertToArray<T>(_values.at(key));
			}
			else
			{
				return std::vector<T>();
			}
		}

		/**
		* @brief: 将json字符串转换为集合类型
		* @param: json json字符串
		* @return: 转换成功返回转换结果否则返回空集合
		*/
		template<typename T>
		static std::vector<T> ConvertToArray(const std::string& json)
		{
			std::vector<T> v;
			if (json.size() > 2)
			{
				size_t startIndex = 1;
				while (startIndex < json.size())
				{
					T t;
					size_t result = FindItem(json, &t, startIndex);
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
			}
			
			return v;
		}

		/**
		* @brief: 将json字符串转换为json集合类型
		* @param: json json字符串
		* @return: 转换成功返回转换结果否则返回空集合
		*/
		static std::vector<std::string> ConvertToItems(const std::string& json)
		{
			std::vector<std::string> v;
			if (json.size() > 2)
			{
				size_t startIndex = 1;
				std::tuple<size_t, std::string> valueTuple;
				while (startIndex < json.size())
				{
					if (json[startIndex] == '{')
					{
						valueTuple = CutByTag(json, startIndex, '{', '}');
					}
					else if (json[startIndex] == '[')
					{
						valueTuple = CutByTag(json, startIndex, '[', ']');
					}
					else
					{
						break;
					}
					startIndex += std::get<0>(valueTuple);
					v.push_back(std::get<1>(valueTuple));
				}
			}
			return v;
		}
	
	private:

		/**
		* @brief: 截取字符串。
		* @param: json json字符串
		* @param: offset 从开头略过的字符数
		* @return: 第一个参数表示搜索的长度，第二个参数表示截取结果，成功返回字段的值，如果搜索失败返回空字符串
		*/
		std::tuple<size_t, std::string> CutString(const std::string& json, size_t offset);

		/**
		* @brief: 截取数字。
		* @param: json json字符串
		* @param: offset 从开头略过的字符数
		* @return: 第一个参数表示搜索的长度，第二个参数表示截取结果，成功返回字段的值，如果搜索失败返回空字符串
		*/
		std::tuple<size_t, std::string> CutInteger(const std::string& json, size_t offset);

		/**
		* @brief: 根据开始和结束标记截取。
		* @param: json json字符串
		* @param: offset 从开头略过的字符数
		* @param: head 开始标记
		* @param: tail 结束标记
		* @return: 第一个参数表示搜索的长度，第二个参数表示截取结果，成功返回字段的值，如果搜索失败返回空字符串
		*/
		static std::tuple<size_t, std::string> CutByTag(const std::string& json, size_t offset, char head, char tail);

		/**
		* @brief: 反序列化数组。
		* @param: json json字符串
		* @param: prefix key前缀
		*/
		void DeserializeArray(const std::string& json, const std::string& prefix = "");

		/**
		* @brief: 从字符串中截取数字数组或布尔数组中的一项。
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
		* @brief: 从字符串中截取字符串数组中的一项。
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

		//反序列化的结果
		std::map<std::string, std::string> _values;
	};
}

