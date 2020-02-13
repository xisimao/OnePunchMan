#pragma once
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <numeric>
#include <map>
#include <iomanip>

namespace Saitama
{
	//�ı�������
	class Text
	{
	public:

		/**
		* @brief: ������ת��Ϊʮ�������ַ���
		* @param: value ����
		* @return: ʮ�������ַ���
		*/
		static std::string ToHex(int value);

		/**
		* @brief: ���ֽ���ת��Ϊʮ�������ַ���
		* @param: buffer �ֽ���
		* @param: size �ֽ�������
		* @return: ʮ�������ַ���
		*/
		static std::string ToHex(const char* buffer, unsigned int size);

		/**
		* @brief: ���ֽ���ת��Ϊʮ�������ַ���
		* @param: begin �ֽ�����ʼ
		* @param: end �ֽ�������
		* @return: ʮ�������ַ���
		*/
		static std::string ToHex(std::string::const_iterator begin, std::string::const_iterator end);

		/**
		* @brief: ��ʮ�������ַ���ת��Ϊ�ֽ���
		* @param: value �ַ���
		* @param: separator �ָ���
		* @return: ����ת������ֽ���
		*/
		static std::string ToBytes(const std::string& value, char separator);

		/**
		* @brief: ���У��
		* @param: buffer �ֽ���ָ��
		* @param: size �ֽ�������
		* @return: �������У����
		*/
		static char Xor(const char* buffer, unsigned int size);

		/**
		* @brief: ���ַ�����Ϊ��д
		* @param: value �ַ���
		*/
		static std::string ToUpper(const std::string& value);

		/**
		* @brief: �Ƴ��ַ���ǰ��Ŀհ��ַ�
		* @param: value �ַ���
		*/
		static std::string Trim(const std::string& value);

		/**
		* @brief: �ָ��ַ���
		* @param: value �ַ���
		* @param: separator �ָ��ַ���
		* @param: filterEmpty �Ƿ���˵��հ�
		*/
		static std::vector<std::string> Split(const std::string& value, const std::string& separator, bool filterEmpty = false);

		/**
		* @brief: ���ַ�����Ϊ32λ�з�������filterEmpty
		* @return: ��һ��������ʾת���Ƿ�ɹ������Ϊtrue���ڶ���������ʾת�����
		*/
		template<typename T>
		static bool Convert(const std::string& value, T* t)
		{
			std::stringstream ss;
			ss << value;
			ss >> *t;
			return !ss.fail();
		}

		/**
		* @brief: ������ת��Ϊ�ַ���
		* @param: value ����
		* @return: ��һ��������ʾת���Ƿ�ɹ������Ϊtrue���ڶ���������ʾת�����
		*/
		template<typename T>
		static std::string ToString(T value)
		{
			std::stringstream ss;
			ss << value;
			return std::string(ss.str());
		}

		/**
		* @brief: �������븡����
		* @param: value Դ������
		* @param: precision ��ȷλ��
		* @return: �����������ַ���
		*/
		static std::string Rounding(float value, int precision);
	};
}

