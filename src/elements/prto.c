#include <powder.h>

int update_PRTO(UPDATE_FUNC_ARGS) {
	int r, temp, nnx, temprange = 100;
	int count = 0;
	for ( temp = 0; temp < MAX_TEMP; temp += temprange)
		if (parts[i].temp-73.15>temp&&parts[i].temp-73.15 <temp+temprange)
			parts[i].tmp = temp/100;
	for (ny=1; ny>-2; ny--)
		for (nx=1; nx>-2; nx--)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				count ++;
				r = pmap[y+ny][x+nx];
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
						if (portal[parts[i].tmp][randomness-1][nnx]==PT_SPRK)//todo. make it look better
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
							create_part(-1,x+nx,y+ny,portal[parts[i].tmp][randomness-1][nnx]);
							parts[pmap[y+ny][x+nx]>>8].temp = portaltemp[parts[i].tmp][randomness-1][nnx];
							portal[parts[i].tmp][randomness-1][nnx] = 0;
							portaltemp[parts[i].tmp][randomness-1][nnx] = 0;
							break;
						}
					}
				}
			}
	return 0;
}
