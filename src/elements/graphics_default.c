#include <element.h>

int graphics_DEFAULT(GRAPHICS_FUNC_ARGS)
{	
	int t = cpart->type;
	//Property based defaults
	if(ptypes[t].properties & PROP_RADIOACTIVE) *pixel_mode |= PMODE_GLOW;
	if(ptypes[t].properties & TYPE_LIQUID)
	{
		*pixel_mode |= PMODE_BLUR;
	}
	if(ptypes[t].properties & TYPE_GAS)
	{
		*pixel_mode &= ~PMODE;
		*pixel_mode |= FIRE_BLEND;
		*firer = *colr/2;
		*fireg = *colg/2;
		*fireb = *colb/2;
		*firea = 125;
	}
	return 1;
}
