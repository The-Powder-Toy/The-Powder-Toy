#include <element.h>

int update_SPAWN2(UPDATE_FUNC_ARGS) {
	if (!player2.spwn)
		create_part(-1, x, y, PT_STKM2);

	return 0;
}

int update_STKM2(UPDATE_FUNC_ARGS) {
	run_stickman(&player2, UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

int graphics_STKM2(GRAPHICS_FUNC_ARGS)
{	
	*pixel_mode = PSPEC_STICKMAN;
	if ((int)player2.elem<PT_NUM)
	{
		*colr = PIXR(ptypes[player2.elem].pcolors);
		*colg = PIXG(ptypes[player2.elem].pcolors);
		*colb = PIXB(ptypes[player2.elem].pcolors);
	}
	else
	{
		*colr = *colg = *colb = 255;
	}
	return 1;
}
