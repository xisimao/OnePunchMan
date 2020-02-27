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
		* @brief: 序列化Json
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
		* @brief: 反序列化字段。
		* @param: json json字符串
		* @param: key 字段键
		* @param: t 字段值的指针
		*/
		template<typename T>
		static void Deserialize(const std::string& json, const std::string& key,T* t)
		{
			std::string pattern = "\"" + key + "\":";
			size_t index = json.find(pattern);
			if (index != std::string::npos)
			{
				DeserializeValue(json, t, index + pattern.size());
			}
		}

		/**
		* @brief: 反序列化字段的值为列表。
		* @param: json json字符串
		* @param: v 字段值列表的指针
		* @param: offset 从开头略过的字符数
		*/
		template<typename T>
		static void DeserializeValue(const std::string& json, std::vector<T>* v,size_t offset=0)
		{
			std::string arrayValue;
			DeserializeValue(json, &arrayValue, offset);
			if (!arrayValue.empty())
			{
				offset = 1;
				while (true)
				{
					std::string value;
					offset += DeserializeValue(arrayValue, &value, offset);
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
			}
		}

		/**
		* @brief: 反序列化字段的值为集合。
		* @param: json json字符串
		* @param: s 字段值集合的指针
		* @param: offset 从开头略过的字符数
		*/
		template<typename T>
		static void DeserializeValue(const std::string& json, std::set<T>* s, size_t offset = 0)
		{
			std::string arrayValue;
			DeserializeValue(json, &arrayValue, offset);
			if (!arrayValue.empty())
			{
				offset = 1;
				while (true)
				{
					std::string value;
					offset += DeserializeValue(arrayValue, &value, offset);
					if (value.empty())
					{
						break;
					}
					else
					{
						T t;
						StringEx::Convert(value, &t);
						s->insert(t);
					}
				}
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
		* @brief: 反序列化字段的值为指定类型。
		* @param: json json字符串
		* @param: t 字段值的指针
		* @param: offset 从开头略过的字符数
		* @return: 解析成功返回解析的总字符数，否则返回0
		*/
		template<typename T>
		static size_t DeserializeValue(const std::string& json, T* t, size_t offset = 0)
		{
			size_t tagIndex = json.find_first_not_of(" ,\t", offset);
			std::string value;
			if (tagIndex != std::string::npos)
			{
				char tag = json[tagIndex];
				if (tag == '"')
				{
					value = DeserializeString(json, tagIndex);
				}
				else if (tag == '{')
				{
					value = DeserializeByTag(json, tagIndex, '{', '}');
				}
				else if (tag == '[')
				{
					value = DeserializeByTag(json, tagIndex, '[', ']');
				}
				else
				{
					value = DeserializeInteger(json, tagIndex);
				}
			}
			if (value.empty())
			{
				return 0;
			}
			else
			{
				StringEx::Convert(value, t);
				return tagIndex - offset+value.size();
			}
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
			int headCount = 0;
			int tailCount = 0;
			std::string findHead;
			findHead.push_back(head);
			offset = json.find(findHead.c_str(), offset);
			if (offset != std::string::npos)
			{
				for (std::string::const_iterator it = json.begin() + offset; it < json.end(); ++it)
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
							return json.substr(offset, it - json.begin() - offset + 1);
						}
					}
				}
			}

			return std::string();
		}

		/**
		* @brief: 截取字符串
		* @param: json json字符串
		* @param: offset 从开头略过的字符数
		* @return: 字段的值，如果搜索失败返回空字符串
		*/
		static std::string DeserializeString(const std::string& json, size_t offset)
		{
			offset = json.find('"', offset);
			size_t endIndex = offset;
			std::string temp;
			while (true)
			{
				endIndex = json.find('"', endIndex + 1);
				if (endIndex == std::string::npos)
				{
					break;
				}
				else
				{
					if (json[endIndex - 1] == '\\')
					{
						temp.append(json.substr(offset + 1, endIndex - offset - 2));
						offset = endIndex - 1;
					}
					else
					{

						temp.append(json.substr(offset + 1, endIndex - offset - 1));
						endIndex += 1;
						break;
					}
				}
			}
			return temp;
		}

		/**
		* @brief: 截取数字
		* @param: json json字符串
		* @param: offset 从开头略过的字符数
		* @return: 字段的值，如果搜索失败返回空字符串
		*/
		static std::string DeserializeInteger(const std::string& json, size_t offset)
		{
			const std::string Tail = ",}]";
			size_t endIndex = json.find_first_of(Tail, offset);
			if (endIndex == std::string::npos)
			{
				return std::string();
			}
			else
			{
				return json.substr(offset, endIndex - offset);
			}
		}
	};
}

