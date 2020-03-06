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
	class StringEx
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
		* @brief: ���ַ�����Ϊ��������
		* @param: value �ַ���
		* @param: t ���ֻ򲼶�����ָ��
		* @return: ת���ɹ�����true
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
		* @brief: ���ַ�����Ϊ��������
		* @param: value �ַ���
		* @param: t ���ֻ򲼶�����ָ��
		* @return: ת���ɹ�����true
		*/
		template<>
		static bool Convert(const std::string& value, bool* t)
		{
			int i = 0;
			std::stringstream ss;
			ss << value;
			ss >> i;
			if (ss.fail())
			{
				if (ToUpper(value).compare("TRUE")==0)
				{
					*t = true;
				}
				else if (ToUpper(value).compare("FALSE")==0)
				{
					*t = false;
				}
				else
				{
					return false;
				}
			}
			else
			{
				*t = i == 0 ? false : true;
			}
			return true;
		}

		/**
		* @brief: ���ַ�����Ϊ�ַ���,�÷�����Ϊ�˷���������
		* @param: value �ַ���
		* @param: t �ַ���ָ��
		* @return: �̶�����true
		*/
		template<>
		static bool Convert(const std::string& value, std::string* t)
		{
			t->assign(value);
			return true;
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

		/**
		* @brief: �����Ԫ����ϳ�һ���ַ�����
		* @param: t ���ַ����ĸ����ֹ��ɵ�Ԫ��
		* @param: u... ���ַ����ĸ����ֹ��ɵ�Ԫ��
		* @return: ����ϵ��ַ�����
		*/
		template<typename T, typename ...U>
		static std::string Combine(const T& t, const U& ...u)
		{
			std::stringstream ss;
			Combine(&ss, t, u...);
			return ss.str();
		}

		/**
		* @brief: �����Ԫ����ϳ�һ���ַ�����
		* @param: t ���ַ����ĸ����ֹ��ɵ�Ԫ�ء�
		* @param: u... ���ַ����ĸ����ֹ��ɵ�Ԫ�ء�
		*/
		template<typename T, typename ...U>
		static void Combine(std::stringstream* ss, const T& t, const U& ...u)
		{
			(*ss) << t;
			Combine(ss, u...);
		}

		/**
		* @brief: �����һ��Ԫ��ƴ�ӵ��ַ����ϡ�
		* @param: t ����ַ��������һ����Ԫ�ء�
		*/
		template<typename T>
		static void Combine(std::stringstream* ss, const T& t)
		{
			(*ss) << t;
		}
	};
}

