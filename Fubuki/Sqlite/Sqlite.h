#pragma once
#include <string>
#include <iostream>

#include "sqlite3.h"
#include "LogPool.h"

namespace OnePunchMan
{
	//sqlite查询
	class SqliteReader
	{
	public:

		/**
		* @brief: 构造函数
		* @param: filePath 数据库文件路径
		*/
		SqliteReader(const std::string& filePath);

		/**
		* @brief: 析构函数
		*/
		~SqliteReader();

		/**
		* @brief: 查询语句
		* @param: sql sql语句
		* @return: 查询结果
		*/
		bool BeginQuery(const std::string& sql);

		/**
		* @brief: 当前行是否有值
		* @return: 返回true表示当前行有值
		*/
		bool HasRow();

		/**
		* @brief: 读取字符串列
		* @param: index 列序号
		* @return: 列的字符串值
		*/
		std::string GetString(int index) const;

		/**
		* @brief: 读取32位数字列
		* @param: index 列序号
		* @return: 列的数字值
		*/
		int GetInt(int index) const;

		/**
		* @brief: 读取64位数字列
		* @param: index 列序号
		* @return: 列的数字值
		*/
		long long GetLong(int index) const;

		/**
		* @brief: 结束查询
		*/
		void EndQuery();

		/**
		* @brief: 查询语句
		* @param: sql sql语句
		* @return: 查询所返回的结果集中第一行的第一列
		*/
		int ExecuteScalar(const std::string& sql);

		/**
		* @brief: 获取最后一次错误消息
		* @return: 最后一次错误消息
		*/
		const std::string& LastError();

	private:

		/**
		* @brief: 记录错误日志
		*/
		void LogError();

		//数据库
		sqlite3* _db;

		//查询结果
		sqlite3_stmt* _stmt;

		//最后一次出错信息
		std::string _lastError;

	};

	//sqlite修改
	class SqliteWriter
	{
	public:

		/**
		* @brief: 构造函数
		* @param: filePath 数据库文件路径
		*/
		SqliteWriter(const std::string& filePath);

		/**
		* @brief: 析构函数
		*/
		~SqliteWriter();

		/**
		* @brief: 执行语句不关心返回结果,即使出错也不会记录错误日志
		* @param: sql sql语句
		*/
		void Execute(const std::string& sql);

		/**
		* @brief: 执行语句并返回影响行数
		* @param: sql sql语句
		* @return: 执行成功返回影响行数否则返回-1
		*/
		int ExecuteRowCount(const std::string& sql);

		/**
		* @brief: 执行语句并返回主键
		* @param: sql sql语句
		* @return: 执行成功返回主键否则返回-1
		*/
		int ExecuteKey(const std::string& sql);

		/**
		* @brief: 获取最后一次错误消息
		* @return: 最后一次错误消息
		*/
		const std::string& LastError();

	private:

		/**
		* @brief: 记录错误日志
		*/
		void LogError();

		//数据库
		sqlite3* _db;

		//最后一次出错信息
		std::string _lastError;
	};
}


