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
		* @param: t 值
		* @return: 读取成功返回true
		*/
		template<typename T>
		bool ReadConfig(const std::string& key, T* t) const
		{
			if (_configs.find(key) == _configs.end())
			{
				return false;
			}
			else
			{
				const std::string value = _configs.at(key);
				return StringEx::Convert<T>(value, t);
			}
		}

		/**
		* @brief: 读取配置文件
		* @param: key 键
		* @param: v 值集合
		*/
		template<typename T>
		bool ReadConfigs(const std::string& key, std::vector<T>* v) const
		{
			if (_configs.find(key) == _configs.end())
			{
				return false;
			}
			else
			{
				const std::string value = _configs.at(key);
				size_t start = 0;
				size_t end = 0;
				do
				{
					end = value.find(',', start);
					T t;
					StringEx::Convert<T>(value.substr(start, end - start), &t);
					v->push_back(t);
					start = end + 1;

				} while (end != std::string::npos);
				return true;
			}
		}

	private:

		//配置项
		std::map<std::string, std::string> _configs;
	};

}

