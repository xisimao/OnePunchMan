#pragma once
#include <vector>
#include <string>

#include "LogPool.h"

#ifdef _WIN32
#define popen(cmd,mode) _popen(cmd,mode)
#define pclose(file) _pclose(file)
#endif 

namespace OnePunchMan
{
	//命令行
	class Command
	{
	public:
		
		/**
		* @brief: 执行命令
		* @param: cmd 命令
		* @return: 执行结果
		*/
		static std::string Execute(const std::string& cmd);

		/**
		* @brief: 替换命令中的正则表达式
		* @param: cmd 命令
		* @return: 替换后的字符串
		*/
		static std::string ReplacePattern(const std::string& cmd);

		/**
		* @brief: 替换命令中的转义字符
		* @param: cmd 命令
		* @return: 替换后的字符串
		*/
		static std::string RepleaceEscape(const std::string& cmd);
	};

}
