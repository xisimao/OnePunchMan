#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "StringEx.h"

namespace Saitama
{
	//读取文件配置文件
	class FileConfig
	{
	public:

		/**
		* @brief: 构造函数
		*/
		FileConfig();

		/**
		* @brief: 构造函数
		* @param: filePath 文件路径
		*/
		FileConfig(const std::string& filePath);

		/**
		* @brief: 读取配置文件
		* @param: key 键
		* @return:读取成功返回读取结果，否则返回0或者空字符串
		*/
		template<typename T>
		T Get(const std::string& key) const
		{
			std::string value;
			if (_configs.find(key) != _configs.end())
			{
				value = _configs.at(key);
			}
			return StringEx::Convert<T>(value);
		}

		/**
		* @brief: 读取配置文件
		* @param: key 键
		* @param: t 读取失败时返回的默认值
		* @return: 读取成功返回读取结果，否则返回默认值
		*/
		template<typename T>
		T Get(const std::string& key, T defaultValue) const
		{
			std::string value;
			if (_configs.find(key) != _configs.end())
			{
				value = _configs.at(key);
			}
			return StringEx::Convert<T>(value, defaultValue);
		}


		/**
		* @brief: 读取配置文件
		* @param: key 键
		* @return: 读取成功返回读取结果，否则返回空集合
		*/
		template<typename T>
		std::vector<T> GetArray(const std::string& key) const
		{
			std::string value;
			if (_configs.find(key) != _configs.end())
			{
				value = _configs.at(key);
			}
			return StringEx::ConvertToArray<T>(value);
		}

	private:

		//配置项
		std::map<std::string, std::string> _configs;
	};

}

