#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "StringEx.h"

namespace Saitama
{
	//��ȡ�ļ������ļ�
	class FileConfig
	{
	public:

		/**
		* @brief: ���캯��
		*/
		FileConfig();

		/**
		* @brief: ���캯��
		* @param: filePath �ļ�·��
		*/
		FileConfig(const std::string& filePath);

		/**
		* @brief: ��ȡ�����ļ�
		* @param: key ��
		* @param: t ֵ
		* @return: ��ȡ�ɹ�����true
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
		* @brief: ��ȡ�����ļ�
		* @param: key ��
		* @param: v ֵ����
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

		//������
		std::map<std::string, std::string> _configs;
	};

}

