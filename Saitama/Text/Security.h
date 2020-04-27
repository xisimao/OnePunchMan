#pragma once
#include <string>

namespace OnePunchMan
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

	private:
		//sha1
		static inline unsigned int rol(unsigned int value, unsigned int steps);
		static inline void clearWBuffert(unsigned int * buffert);
		static inline void innerHash(unsigned int * result, unsigned int * w);
	};

}


