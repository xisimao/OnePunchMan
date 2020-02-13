#pragma once
#include <string>
#include <vector>
#include <set>
#include <sstream>

#include "Text.h"

namespace Saitama
{
	//二进制序列化
	class JsonFormatter
	{
	public:

		/**
		* @brief: 递归序列化Json。
		* @param: json 用于存放序列化结果的字符串
		* @param: t 当前递归到的序列化的第一项
		* @param: ...u 序列化的其他项
		*/
		template<typename T, typename ...U>
		static void Serialize(std::string* json, const std::tuple<std::string, T>& t, const std::tuple<std::string, U> ...u)
		{
			json->append("{");
			SerializeItem(json,t);
			SerializeItem(json,u...);
			if (!json->empty() && json->at(json->size() - 1) == ',')
			{
				json->erase(json->end() - 1, json->end());
			}
			json->append("}");
		}

		/**
		* @brief: 序列化Json
		* @param: json 用于存放序列化结果的字符串
		* @param: t 第一项表示key，第二项表示value
		*/
		template<typename T>
		static void Serialize(std::string* json, const std::tuple<std::string, T>& t)
		{
			json->append("{");
			SerializeItem(json,t);
			if (!json->empty() && json->at(json->size() - 1) == ',')
			{
				json->erase(json->end() - 1, json->end());
			}
			json->append("}");
		}

		/**
		* @brief: 序列化数组的Json
		* @param: json 用于存放序列化结果的字符串
		* @param: v 实现了ToJson函数的列表
		*/
		template<typename T>
		static void Serialize(std::string* json, const std::vector<T>& v)
		{
			json->append("[");
			for_each(v.begin(), v.end(), [&json](const T& t) {

				json->append(t.ToJson());
				json->append(",");

			});
			if (!json->empty() && json->at(json->size() - 1) == ',')
			{
				json->erase(json->end() - 1, json->end());
			}
			json->append("]");
		}

		/**
		* @brief: 递归反序列化可变参数。
		* @param: json json字符串
		* @param: t 当前递归到的反序列化的第一项
		* @param: ...u 反序列化的其他项
		*/
		template<typename T, typename ...U>
		static void Deserialize(const std::string& json, const std::tuple<std::string, T*>& t, const std::tuple<std::string, U*>& ... u)
		{
			Deserialize(json, t);
			Deserialize(json, u...);
		}

		/**
		* @brief: 反序列化字段。
		* @param: json json字符串
		* @param: t 字段的键和值指针
		*/
		template<typename T>
		static void Deserialize(const std::string& json, const std::tuple<std::string, T*>& t)
		{
			const std::string& key = std::get<0>(t);
			T* value = std::get<1>(t);
			std::string pattern = "\"" + key + "\":";
			size_t start = json.find(pattern);
			if (start != std::string::npos)
			{
				DeserializeValue(json, start+pattern.size(), value);
			}
		}

	private:

		/**
		* @brief: 递归序列化一个字段
		* @param: json 用于存放序列化结果的字符串
		* @param: t 当前递归到的序列化的第一项
		* @param: ...u 序列化的其他项
		*/
		template<typename T, typename ...U>
		static void SerializeItem(std::string* json, const std::tuple<std::string, T>& t, const std::tuple<std::string, U> ...u)
		{
			const std::string& key = std::get<0>(t);
			const T& value = std::get<1>(t);
			SerializeKey(json,key);
			SerializeValue(json, value);
			SerializeItem(json, u...);
		}

		/**
		* @brief: 递归序列化一个字段
		* @param: json 用于存放序列化结果的字符串
		* @param: t 当前递归到的序列化
		*/
		template<typename T>
		static void SerializeItem(std::string* json, const std::tuple<std::string, T>& t)
		{
			const std::string& key = std::get<0>(t);
			const T& value = std::get<1>(t);
			SerializeKey(json,key);
			SerializeValue(json,value);
		}

		/**
		* @brief: 序列化字段的键
		* @param: json 用于存放序列化结果的字符串
		* @param: key 键
		*/
		static void SerializeKey(std::string* json, const std::string& key)
		{
			std::stringstream ss;
			ss << "\"" << key << "\":";
			json->append(ss.str());
		}

		/**
		* @brief: 序列化字段的值
		* @param: json 用于存放序列化结果的字符串
		* @param: value 值
		*/
		template<typename T>
		static void SerializeValue(std::string* json, const T& value)
		{
			std::stringstream ss;
			ss << value<<",";
			json->append(ss.str());
		}

