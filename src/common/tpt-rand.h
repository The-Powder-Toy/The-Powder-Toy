#pragma once
#include "ExplicitSingleton.h"
#include <stdint.h>

class RNGType
{
private:
	uint64_t s[2];
	uint64_t next();
public:
	unsigned int operator()();
	unsigned int gen();
	int between(int lower, int upper);
	bool chance(int nominator, unsigned int denominator);
	float uniform01();

	RNGType();
	void seed(unsigned int sd);
};

// Needed because we also have random_gen, and that would take the singleton role if RNGType had an ExplicitSingleton base.
class RNG : public RNGType, public ExplicitSingleton<RNG>
{
};

extern RNGType random_gen;
