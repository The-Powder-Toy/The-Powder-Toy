/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <element.h>

int update_FIGH(UPDATE_FUNC_ARGS)
{
	playerst* figh = &fighters[(unsigned char)parts[i].tmp];

	unsigned int tarx, tary;

	parts[i].tmp2 = 0; //0 - stay in place, 1 - seek a stick man

	//Set target cords
	if (player.spwn && player2.spwn)
	{
			if ((pow(player.legs[2]-x, 2) + pow(player.legs[3]-y, 2))<=
					(pow(player2.legs[2]-x, 2) + pow(player2.legs[3]-y, 2)))
			{
				tarx = (unsigned int)player.legs[2];
				tary = (unsigned int)player.legs[3];
			}
			else
			{
				tarx = (unsigned int)player2.legs[2];
				tary = (unsigned int)player2.legs[3];
			}
			parts[i].tmp2 = 1;
	}
	else
	{
		if (player.spwn)
		{
			tarx = (unsigned int)player.legs[2];
			tary = (unsigned int)player.legs[3];
			parts[i].tmp2 = 1;
		}
		if (player2.spwn)
		{
			tarx = (unsigned int)player2.legs[2];
			tary = (unsigned int)player2.legs[3];
			parts[i].tmp2 = 1;
		}
	}

	switch (parts[i].tmp2)
	{
		case 1:
			if ((pow(tarx-x, 2) + pow(tary-y, 2))<600)
			{
				if (figh->elem == PT_LIGH || figh->elem == PT_NEUT 
						|| ptypes[figh->elem].properties&(PROP_DEADLY|PROP_RADIOACTIVE) 
						|| ptypes[figh->elem].heat>=323 || ptypes[figh->elem].heat<=243)
					figh->comm = (int)figh->comm | 0x08;
			}
			else
				if (tarx<x)
				{
					if(!(eval_move(PT_DUST, figh->legs[4]-10, figh->legs[5]+6, NULL) 
					  && eval_move(PT_DUST, figh->legs[4]-10, figh->legs[5]+3, NULL)))
						figh->comm = 0x01;
					else
						figh->comm = 0x02;

					if (!eval_move(PT_DUST, figh->legs[4]-4, figh->legs[5]-1, NULL) 
							|| !eval_move(PT_DUST, figh->legs[12]-4, figh->legs[13]-1, NULL)
							|| eval_move(PT_DUST, 2*figh->legs[4]-figh->legs[6], figh->legs[5]+5, NULL))
						figh->comm = (int)figh->comm | 0x04;
				}
				else
				{ 
					if (!(eval_move(PT_DUST, figh->legs[12]+10, figh->legs[13]+6, NULL)
					   && eval_move(PT_DUST, figh->legs[12]+10, figh->legs[13]+3, NULL)))
						figh->comm = 0x02;
					else
						figh->comm = 0x01;

					if (!eval_move(PT_DUST, figh->legs[4]+4, figh->legs[5]-1, NULL) 
							|| !eval_move(PT_DUST, figh->legs[4]+4, figh->legs[5]-1, NULL)
							|| eval_move(PT_DUST, 2*figh->legs[12]-figh->legs[14], figh->legs[13]+5, NULL))
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
	playerst * cplayer = &fighters[(unsigned char)cpart->tmp];
	*pixel_mode = PSPEC_STICKMAN;
	if (cplayer->elem<PT_NUM)
	{
		*colr = PIXR(ptypes[cplayer->elem].pcolors);
		*colg = PIXG(ptypes[cplayer->elem].pcolors);
		*colb = PIXB(ptypes[cplayer->elem].pcolors);
	}
	else
	{
		*colr = *colg = *colb = 255;
	}
	return 1;
}
