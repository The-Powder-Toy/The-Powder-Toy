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
/*these are the count values of where the particle gets stored, depending on where it came from
   0 1 2
   7 . 3
   6 5 4
   PRTO does (count+4)%8, so that it will come out at the opposite place to where it came in
   PRTO does +/-1 to the count, so it doesn't jam as easily
*/
int update_PRTO(UPDATE_FUNC_ARGS) {
	int r, nnx, rx, ry, np, fe = 0;
	int count = 0;
	parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
	if (parts[i].tmp>=CHANNELS) parts[i].tmp = CHANNELS-1;
	else if (parts[i].tmp<0) parts[i].tmp = 0;
	for (count=0; count<8; count++)
	{
		rx = portal_rx[count];
		ry = portal_ry[count];
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					fe = 1;
				if (r)
					continue;
				if (!r)
				{
					for ( nnx =0 ; nnx<80; nnx++)
					{
						int randomness = (count + rand()%3-1 + 4)%8;//add -1,0,or 1 to count
						if (portalp[parts[i].tmp][randomness][nnx].type==PT_SPRK)// TODO: make it look better, spark creation
						{
							create_part(-1,x+1,y,PT_SPRK);
							create_part(-1,x+1,y+1,PT_SPRK);
							create_part(-1,x+1,y-1,PT_SPRK);
							create_part(-1,x,y-1,PT_SPRK);
							create_part(-1,x,y+1,PT_SPRK);
							create_part(-1,x-1,y+1,PT_SPRK);
							create_part(-1,x-1,y,PT_SPRK);
							create_part(-1,x-1,y-1,PT_SPRK);
							portalp[parts[i].tmp][randomness][nnx] = emptyparticle;
							break;
						}
						else if (portalp[parts[i].tmp][randomness][nnx].type)
						{
							if (portalp[parts[i].tmp][randomness][nnx].type==PT_STKM)
								player.spwn = 0;
							if (portalp[parts[i].tmp][randomness][nnx].type==PT_STKM2)
								player2.spwn = 0;
							if (portalp[parts[i].tmp][randomness][nnx].type==PT_FIGH)
							{
								fighcount--;
								fighters[(unsigned char)portalp[parts[i].tmp][randomness][nnx].tmp].spwn = 0;
							}
							np = create_part(-1,x+rx,y+ry,portalp[parts[i].tmp][randomness][nnx].type);
							if (np<0)
							{
								if (portalp[parts[i].tmp][randomness][nnx].type==PT_STKM)
									player.spwn = 1;
								if (portalp[parts[i].tmp][randomness][nnx].type==PT_STKM2)
									player2.spwn = 1;
								if (portalp[parts[i].tmp][randomness][nnx].type==PT_FIGH)
								{
									fighcount++;
									fighters[(unsigned char)portalp[parts[i].tmp][randomness][nnx].tmp].spwn = 1;
								}
								continue;
							}
							if (parts[np].type==PT_FIGH)
							{
								// Release the fighters[] element allocated by create_part, the one reserved when the fighter went into the portal will be used
								fighters[(unsigned char)parts[np].tmp].spwn = 0;
								fighters[(unsigned char)portalp[parts[i].tmp][randomness][nnx].tmp].spwn = 1;
							}
							parts[np] = portalp[parts[i].tmp][randomness][nnx];
							parts[np].x = x+rx;
							parts[np].y = y+ry;
							portalp[parts[i].tmp][randomness][nnx] = emptyparticle;
							break;
						}
					}
				}
			}
	}
	if (fe) {
		int orbd[4] = {0, 0, 0, 0};	//Orbital distances
		int orbl[4] = {0, 0, 0, 0};	//Orbital locations
		if (!parts[i].life) parts[i].life = rand()*rand()*rand();
		if (!parts[i].ctype) parts[i].ctype = rand()*rand()*rand();
		orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
		for (r = 0; r < 4; r++) {
			if (orbd[r]<254) {
				orbd[r] += 16;
				if (orbd[r]>254) {
					orbd[r] = 0;
					orbl[r] = rand()%255;
				} else {
					orbl[r] += 1;
					orbl[r] = orbl[r]%255;
				}
			} else {
				orbd[r] = 0;
				orbl[r] = rand()%255;
			}
		}
		orbitalparts_set(&parts[i].life, &parts[i].ctype, orbd, orbl);
	} else {
		parts[i].life = 0;
		parts[i].ctype = 0;
	}
	return 0;
}
