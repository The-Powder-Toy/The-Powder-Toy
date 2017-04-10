#ifndef __E189_UPDATE_H__
#define __E189_UPDATE_H__
class Simulation;
struct Particle;
class E189_Update
{
public:
	E189_Update();
	virtual ~E189_Update();
	static int update(UPDATE_FUNC_ARGS);
};
#endif