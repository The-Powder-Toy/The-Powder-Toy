#include <powder.h>

int update_SHLD1(UPDATE_FUNC_ARGS) {
	int r, nnx, nny;
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				else if (parts[r>>8].type==PT_SPRK&&parts[i].life==0)
					for ( nnx=-1; nnx<2; nnx++)
						for ( nny=-1; nny<2; nny++)
						{
							if (7>rand()%200&&parts[i].life==0)
							{
								parts[i].type = PT_SHLD2;
								parts[i].life = 7;
							}
							else if (!pmap[y+ny+nny][x+nx+nnx])
							{
								create_part(-1,x+nx+nnx,y+ny+nny,PT_SHLD1);
								//parts[pmap[y+ny+nny][x+nx+nnx]>>8].life=7;
							}
						}
				else if (parts[r>>8].type==PT_SHLD3&&4>rand()%10)
				{
					parts[i].type=PT_SHLD2;
					parts[i].life = 7;
				}
			}
	return 0;
}

int update_SHLD2(UPDATE_FUNC_ARGS) {
	int r, nnx, nny;
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART)
					continue;
				if (!r && parts[i].life>0)
					create_part(-1,x+nx,y+ny,PT_SHLD1);
				if (!r)
					continue;
				else if (parts[r>>8].type==PT_SPRK&&parts[i].life==0)
					for ( nnx=-1; nnx<2; nnx++)
						for ( nny=-1; nny<2; nny++)
						{
							if (3>rand()%200&&parts[i].life==0)
							{
								parts[i].type = PT_SHLD3;
								parts[i].life = 7;
							}
							else if (!pmap[y+ny+nny][x+nx+nnx])
							{
								create_part(-1,x+nx+nnx,y+ny+nny,PT_SHLD1);
								parts[pmap[y+ny+nny][x+nx+nnx]>>8].life=7;
							}
						}
				else if (parts[r>>8].type==PT_SHLD4&&4>rand()%10)
				{
					parts[i].type=PT_SHLD3;
					parts[i].life = 7;
				}
			}
	return 0;
}

int update_SHLD3(UPDATE_FUNC_ARGS) {
	int r, nnx, nny;
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART)
					continue;
				if (!r)
				{
					if (1>rand()%2500)
					{
						create_part(-1,x+nx,y+ny,PT_SHLD1);
						parts[pmap[y+ny][x+nx]>>8].life=7;
						parts[i].type = PT_SHLD2;
					}
					else
						continue;

				}
				if (parts[r>>8].type==PT_SHLD1 && parts[i].life>3)
				{
					parts[r>>8].type = PT_SHLD2;
					parts[r>>8].life=7;
				}
				else if (parts[r>>8].type==PT_SPRK&&parts[i].life==0)
					for ( nnx=-1; nnx<2; nnx++)
						for ( nny=-1; nny<2; nny++)
						{
							if (2>rand()%3000&&parts[i].life==0)
							{
								parts[i].type = PT_SHLD4;
								parts[i].life = 7;
							}
							else if (!pmap[y+ny+nny][x+nx+nnx])
							{
								create_part(-1,x+nx+nnx,y+ny+nny,PT_SHLD1);
								parts[pmap[y+ny+nny][x+nx+nnx]>>8].life=7;
							}
						}
			}
	return 0;
}

int update_SHLD4(UPDATE_FUNC_ARGS) {
	int r, nnx, nny;
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART)
					continue;
				if (!r)
				{
					if (1>rand()%5500)
					{
						create_part(-1,x+nx,y+ny,PT_SHLD1);
						parts[pmap[y+ny][x+nx]>>8].life=7;
						parts[i].type = PT_SHLD2;
					}
					else
						continue;

				}
				if (parts[r>>8].type==PT_SHLD2 && parts[i].life>3)
				{
					parts[r>>8].type = PT_SHLD3;
					parts[r>>8].life = 7;
				}
				else if (parts[r>>8].type==PT_SPRK&&parts[i].life==0)
					for ( nnx=-1; nnx<2; nnx++)
						for ( nny=-1; nny<2; nny++)
						{
							if (!pmap[y+ny+nny][x+nx+nnx])
							{
								create_part(-1,x+nx+nnx,y+ny+nny,PT_SHLD1);
								parts[pmap[y+ny+nny][x+nx+nnx]>>8].life=7;
							}
						}
			}
	return 0;
}
