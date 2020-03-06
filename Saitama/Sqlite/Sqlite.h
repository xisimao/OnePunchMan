#pragma once
#include <string>
#include <iostream>

#include "sqlite3.h"
#include "LogPool.h"

namespace Saitama
{
	//sqlite查询
	class SqliteReader
	{
	public:

		/**
		* @brief: 构造函数
		*/
		SqliteReader();

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
		* @brief: 读取数字列
		* @param: index 列序号
		* @return: 列的数字值
		*/
		int GetInteger(int index) const;

		/**
		* @brief: 结束查询
		* @param: stmt 查询结果
		*/
		void EndQuery();

	private:

		//数据库
		sqlite3* _db;

		//查询结果
		sqlite3_stmt* _stmt;

	};

	//sqlite修改
	class SqliteWriter
	{
	public:

		/**
		* @brief: 构造函数
		*/
		SqliteWriter();

		/**
		* @brief: 析构函数
		*/
		~SqliteWriter();

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

	private:

		//数据库
		sqlite3* _db;
	};
}


