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




int update_PROT(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					r = photons[y+ry][x+rx];
				if (!r)
					continue;
				if (((r&0xFF)!=PT_PROT)&&((r&0xFF)!=PT_DMND)&&((r&0xFF)!=PT_PRTI)&&((r&0xFF)!=PT_PRTO)&&((r&0xFF)!=PT_HSWC)&&((r&0xFF)!=PT_INSL)&&((r&0xFF)!=PT_DLAY)&&((r&0xFF)!=PT_WIFI))
					parts[r>>8].temp += ((parts[i].temp-parts[r>>8].temp) * 0.15);
				if ((r&0xFF)==PT_DMND)
					parts[i].temp += ((parts[r>>8].temp-parts[i].temp) * 0.5);
				if ((r&0xFF)==PT_EXOT)
					parts[r>>8].temp -= 15.0f;
				if (((r&0xFF)==PT_EXOT)&&10>(rand()%400))
					create_part(-2, x+rx, y+ry, PT_PROT);
				if (((r&0xFF)==PT_EXOT)&&15>(rand()%1000))
				{
					part_change_type(r>>8, x+rx, y+ry, PT_HFLM);
					parts[r>>8].life = 350;
					parts[r>>8].tmp = 5;
				}
				if ((r&0xFF)==PT_ELEC)
				{
					part_change_type(r>>8, x+rx, y+ry, PT_H2);
					kill_part(i);
					return 1;
				}
				if ((10>rand()%100)&&(pv[y/CELL][x/CELL] > -1.0f))
					pv[y/CELL][x/CELL] -= 0.05f;
				if (((r&0xFF)==PT_BMTL)&&(10>(rand()%1000)) && (pv[y/CELL][x/CELL] < -5.0f))
					part_change_type(r>>8, x+rx, y+ry, PT_BRMT);
				if (((r&0xFF)==PT_GLAS)&&(10>(rand()%1000)) && (pv[y/CELL][x/CELL] < -5.0f))
					part_change_type(r>>8, x+rx, y+ry, PT_BGLA);
				if (((r&0xFF)==PT_COAL)&&(10>(rand()%1000)) && (pv[y/CELL][x/CELL] < -5.0f))
					part_change_type(r>>8, x+rx, y+ry, PT_BCOL);
				if (((r&0xFF)==PT_BREL)&&(parts[r>>8].life==0))
				{
					part_change_type(r>>8, x+rx, y+ry, PT_SPRK);
					parts[r>>8].life = 4;
					parts[r>>8].ctype = PT_BREL;
					kill_part(i);
					return 1;
				}
				if (((r&0xFF)==PT_BANG)||((r&0xFF)==PT_PLEX)||((r&0xFF)==PT_GUNP)||((r&0xFF)==PT_NITR)||((r&0xFF)==PT_FIRW)||((r&0xFF)==PT_THRM)||((r&0xFF)==PT_IGNT))
				{
					part_change_type(r>>8, x+rx, y+ry, PT_FIRE);
					parts[r>>8].life = 10;
				}
				if ((r&0xFF)==PT_C5)
				{
					part_change_type(r>>8, x+rx, y+ry, PT_HFLM);
					parts[r>>8].life = 10;
				}
			}
	return 0;
}

int graphics_PROT(GRAPHICS_FUNC_ARGS)
{
	*firea = 40;
	*firer = 250;
	*fireg = 128;
	*fireb = 128;

	*pixel_mode &= ~PMODE_FLAT;
	*pixel_mode |= FIRE_ADD | PMODE_ADD;
	return 1;
}
