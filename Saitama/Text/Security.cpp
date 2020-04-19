#include "Security.h"

using namespace std;
using namespace Saitama;

void Security::Sha1(const string& value, unsigned char * hash, unsigned int capacity) {
	
	if (capacity < 20)
	{
		return;
	}
	// Init the result array.
	unsigned int result[5] = { 0x67452301, 0xefcdab89, 0x98badcfe,
							   0x10325476, 0xc3d2e1f0 };

	// Cast the void src pointer to be the byte array we can work with.
	unsigned char const * sarray = (unsigned char const *)value.c_str();

	// The reusable round buffer
	unsigned int w[80];

	// Loop through all complete 64byte blocks.

	size_t endCurrentBlock;
	size_t currentBlock = 0;

	if (value.size() >= 64) {
		size_t const endOfFullBlocks = value.size() - 64;

		while (currentBlock <= endOfFullBlocks) {
			endCurrentBlock = currentBlock + 64;

			// Init the round buffer with the 64 byte block data.
			for (int roundPos = 0; currentBlock < endCurrentBlock; currentBlock += 4)
			{
				// This line will swap endian on big endian and keep endian on
				// little endian.
				w[roundPos++] = (unsigned int)sarray[currentBlock + 3]
					| (((unsigned int)sarray[currentBlock + 2]) << 8)
					| (((unsigned int)sarray[currentBlock + 1]) << 16)
					| (((unsigned int)sarray[currentBlock]) << 24);
			}
			innerHash(result, w);
		}
	}

	// Handle the last and not full 64 byte block if existing.
	endCurrentBlock = value.size() - currentBlock;
	clearWBuffert(w);
	size_t lastBlockBytes = 0;
	for (; lastBlockBytes < endCurrentBlock; ++lastBlockBytes) {
		w[lastBlockBytes >> 2] |= (unsigned int)sarray[lastBlockBytes + currentBlock] << ((3 - (lastBlockBytes & 3)) << 3);
	}

	w[lastBlockBytes >> 2] |= 0x80 << ((3 - (lastBlockBytes & 3)) << 3);
	if (endCurrentBlock >= 56) {
		innerHash(result, w);
		clearWBuffert(w);
	}
	w[15] = static_cast<unsigned int>(value.size() << 3);
	innerHash(result, w);

	// Store hash in result pointer, and make sure we get in in the correct
	// order on both endian models.
	for (int hashByte = 20; --hashByte >= 0;) {
		hash[hashByte] = (result[hashByte >> 2] >> (((3 - hashByte) & 0x3) << 3)) & 0xff;
	}
}

inline unsigned int Security::rol(unsigned int value, unsigned int steps) {
	return ((value << steps) | (value >> (32 - steps)));
}

inline void Security::clearWBuffert(unsigned int * buffert)
{
	for (int pos = 16; --pos >= 0;)
	{
		buffert[pos] = 0;
	}
}

inline void Security::innerHash(unsigned int * result, unsigned int * w)
{
	unsigned int a = result[0];
	unsigned int b = result[1];
	unsigned int c = result[2];
	unsigned int d = result[3];
	unsigned int e = result[4];

	int round = 0;

#define sha1macro(func,val) \
    { \
        const unsigned int t = rol(a, 5) + (func) + e + val + w[round]; \
        e = d; \
        d = c; \
        c = rol(b, 30); \
        b = a; \
        a = t; \
    }

	while (round < 16)
	{
		sha1macro((b & c) | (~b & d), 0x5a827999)
			++round;
	}
	while (round < 20)
	{
		w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
		sha1macro((b & c) | (~b & d), 0x5a827999)
			++round;
	}
	while (round < 40)
	{
		w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
		sha1macro(b ^ c ^ d, 0x6ed9eba1)
			++round;
	}
	while (round < 60)
	{
		w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
		sha1macro((b & c) | (b & d) | (c & d), 0x8f1bbcdc)
			++round;
	}
	while (round < 80)
	{
		w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
		sha1macro(b ^ c ^ d, 0xca62c1d6)
			++round;
	}

#undef sha1macro

	result[0] += a;
	result[1] += b;
	result[2] += c;
	result[3] += d;
	result[4] += e;
}
