#include <powder.h>

int update_PRTI(UPDATE_FUNC_ARGS) {
	int r, nnx;
	int count =0;
	parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
	for (ny=-1; ny<2; ny++)
		for (nx=-1; nx<2; nx++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				count ++;
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (parts[r>>8].type==PT_SPRK || (parts[r>>8].type!=PT_PRTI && parts[r>>8].type!=PT_PRTO && (ptypes[parts[r>>8].type].falldown!= 0 || pstates[parts[r>>8].type].state == ST_GAS)))
					for ( nnx=0; nnx<80; nnx++)
						if (!portal[parts[i].tmp][count-1][nnx])
						{
							portal[parts[i].tmp][count-1][nnx] = parts[r>>8].type;
							portaltemp[parts[i].tmp][count-1][nnx] = parts[r>>8].temp;
							if (parts[r>>8].type==PT_SPRK)
								parts[r>>8].type = parts[r>>8].ctype;
							else
								parts[r>>8].type = PT_NONE;
							break;
						}
			}
	return 0;
}
