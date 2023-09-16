#pragma once
#include "ExplicitSingleton.h"
#include <stdint.h>
#include <array>

class RNG
{
public:
	using State = std::array<uint64_t, 2>;

private:
	State s;
	uint64_t next();
public:
	unsigned int operator()();
	unsigned int gen();
	int between(int lower, int upper);
	bool chance(int numerator, unsigned int denominator);
	float uniform01();

	RNG();
	void seed(unsigned int sd);

	void state(State ns)
	{
		s = ns;
	}

	State state() const
	{
		return s;
	}
};

// Please only use this on the main thread and never for simulation stuff.
// For simulation stuff, use Simulation::rng. For renderer stuff, use Renderer::rng.
// For anything else, prefer a dedicated RNG instance over this one.
extern RNG interfaceRng;
