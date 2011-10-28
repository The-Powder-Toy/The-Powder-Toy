#include <element.h>

int graphics_SMKE(GRAPHICS_FUNC_ARGS)
{
	*colr = 55;
	*colg = 55;
	*colb = 55;
	
	*firea = 75;
	*firer = 55;
	*fireg = 55;
	*fireb = 55;
	
	*pixel_mode = PMODE_NONE; //Clear default, don't draw pixel
	*pixel_mode |= FIRE_BLEND;
	//Returning 1 means static, cache as we please
	return 1;
}