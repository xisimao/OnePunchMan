#pragma once
#include <string>
#ifdef _WIN32 
#include <direct.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32 
#define getcwd(buffer,capcacity) _getcwd(buffer,capcacity)
#endif

namespace Saitama
{
	// 对包含文件或目录路径信息的执行操作
	class Path
	{
	public:

		/**
		* @brief: 获取当前程序的目录
		* @return: 当前程序的目录
		*/
		static std::string GetCurrentPath();

		/**
		* @brief: 获取文件名
		* @param: path 路径字符串
		* @return: 获取成功时返回文件名，失败时返回空字符串
		*/
		static std::string GetFileName(const std::string& path);

		/**
		* @brief: 获取文件后缀名
		* @param: path 路径字符串
		* @return: 获取成功时返回后缀名，失败时返回空字符串
		*/
		static std::string GetExtension(const std::string& path);

		//路径分隔符
		static const char Separator;

	};
}

