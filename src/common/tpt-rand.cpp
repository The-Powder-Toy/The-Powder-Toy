#include "tpt-rand.h"
#include <cstdlib>

/* xoroshiro128+ by David Blackman and Sebastiano Vigna */

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

uint64_t RandomGen::next(void) {
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t result = s0 + s1;

	s1 ^= s0;
	s[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
	s[1] = rotl(s1, 36); // c

	return result;
}

unsigned int RandomGen::operator()()
{
	return next()&0xFFFFFFFF;
}

unsigned int RandomGen::between(unsigned int lower, unsigned int upper)
{
	unsigned int r = (*this)();
	
	return r % (upper - lower + 1) + lower;
}

float RandomGen::uniform01()
{
	return static_cast<float>(random_gen())/(float)0xFFFFFFFF;
}

RandomGen::RandomGen()
{
	s[0] = 1;
	s[1] = 2;
}

void RandomGen::seed(unsigned int sd)
{
	s[0] = sd;
	s[1] = sd;
}

RandomGen random_gen;
