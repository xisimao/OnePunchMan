#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "StringEx.h"
#include "JsonFormatter.h"

namespace OnePunchMan
{
	//读取文件配置文件
	class FileConfig
	{
	public:
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
			return _jd.Get<T>(key);
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
			return _jd.Get<T>(key, defaultValue);
		}

	private:

		//配置项
		JsonDeserialization _jd;
	};

}

