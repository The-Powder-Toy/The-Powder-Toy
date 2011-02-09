#include <element.h>

int update_PRTI(UPDATE_FUNC_ARGS) {
	int r, nnx, rx, ry;
	int count =0;
	parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
	if (parts[i].tmp>=CHANNELS) parts[i].tmp = CHANNELS-1;
	else if (parts[i].tmp<0) parts[i].tmp = 0;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				count ++;
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_SPRK || ((r&0xFF)!=PT_PRTI && (r&0xFF)!=PT_PRTO && (ptypes[r&0xFF].falldown!= 0 || ptypes[r&0xFF].state == ST_GAS)))
					for ( nnx=0; nnx<80; nnx++)
						if (!portal[parts[i].tmp][count-1][nnx])
						{
							portal[parts[i].tmp][count-1][nnx] = parts[r>>8].type;
							portaltemp[parts[i].tmp][count-1][nnx] = parts[r>>8].temp;
							portalctype[parts[i].tmp][count-1][nnx] = parts[r>>8].ctype;
							if ((r&0xFF)==PT_SPRK)
								part_change_type(r>>8,x+rx,y+ry,parts[r>>8].ctype);
							else
								kill_part(r>>8);
							break;
						}
			}
	return 0;
}