		/**
		* @brief: 序列化字段的字符串值
		* @param: json 用于存放序列化结果的字符串
		* @param: value 字符串值
		*/
		static void SerializeValue(std::string* json, const std::string& value)
		{
			std::stringstream ss;
			if (value.empty())
			{
				ss << "\"" << value << "\",";
			}
			else
			{
				if ((value[0] == '{'&&value[value.size() - 1] == '}') ||
					(value[0] == '['&&value[value.size() - 1] == ']'))
				{
					ss << value << ",";
				}
				else
				{
					ss << "\"" << value << "\",";
				}
			}
			json->append(ss.str());
		}

		/**
		* @brief: 序列化字段的列表值
		* @param: json 用于存放序列化结果的字符串
		* @param: value 列表值
		*/
		template<typename T>
		static void SerializeValue(std::string* json, const std::vector<T>& values)
		{
			json->append("[");
			for_each(values.begin(), values.end(), [&json](const T& t) {

				SerializeValue(json, t);
				json->append(",");
			});
			if (!values.empty())
			{
				json->erase(json->end() - 1);
			}
			json->append("],");
		}

		/**
		* @brief: 序列化字段的集合值
		* @param: json 用于存放序列化结果的字符串
		* @param: value 集合值
		*/
		template<typename T>
		static void SerializeValue(std::string* json, const std::set<T>& values)
		{
			json->append("[");
			for_each(values.begin(), values.end(), [&json](const T& t) {

				SerializeValue(json, t);
				json->append(",");
			});
			if (!values.empty())
			{
				json->erase(json->end() - 1);
			}
			json->append("],");
		}

		/**
		* @brief: 反序列化字段的值。
		* @param: json json字符串
		* @param: start 数字的开始位置，例如123的开始位置为1的位置
		* @param: t 值的指针
		* @return: 数字结束的位置,结束符号,]}的位置
		*/
		template<typename T>
		static size_t DeserializeValue(const std::string& json,size_t start, T* t)
		{
			const std::string Tail = ",}]";
			size_t end = json.find_first_of(Tail, start);
			if (end != std::string::npos)
			{
				Text::Convert<T>(json.substr(start, end - start), t);
			}
			return end;
		}

		/**
		* @brief: 反序列化字段的值。
		* @param: json json字符串
		* @param: start 字符串起始位置，开始"的位置
		* @param: t 值的指针
		* @return: 字符串结束的位置，结束"的位置+1
		*/
		static size_t DeserializeValue(const std::string& json, size_t start, std::string* t)
		{	
			start = json.find('"', start);
			size_t end = start;
			std::string temp;
			while(true)
			{
				end = json.find('"', end + 1);
				if (end == std::string::npos)
				{
					break;
				}
				else
				{
					if (json[end - 1] == '\\')
					{
						temp.append(json.substr(start + 1, end - start - 2));
						start = end - 1;
					}
					else
					{
		
						temp.append(json.substr(start + 1, end - start - 1));
						t->assign(temp);
						end += 1;
						break;
					}
				}
			}
			return end;
		}

		/**
		* @brief: 反序列化列表字段的值。
		* @param: json json字符串
		* @param: start 值的起始位置
		* @param: t 列表值的指针
		*/
		template<typename T>
		static void DeserializeValue(const std::string& json,size_t start, std::vector<T>* vt)
		{	
			start = json.find("[",start);
			size_t end = json.find("]",start+1);
			size_t index = start+1;
			while (index != std::string::npos&&index<end)
			{
				T t;
				index = DeserializeValue(json, index, &t);
				if (index != std::string::npos)
				{
					vt->push_back(t);
				}
				index = index+1;
			}
		}

		/**
		* @brief: 搜索集合字段的值。
		* @param: json json字符串
		* @param: start 值的起始位置
		* @param: t 集合值的指针
		*/
		template<typename T>
		static void DeserializeValue(const std::string& json,size_t start, std::set<T>* st)
		{
			start = json.find("[", start);
			size_t end = json.find("]", start + 1);
			size_t index = start + 1;
			while (index != std::string::npos&&index < end)
			{
				T t;
				index = DeserializeValue(json, index, &t);
				if (index != std::string::npos)
				{
					st->insert(t);
				}
				index = index + 1;
			}
		}
	};
}

