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
		* @brief: ���ַ���ת��Ϊ��������
		* @param: value �ַ���
		* @return: ת���ɹ�����ת��������򷵻ؿռ���
		*/
		template<typename T>
		static std::vector<T> ConvertToArray(const std::string& value)
		{
			std::vector<T> v;
			size_t startIndex = 0;
			while (startIndex<value.size())
			{
				T t;
				size_t result = FindItem(value, &t, startIndex);
				if (result == 0)
				{
					break;
				}
				else
				{
					startIndex += result;
					v.push_back(t);
				}
			}
			return v;
		}

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

#ifdef _WIN32
		template<>
		static std::string Convert<std::string>(const std::string& value)
		{
			return Convert<std::string>(value, std::string());
		}

		template<>
		static bool TryConvert<bool>(const std::string& value, bool* t)
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

		template<>
		static bool TryConvert<std::string>(const std::string& value, std::string* t)
		{
			*t = value;
			return true;
		}

#endif

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

	private:

		/**
		* @brief: ���ַ����н�ȡ����򲼶������е�һ�
		* @param: json json�ַ���
		* @param: t �ֶ�ֵ��ָ��
		* @param: offset �ӿ�ͷ�Թ����ַ���
		* @return: �����ɹ����ؽ��������ַ��������򷵻�0
		*/
		template<typename T>
		static size_t FindItem(const std::string& json, T* t, size_t offset)
		{
			size_t endIndex = json.find(',', offset);
			size_t size;
			size_t hasTail;
			if (endIndex == std::string::npos)
			{
				size = json.size() - offset;
				hasTail = false;
			}
			else
			{
				size = endIndex - offset;
				hasTail = true;
			}
			if (StringEx::TryConvert(json.substr(offset, size), t))
			{
				return hasTail ? size + 1 : size;
			}
			else
			{
				return 0;
			}
		}

		/**
		* @brief: ���ַ����н�ȡ�ַ��������е�һ���
		* @param: json json�ַ���
		* @param: t �ֶ�ֵ��ָ��
		* @param: offset �ӿ�ͷ�Թ����ַ���
		* @return: �����ɹ����ؽ��������ַ��������򷵻�0
		*/
		static size_t FindItem(const std::string& json, std::string* t, size_t offset)
		{
			size_t startIndex = json.find('"', offset);
			if (startIndex == std::string::npos)
			{
				return 0;
			}
			else
			{
				size_t endIndex = startIndex;
				while (true)
				{
					endIndex = json.find('"', endIndex + 1);
					if (endIndex == std::string::npos)
					{
						return 0;
					}
					else
					{
						if (json[endIndex - 1] == '\\')
						{
							startIndex = endIndex - 1;
						}
						else
						{
							break;
						}
					}
				}
				t->assign(json.substr(startIndex + 1, endIndex - startIndex - 1));
				endIndex = json.find(',', endIndex);
				if (endIndex == std::string::npos)
				{
					return json.size() - startIndex;
				}
				else
				{
					return endIndex - startIndex + 1;
				}
			}
		}
	};



}

