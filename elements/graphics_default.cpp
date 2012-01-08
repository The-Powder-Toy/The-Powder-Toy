#include "element.h"

int graphics_DEFAULT(GRAPHICS_FUNC_ARGS)
{	
	int t = cpart->type;
	//Property based defaults
	if(ren->sim->ptypes[t].properties & PROP_RADIOACTIVE) *pixel_mode |= PMODE_GLOW;
	if(ren->sim->ptypes[t].properties & TYPE_LIQUID)
	{
		*pixel_mode |= PMODE_BLUR;
	}
	if(ren->sim->ptypes[t].properties & TYPE_GAS)
	{
		*pixel_mode &= ~PMODE;
		*pixel_mode |= FIRE_BLEND;
		*firer = *colr/2;
		*fireg = *colg/2;
		*fireb = *colb/2;
		*firea = 125;
		*pixel_mode |= DECO_FIRE;
	}
	return 1;
}
