#pragma once
#include <string>
#include <vector>
#include <map>

#include "StringEx.h"

namespace Saitama
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
		static void Serialize(std::string* json, const std::string& key, T value)
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
		* @brief: 序列化Json字符串
		* @param: json 用于存放序列化结果的字符串
		* @param: key 字段的键
		* @param: value 字段的值
		*/
		static void SerializeJsons(std::string* json, const std::string& key, const std::string& value)
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

				SerializeValue(json, value);
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

		JsonDeserialization(const std::string& json);

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
				value = _values.at(key);
				return StringEx::ConvertToArray<T>(value.substr(1, value.size() - 2));
			}
			else
			{
				return std::vector<T>();
			}
			
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
		std::tuple<size_t, std::string> CutByTag(const std::string& json, size_t offset, char head, char tail);

		/**
		* @brief: 反序列化数组。
		* @param: json json字符串
		* @param: prefix key前缀
		*/
		void DeserializeArray(const std::string& json, const std::string& prefix = "");

		/**
		* @brief: 反序列化Json
		* @param: json json字符串
		* @param: prefix key前缀
		*/
		void Deserialize(const std::string& json, const std::string& prefix = "");

		//反序列化的结果
		std::map<std::string, std::string> _values;
	};
}

