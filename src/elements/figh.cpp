#include "simulation/Element.h"

int update_FIGH(UPDATE_FUNC_ARGS)
{
	playerst* figh = &sim->fighters[(unsigned char)parts[i].tmp];

	unsigned int tarx, tary;

	parts[i].tmp2 = 0; //0 - stay in place, 1 - seek a stick man

	//Set target cords
	if (sim->player.spwn && sim->player2.spwn)
	{
			if ((pow((float)sim->player.legs[2]-x, 2) + pow((float)sim->player.legs[3]-y, 2))<=
					(pow((float)sim->player2.legs[2]-x, 2) + pow((float)sim->player2.legs[3]-y, 2)))
			{
				tarx = (unsigned int)sim->player.legs[2];
				tary = (unsigned int)sim->player.legs[3];
			}
			else
			{
				tarx = (unsigned int)sim->player2.legs[2];
				tary = (unsigned int)sim->player2.legs[3];
			}
			parts[i].tmp2 = 1;
	}
	else
	{
		if (sim->player.spwn)
		{
			tarx = (unsigned int)sim->player.legs[2];
			tary = (unsigned int)sim->player.legs[3];
			parts[i].tmp2 = 1;
		}
		if (sim->player2.spwn)
		{
			tarx = (unsigned int)sim->player2.legs[2];
			tary = (unsigned int)sim->player2.legs[3];
			parts[i].tmp2 = 1;
		}
	}

	switch (parts[i].tmp2)
	{
		case 1:
			if ((pow(float(tarx-x), 2) + pow(float(tary-y), 2))<600)
			{
				if (figh->elem == PT_LIGH || figh->elem == PT_NEUT 
						|| sim->ptypes[figh->elem].properties&(PROP_DEADLY|PROP_RADIOACTIVE)
						|| sim->ptypes[figh->elem].heat>=323 || sim->ptypes[figh->elem].heat<=243)
					figh->comm = (int)figh->comm | 0x08;
			}
			else
				if (tarx<x)
				{
					if(!(sim->eval_move(PT_DUST, figh->legs[4]-10, figh->legs[5]+6, NULL)
					  && sim->eval_move(PT_DUST, figh->legs[4]-10, figh->legs[5]+3, NULL)))
						figh->comm = 0x01;
					else
						figh->comm = 0x02;

					if (!sim->eval_move(PT_DUST, figh->legs[4]-4, figh->legs[5]-1, NULL)
							|| !sim->eval_move(PT_DUST, figh->legs[12]-4, figh->legs[13]-1, NULL)
							|| sim->eval_move(PT_DUST, 2*figh->legs[4]-figh->legs[6], figh->legs[5]+5, NULL))
						figh->comm = (int)figh->comm | 0x04;
				}
				else
				{ 
					if (!(sim->eval_move(PT_DUST, figh->legs[12]+10, figh->legs[13]+6, NULL)
					   && sim->eval_move(PT_DUST, figh->legs[12]+10, figh->legs[13]+3, NULL)))
						figh->comm = 0x02;
					else
						figh->comm = 0x01;

					if (!sim->eval_move(PT_DUST, figh->legs[4]+4, figh->legs[5]-1, NULL)
							|| !sim->eval_move(PT_DUST, figh->legs[4]+4, figh->legs[5]-1, NULL)
							|| sim->eval_move(PT_DUST, 2*figh->legs[12]-figh->legs[14], figh->legs[13]+5, NULL))
						figh->comm = (int)figh->comm | 0x04;
				}
			break;
		default:
			figh->comm = 0;
			break;
	}

	figh->pcomm = figh->comm;

	run_stickman(figh, UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

int graphics_FIGH(GRAPHICS_FUNC_ARGS)
{	
	playerst * cplayer;// = &sim->fighters[(unsigned char)cpart->tmp];
	*pixel_mode = PSPEC_STICKMAN;
	/*if (cplayer->elem<PT_NUM)
	{
		*colr = PIXR(sim->ptypes[cplayer->elem].pcolors);
		*colg = PIXG(sim->ptypes[cplayer->elem].pcolors);
		*colb = PIXB(sim->ptypes[cplayer->elem].pcolors);
	}
	else*/
	{
		*colr = *colg = *colb = 255;
	}
	return 1;
}
