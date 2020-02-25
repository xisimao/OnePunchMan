#pragma once
#include <string>

namespace Saitama
{
	class Security
	{
	public:


		/**
		* @brief: sha1算法加密
		* @param: value 需要加密的字符串
		* @param: hash 加密后的字节流
		*/
		static void Sha1(const std::string& value, unsigned char * hash,unsigned int capacity);

		/**
		* @brief: base64算法加密
		* @param: buffer 需要加密的字节流
		* @param: size 需要加密的字节流长度
		* @return: 加密后的字符串
		*/
		static std::string Base64_Encode(const unsigned char* buffer, unsigned int size);

		/**
		* @brief: base64算法加密
		* @param: value 需要加密的字符串
		* @return: 加密后的字符串
		*/
		static std::string Base64_Encode(const std::string& value);

		/**
		* @brief: base64算法解密
		* @param: value 需要解密的字符串
		* @return: 解密后的字符串
		*/
		static std::string Base64_Decode(const std::string& value);

	private:

		//base64
		static const std::string Base64_Chars;
		static inline bool is_base64(unsigned char c);

		//sha1
		static inline unsigned int rol(unsigned int value, unsigned int steps);
		static inline void clearWBuffert(unsigned int * buffert);
		static inline void innerHash(unsigned int * result, unsigned int * w);
	};

}


