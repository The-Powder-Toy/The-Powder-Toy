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
 
int update_WIRE(UPDATE_FUNC_ARGS) {
    int s,r,rx,ry,count;    
    /*
    0:  wire
    1:  spark head
    2:  spark tail
    
    tmp is previous state, ctype is current state
    */
    //parts[i].tmp=parts[i].ctype;
    parts[i].ctype=0;
    if(parts[i].tmp==1)
	{
		parts[i].ctype=2;
	}
    if(parts[i].tmp==2)
	{
		parts[i].ctype=0;
	}
    
    count=0;
    for(rx=-1; rx<2; rx++)
        for(ry=-1; ry<2; ry++)
        {
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
				if (!r)
					continue;
                if((r&0xFF)==PT_SPRK && parts[r>>8].life==3 && parts[r>>8].ctype==PT_PSCN)
                {
						parts[i].ctype=1;
						return 0;
				}
				else if((r&0xFF)==PT_NSCN && parts[i].tmp==1){create_part(-1, x+rx, y+ry, PT_SPRK);}
                else if((r&0xFF)==PT_WIRE && parts[r>>8].tmp==1 && !parts[i].tmp){count++;}
            }
        }
    if(count==1 || count==2)
        parts[i].ctype=1;
	return 0;
}

int graphics_WIRE(GRAPHICS_FUNC_ARGS)
{
	if (cpart->ctype==0)
	{
		*colr = 255;
		*colg = 204;
		*colb = 0;
		return 0;
	}
	if (cpart->ctype==1)
	{
		*colr = 50;
		*colg = 100;
		*colb = 255;
		//*pixel_mode |= PMODE_GLOW;
		return 0;
	}
	if (cpart->ctype==2)
	{
		*colr = 255;
		*colg = 100;
		*colb = 50;
		//*pixel_mode |= PMODE_GLOW;
		return 0;
	}
}
