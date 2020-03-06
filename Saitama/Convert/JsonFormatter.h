#pragma once
#include <string>
#include <vector>
#include <set>

#include "StringEx.h"

namespace Saitama
{
	//json序列化
	class JsonFormatter
	{
	public:

		/**
		* @brief: 序列化字段
		* @param: json 用于存放序列化结果的字符串
		* @param: key 字段的键
		* @param: value 字段的值
		*/
		template<typename T>
		static void Serialize(std::string* json, const std::string& key,const T& value)
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
			SerializeValue(json, value);
			json->append("}");
		}

		/**
		* @brief: 序列化Json字符串
		* @param: json 用于存放序列化结果的字符串
		* @param: key 字段的键
		* @param: value 字段的值
		*/
		static void SerializeJson(std::string* json, const std::string& key, const std::string& value)
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
			json->append(value);
			json->append("}");
		}

		/**
		* @brief: 序列化数组中的一项
		* @param: json 用于存放序列化结果的字符串
		* @param: key 字段的键
		* @param: value 字段的值
		*/
		static void SerializeItem(std::string* json, const std::string& item)
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
			json->append(item);
			json->append("]");
		}

		/**
		* @brief: 反序列化字段。
		* @param: json json字符串
		* @param: key 字段键
		* @param: t 字段值的指针
		*/
		template<typename T>
		static bool Deserialize(const std::string& json, const std::string& key,T* t)
		{
			std::string pattern = "\"" + key + "\":";
			size_t index = json.find(pattern);
			if (index == std::string::npos)
			{
				return false;
			}
			else
			{
				return DeserializeValue(json, t, index + pattern.size());
			}
		}

		/**
		* @brief: 反序列化字段的值为列表。
		* @param: json json字符串
		* @param: v 字段值列表的指针
		* @param: offset 从开头略过的字符数
		*/
		static std::string DeserializeJson(const std::string& json, const std::string& key)
		{
			std::string pattern = "\"" + key + "\":";
			size_t index = json.find(pattern);
			if (index == std::string::npos)
			{
				return std::string();
			}	
			else
			{
				return DeserializeByTag(json, index, '{', '}');
			}
		}

		/**
		* @brief: 反序列化字段的值为列表。
		* @param: json json字符串
		* @param: v 字段值列表的指针
		* @param: offset 从开头略过的字符数
		*/
		static std::vector<std::string> DeserializeJsons(const std::string& json, const std::string& key)
		{
			std::string pattern = "\"" + key + "\":";
			size_t index = json.find(pattern);
			if (index == std::string::npos)
			{
				return std::vector<std::string>();
			}
			else
			{
				std::string arrayValue = DeserializeByTag(json, index, '[', ']');
				if (arrayValue.empty())
				{
					return std::vector<std::string>();
				}
				else
				{
					std::vector<std::string> v;
					size_t offset = 1;
					while (true)
					{
						std::string itemValue = DeserializeByTag(arrayValue, offset, '{', '}');
						if (itemValue.empty())
						{
							return v;
						}
						else
						{
							offset += itemValue.size();
							v.push_back(itemValue);
						}
					}
				}
			}
		}

		/**
		* @brief: 反序列化字段的值为指定类型。
		* @param: json json字符串
		* @param: t 字段值的指针
		* @param: offset 从开头略过的字符数
		* @return: 解析成功返回解析的总字符数，否则返回0
		*/
		template<typename T>
		static bool DeserializeValue(const std::string& json, T* t, size_t offset)
		{
			const std::string Tail = ",}]";
			size_t endIndex = json.find_first_of(Tail, offset);
			if (endIndex == std::string::npos)
			{
				return false;
			}
			else
			{
				return StringEx::Convert(json.substr(offset, endIndex - offset), t);
			}
		}

		/**
		* @brief: 反序列化字段的值为指定类型。
		* @param: json json字符串
		* @param: t 字段值的指针
		* @param: offset 从开头略过的字符数
		* @return: 解析成功返回解析的总字符数，否则返回0
		*/
		template<>
		static bool DeserializeValue(const std::string& json, std::string* t, size_t offset)
		{
			size_t startIndex = json.find('"', offset);
			if (startIndex == std::string::npos)
			{
				return false;
			}
			else
			{
				const std::string Tail = ",}]";
				size_t endIndex = json.find_first_of(Tail, offset);
				if (endIndex == std::string::npos)
				{
					return false;
				}
				else
				{
					endIndex = json.substr(startIndex, endIndex - startIndex).find_last_of('"');
					*t = json.substr(startIndex + 1, endIndex - 1);
					return true;
				}
			}
		}

		/**
		* @brief: 反序列化字段的值为指定类型。
		* @param: json json字符串
		* @param: t 字段值的指针
		* @param: offset 从开头略过的字符数
		* @return: 解析成功返回解析的总字符数，否则返回0
		*/
		template<typename T>
		static bool DeserializeValue(const std::string& json, std::vector<T>* v, size_t offset)
		{
			size_t startIndex = json.find('[', offset);
			if (startIndex == std::string::npos)
			{
				return false;
			}
			else
			{
				while (true)
				{
					std::string value;
					startIndex += DeserializeValue(json, &value, startIndex);
					if (value.empty())
					{
						break;
					}
					else
					{
						T t;
						StringEx::Convert(value, &t);
						v->push_back(t);
					}
				}
				return true;
			}
		}

	private:

		/**
		* @brief: 序列化数字和布尔类
		* @param: json 用于存放序列化结果的字符串
		* @param: value 字段的值
		*/
		template<typename T>
		static void SerializeValue(std::string* json,const T& value)
		{
			json->append(StringEx::ToString(value));
		}

		/**
		* @brief: 序列化字符串
		* @param: json 用于存放序列化结果的字符串
		* @param: value 字段的值
		*/
		static void SerializeValue(std::string* json, const char* value)
		{
			json->append(StringEx::Combine("\"", value, "\""));
		}

		/**
		* @brief: 序列化字符串
		* @param: json 用于存放序列化结果的字符串
		* @param: value 字段的值
		*/
		static void SerializeValue(std::string* json, const std::string& value)
		{
			json->append(StringEx::Combine("\"", value, "\""));
		}

		/**
		* @brief: 序列化列表
		* @param: json 用于存放序列化结果的字符串
		* @param: values 字段的值列表
		*/
		template<typename T>
		static void SerializeValue(std::string* json, const std::vector<T>& values)
		{
			json->append("[");
			for_each(values.begin(), values.end(), [&json](const T& value) {

				SerializeValue(json, value);
				});
			if (!values.empty())
			{
				json->erase(json->end() - 1);
			}
			json->append("]");
		}

		/**
		* @brief: 序列化集合
		* @param: json 用于存放序列化结果的字符串
		* @param: values 字段的值集合
		*/
		template<typename T>
		static void SerializeValue(std::string* json, const std::set<T>& values)
		{
			json->append("[");
			for_each(values.begin(), values.end(), [&json](const T& value) {

				SerializeValue(json, value);
				});
			if (!values.empty())
			{
				json->erase(json->end() - 1);
			}
			json->append("]");
		}

		/**
		* @brief: 根据标记截取，适合类和数组。
		* @param: json json字符串
		* @param: offset 从开头略过的字符数
		* @param: head 开始标记
		* @param: tail 结束标记
		* @return: 字段的值，如果搜索失败返回空字符串
		*/
		static std::string DeserializeByTag(const std::string& json, size_t offset, char head,char tail)
		{
			size_t startIndex = json.find(head, offset);
			if (startIndex == std::string::npos)
			{
				return std::string();
			}
			else
			{
				int headCount = 0;
				int tailCount = 0;
				for (std::string::const_iterator it = json.begin() + startIndex; it < json.end(); ++it)
				{
					if (*it == head)
					{
						headCount += 1;
					}
					else if (*it == tail)
					{
						tailCount += 1;
						if (headCount == tailCount)
						{
							return json.substr(startIndex, it - json.begin() - startIndex + 1);
						}
					}
				}
				return std::string();
			}

		
		}

	};
}

