#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "StringEx.h"
#include "JsonFormatter.h"

namespace OnePunchMan
{
	//��ȡ�ļ������ļ�
	class FileConfig
	{
	public:
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
			return _jd.Get<T>(key);
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
			return _jd.Get<T>(key, defaultValue);
		}

	private:

		//������
		JsonDeserialization _jd;
	};

}

