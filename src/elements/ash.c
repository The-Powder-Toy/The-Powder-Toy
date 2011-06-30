#include <element.h>

int update_ASH(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rt;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>PS)>=NPART || !r)
						continue;
                    rt = parts[r>>PS].type;
                    if(rt==PT_WOOD||rt==PT_COAL||rt==PT_BCOL||rt==PT_PLNT||rt==PT_VINE||rt==PT_GRAS||rt==PT_OIL||rt==PT_GAS||rt==PT_PLEX||rt==PT_NITR||rt==PT_RBDM||rt==PT_LRBD||rt==PT_THRM||rt==PT_GUNP||rt==PT_BOOM||rt==PT_TNT||rt==PT_FUSE||rt==PT_FUSE2||rt==PT_FSEP||rt==PT_DUST){
                        parts[i].temp+=(rand()/(RAND_MAX/100));
                        parts[i].life = rand()%50+120;
                        parts[i].type = PT_FIRE;
                        parts[i].life = rand()%50+120;
                        parts[i].temp+=(rand()/(RAND_MAX/100));
                    }
				}
	return 0;
}
