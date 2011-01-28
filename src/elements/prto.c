#include <element.h>

int update_PRTO(UPDATE_FUNC_ARGS) {
	int r, nnx, rx, ry, np;
	int count = 0;
	parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				count ++;
				if ((r>>8)>=NPART || r)
					continue;
				if (!r)
				{
					for ( nnx =0 ; nnx<80; nnx++)
					{
						int randomness = count + rand()%3-1;
						if (randomness<1)
							randomness=1;
						if (randomness>8)
							randomness=8;
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
	return 0;
}
