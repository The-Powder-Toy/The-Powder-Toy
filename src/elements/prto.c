#include <element.h>

int update_PRTO(UPDATE_FUNC_ARGS) {
	int r, nnx, rx, ry, np, fe = 0;
	int count = 0;
	parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
	if (parts[i].tmp>=CHANNELS) parts[i].tmp = CHANNELS-1;
	else if (parts[i].tmp<0) parts[i].tmp = 0;
	for (rx=1; rx>-2; rx--)
		for (ry=1; ry>-2; ry--)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				count ++;
				if (!r)
					fe = 1;
				if ((r>>8)>=NPART || r)
					continue;
				if (!r)
				{
					for ( nnx =0 ; nnx<80; nnx++)
					{
						int randomness = count + rand()%3-1;
						if (randomness<1)
							randomness=1;
						if (randomness>9)
							randomness=9;
						if (portal[parts[i].tmp][randomness-1][nnx]==PT_SPRK)// TODO: make it look better
						{
							create_part(-1,x+1,y,portal[parts[i].tmp][randomness-1][nnx]);
							create_part(-1,x+1,y+1,portal[parts[i].tmp][randomness-1][nnx]);
							create_part(-1,x+1,y-1,portal[parts[i].tmp][randomness-1][nnx]);
							create_part(-1,x,y-1,portal[parts[i].tmp][randomness-1][nnx]);
							create_part(-1,x,y+1,portal[parts[i].tmp][randomness-1][nnx]);
							create_part(-1,x-1,y+1,portal[parts[i].tmp][randomness-1][nnx]);
							create_part(-1,x-1,y,portal[parts[i].tmp][randomness-1][nnx]);
							create_part(-1,x-1,y-1,portal[parts[i].tmp][randomness-1][nnx]);
							portal[parts[i].tmp][randomness-1][nnx] = 0;
							portaltemp[parts[i].tmp][randomness-1][nnx] = 0;
							break;
						}
						else if (portal[parts[i].tmp][randomness-1][nnx])
						{
							np = create_part(-1,x+rx,y+ry,portal[parts[i].tmp][randomness-1][nnx]);
							if (np<0) continue;
							parts[np].temp = portaltemp[parts[i].tmp][randomness-1][nnx];
							parts[np].ctype = portalctype[parts[i].tmp][randomness-1][nnx];
							portal[parts[i].tmp][randomness-1][nnx] = 0;
							portaltemp[parts[i].tmp][randomness-1][nnx] = 0;
							portalctype[parts[i].tmp][randomness-1][nnx] = 0;
							break;
						}
					}
				}
			}
	if(fe){
		if(!parts[i].life) parts[i].life = rand();
		if(!parts[i].ctype) parts[i].life = rand();
		int orbd[4] = {0, 0, 0, 0};	//Orbital distances
		int orbl[4] = {0, 0, 0, 0};	//Orbital locations
		orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
		for(r = 0; r < 4; r++){
			if(orbd[r]<254){
				orbd[r] += 16;
				if(orbd[r]>254){
					orbd[r] = 0;
					orbl[r] = rand()%255;
				}
				//orbl[r] += 1;
				//orbl[r] = orbl[r]%255;
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
