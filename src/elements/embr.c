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

/* ctype - colour RRGGBB (optional)
 * life - decremented each frame, disappears when life reaches zero
 * tmp - mode
 *   0 - BOMB sparks
 *   1 - firework sparks (colour defaults to white)
 *   2 - flash (colour defaults to white)
 */
int update_EMBR(UPDATE_FUNC_ARGS) {
	int r, rx, ry, nb;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((ptypes[r&0xFF].properties & (TYPE_SOLID | TYPE_PART | TYPE_LIQUID)) && !(ptypes[r&0xFF].properties & PROP_SPARKSETTLE))
				{
					kill_part(i);
					return 1;
				}
			}
	return 0;
}
int graphics_EMBR(GRAPHICS_FUNC_ARGS)
{
	if (cpart->ctype&0xFFFFFF)
	{
		int maxComponent;
		*colr = (cpart->ctype&0xFF0000)>>16;
		*colg = (cpart->ctype&0x00FF00)>>8;
		*colb = (cpart->ctype&0x0000FF);
		maxComponent = *colr;
		if (*colg>maxComponent) maxComponent = *colg;
		if (*colb>maxComponent) maxComponent = *colb;
		if (maxComponent<60)//make sure it isn't too dark to see
		{
			float multiplier = 60.0f/maxComponent;
			*colr *= multiplier;
			*colg *= multiplier;
			*colb *= multiplier;
		}
	}
	else if (cpart->tmp != 0)
	{
		*colr = *colg = *colb = 255;
	}

	if (decorations_enable && cpart->dcolour)
	{
		int a = (cpart->dcolour>>24)&0xFF;
		*colr = (a*((cpart->dcolour>>16)&0xFF) + (255-a)**colr) >> 8;
		*colg = (a*((cpart->dcolour>>8)&0xFF) + (255-a)**colg) >> 8;
		*colb = (a*((cpart->dcolour)&0xFF) + (255-a)**colb) >> 8;
	}
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	if (cpart->tmp==1)
	{
		*pixel_mode = FIRE_ADD | PMODE_BLEND | PMODE_GLOW;
		*firea = (cpart->life-15)*4;
		*cola = (cpart->life+15)*4;
	}
	else if (cpart->tmp==2)
	{
		*pixel_mode = PMODE_FLAT | FIRE_ADD;
		*firea = 255;
	}
	else
	{
		*pixel_mode = PMODE_SPARK | PMODE_ADD;
		if (cpart->life<64) *cola = 4*cpart->life;
	}
	return 0;
}
