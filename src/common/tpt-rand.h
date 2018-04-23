#ifndef TPT_RAND_
#define TPT_RAND_

#include <stdint.h>

class RandomGen
{
private:
	uint64_t s[2];
	uint64_t next(void);
public:
	unsigned int operator()();
	unsigned int between(unsigned int lower, unsigned int upper);
	float uniform01();

	RandomGen();
	void seed(unsigned int sd);
};

extern RandomGen random_gen;

#endif /* TPT_RAND_ */
