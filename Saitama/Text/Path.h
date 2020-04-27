#pragma once
#include <string>
#include <sstream>
#ifdef _WIN32 
#include <direct.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32 
#define getcwd(buffer,capcacity) _getcwd(buffer,capcacity)
#endif

namespace OnePunchMan
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

		/**
		* @brief: 将多个元素组合成一个路径。
		* @param: t 由路径的各部分构成的元素
		* @param: u... 由路径的各部分构成的元素
		* @return: 已组合的路径。
		*/
		template<typename T, typename ...U>
		static std::string Combine(T t, U ...u)
		{
			std::stringstream ss;
			Combine(&ss, t, u...);
			return ss.str();
		}

		/**
		* @brief: 将多个元素组合成一个路径。
		* @param: t 由路径的各部分构成的元素。
		* @param: u... 由路径的各部分构成的元素。
		*/
		template<typename T, typename ...U>
		static void Combine(std::stringstream* ss, T t, U ...u)
		{
			(*ss) << t;
			std::string path = ss->str();
			if (!path.empty()&&*path.rbegin() != Separator)
			{
				(*ss) << Separator;
			}

			Combine(ss, u...);
		}

		/**
		* @brief: 将最后一个元素拼接到路径上，该方法不会增加目录分隔符。
		* @param: t 组成路径的最后一个的元素
		*/
		template<typename T>
		static void Combine(std::stringstream* ss, T t)
		{
			(*ss) << t;
		}

		//路径分隔符
		static const char Separator;

	};
}

