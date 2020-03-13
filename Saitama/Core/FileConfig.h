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
		* @return:��ȡ�ɹ����ض�ȡ��������򷵻�0���߿��ַ���
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
		* @brief: ��ȡ�����ļ�
		* @param: key ��
		* @param: t ��ȡʧ��ʱ���ص�Ĭ��ֵ
		* @return: ��ȡ�ɹ����ض�ȡ��������򷵻�Ĭ��ֵ
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
		* @brief: ��ȡ�����ļ�
		* @param: key ��
		* @return: ��ȡ�ɹ����ض�ȡ��������򷵻ؿռ���
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

		//������
		std::map<std::string, std::string> _configs;
	};

}

