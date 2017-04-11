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
	static void conductTo (Simulation* sim, int r, int x, int y, Particle *parts) // Inline or macro?
	{
		parts[r>>8].ctype = r&0xFF;
		sim->part_change_type(r>>8, x, y, PT_SPRK);
		parts[r>>8].life = 4;
	}
};
#endif