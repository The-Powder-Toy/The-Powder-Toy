#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BOMB PT_BOMB 129
Element_BOMB::Element_BOMB()
{
	Identifier = "DEFAULT_PT_BOMB";
	Name = "BOMB";
	Colour = PIXPACK(0xFFF288);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;
	
	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;
	
	Weight = 30;
	
	Temperature = R_TEMP-2.0f	+273.15f;
	HeatConduct = 29;
	Description = "Bomb.";
	
	State = ST_NONE;
	Properties = TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC|PROP_SPARKSETTLE;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_BOMB::update;
	Graphics = &Element_BOMB::graphics;
}

//#TPT-Directive ElementHeader Element_BOMB static int update(UPDATE_FUNC_ARGS)
int Element_BOMB::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, nb;
	
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)!=PT_BOMB && (r&0xFF)!=PT_EMBR && (r&0xFF)!=PT_DMND && (r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN && (r&0xFF)!=PT_BCLN && (r&0xFF)!=PT_VIBR)
				{
					int rad = 8;
					int nxi;
					int nxj;
					pmap[y][x] = 0;
					for (nxj=-rad; nxj<=rad; nxj++)
						for (nxi=-rad; nxi<=rad; nxi++)
							if ((pow((float)nxi,2))/(pow((float)rad,2))+(pow((float)nxj,2))/(pow((float)rad,2))<=1)
								if ((pmap[y+nxj][x+nxi]&0xFF)!=PT_DMND && (pmap[y+nxj][x+nxi]&0xFF)!=PT_CLNE && (pmap[y+nxj][x+nxi]&0xFF)!=PT_PCLN && (pmap[y+nxj][x+nxi]&0xFF)!=PT_BCLN && (pmap[y+nxj][x+nxi]&0xFF)!=PT_VIBR)
								{
									sim->delete_part(x+nxi, y+nxj, 0);
									sim->pv[(y+nxj)/CELL][(x+nxi)/CELL] += 0.1f;
									nb = sim->create_part(-3, x+nxi, y+nxj, PT_EMBR);
									if (nb!=-1)
									{
										parts[nb].tmp = 2;
										parts[nb].life = 2;
										parts[nb].temp = MAX_TEMP;
									}
								}
					for (nxj=-(rad+1); nxj<=(rad+1); nxj++)
						for (nxi=-(rad+1); nxi<=(rad+1); nxi++)
							if ((pow((float)nxi,2))/(pow((float)(rad+1),2))+(pow((float)nxj,2))/(pow((float)(rad+1),2))<=1 && !(pmap[y+nxj][x+nxi]&0xFF))
							{
								nb = sim->create_part(-3, x+nxi, y+nxj, PT_EMBR);
								if (nb!=-1)
								{
									parts[nb].tmp = 0;
									parts[nb].life = 50;
									parts[nb].temp = MAX_TEMP;
									parts[nb].vx = rand()%40-20;
									parts[nb].vy = rand()%40-20;
								}
							}
					sim->kill_part(i);
					return 1;
				}
			}
	return 0;
}


//#TPT-Directive ElementHeader Element_BOMB static int graphics(GRAPHICS_FUNC_ARGS)
int Element_BOMB::graphics(GRAPHICS_FUNC_ARGS)

{
	*pixel_mode |= PMODE_FLARE;
	return 1;
}


Element_BOMB::~Element_BOMB() {}
