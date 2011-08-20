#include <element.h>
/*these are the count values of where the particle gets stored, depending on where it came from
   0 1 2
   7 . 3
   6 5 4
   PRTO does (count+4)%8, so that it will come out at the opposite place to where it came in
   PRTO does +/-1 to the count, so it doesn't jam as easily
*/
int portal_rx[8] = {-1, 0, 1, 1, 1, 0,-1,-1};
int portal_ry[8] = {-1,-1,-1, 0, 1, 1, 1, 0};

int update_PRTI(UPDATE_FUNC_ARGS) {
	int r, nnx, rx, ry, fe = 0;
	int count =0;
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
				if ((r>>8)>=NPART)
					continue;
				if (!r || (r&0xFF)==PT_PRTI || (r&0xFF)==PT_PRTO || (ptypes[r&0xFF].falldown== 0 && ptypes[r&0xFF].state != ST_GAS && (r&0xFF)!=PT_SPRK))
				{
					r = photons[y+ry][x+rx];
					if ((r>>8)>=NPART)
						continue;
					if (!r || (r&0xFF)==PT_PRTI || (r&0xFF)==PT_PRTO || (ptypes[r&0xFF].falldown== 0 && ptypes[r&0xFF].state != ST_GAS && (r&0xFF)!=PT_SPRK))
						continue;
				}

				if ((r&0xFF) == PT_SOAP)
					detach(r>>8);

				for ( nnx=0; nnx<80; nnx++)
					if (!portalp[parts[i].tmp][count][nnx].type)
					{
						portalp[parts[i].tmp][count][nnx] = parts[r>>8];
						if ((r&0xFF)==PT_SPRK)
							part_change_type(r>>8,x+rx,y+ry,parts[r>>8].ctype);
						else
							kill_part(r>>8);
						fe = 1;
						break;
					}
			}
	}


	if (fe) {
		int orbd[4] = {0, 0, 0, 0};	//Orbital distances
		int orbl[4] = {0, 0, 0, 0};	//Orbital locations
		if (!parts[i].life) parts[i].life = rand();
		if (!parts[i].ctype) parts[i].ctype = rand();
		orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
		for (r = 0; r < 4; r++) {
			if (orbd[r]>1) {
				orbd[r] -= 12;
				if (orbd[r]<1) {
					orbd[r] = (rand()%128)+128;
					orbl[r] = rand()%255;
				} else {
					orbl[r] += 2;
					orbl[r] = orbl[r]%255;
				}
			} else {
				orbd[r] = (rand()%128)+128;
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
