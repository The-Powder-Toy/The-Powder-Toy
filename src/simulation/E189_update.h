#ifndef __E189_UPDATE_H__
#define __E189_UPDATE_H__

class Simulation;
class Renderer;
struct Particle;
class E189_Update
{
public:
	E189_Update();
	virtual ~E189_Update();
	static int update(UPDATE_FUNC_ARGS);
	static int graphics(GRAPHICS_FUNC_ARGS);
	static void InsertText(Simulation *sim, int i, int x, int y, int ix, int iy);
	static int AddCharacter(Simulation *sim, int x, int y, int c, int rgb);
	static void conductTo (Simulation* sim, int r, int x, int y, Particle *parts) // Inline or macro?
	{
		if (!parts[r>>8].life)
		{
			parts[r>>8].ctype = r&0xFF;
			sim->part_change_type(r>>8, x, y, PT_SPRK);
			parts[r>>8].life = 4;
		}
	}
};
#endif
