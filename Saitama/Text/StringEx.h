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
		* @brief: ���ַ���ת��Ϊ���ֻ��߲�������
		* @param: value �ַ���
		* @return: ת���ɹ�����ת��������򷵻�0
		*/
		template<typename T>
		static T Convert(const std::string& value)
		{
			return Convert<T>(value, 0);
		}

#ifdef _WIN32
		/**
		* @brief: ���ַ���ת��Ϊ�ַ�������
		* @param: value �ַ���
		* @return: ת���ɹ�����ת��������򷵻ؿ��ַ���
		*/
		template<>
		static std::string Convert<std::string>(const std::string& value)
		{
			return Convert(value, std::string());
		}
#endif
		/**
		* @brief: ���ַ���ת��Ϊ���ֻ��߲�������
		* @param: value �ַ���
		* @param: defaultValue ת��ʧ�ܷ��ص�Ĭ��ֵ
		* @return: ת���ɹ�����ת��������򷵻�Ĭ��ֵ
		*/
		template<typename T>
		static T Convert(const std::string& value,T defaultValue)
		{
			T t;
			if (TryConvert(value, &t))
			{
				return t;
			}
			else
			{
				return defaultValue;
			}
		}

		/**
		* @brief: ���ַ���ת��Ϊ��������
		* @param: value �ַ���
		* @param: t ���ֻ򲼶�����ָ��
		* @return: ת���ɹ�����true
		*/
		template<typename T>
		static bool TryConvert(const std::string& value, T* t)
		{
			std::stringstream ss;
			ss << value;
			ss >> *t;
			return !ss.fail();
		}

		/**
		* @brief: ���ַ���ת��Ϊ��������
		* @param: value �ַ���
		* @param: t ���ֻ򲼶�����ָ��
		* @return: ת���ɹ�����true
		*/
		static bool TryConvert(const std::string& value, bool* t)
		{
			if (ToUpper(value).compare("TRUE") == 0)
			{
				*t = true;
				return true;
			}
			else if (ToUpper(value).compare("FALSE") == 0)
			{
				*t = false;
				return true;
			}
			else
			{
				return false;
			}
		}

		/**
		* @brief: ���ַ���ת��Ϊ�ַ�������
		* @param: value �ַ���
		* @param: t ���ֻ򲼶�����ָ��
		* @return: ת���ɹ�����true
		*/
		static bool TryConvert(const std::string& value, std::string* t)
		{
			*t = value;
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

