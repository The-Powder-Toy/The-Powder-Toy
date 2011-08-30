#include <element.h>

//Used by ALL 3 BIZR states
int update_BIZR(UPDATE_FUNC_ARGS) {
	int r, rx, ry, nr, ng, nb, na;
	float tr, tg, tb, ta, mr, mg, mb, ma;
	float blend;
	if(parts[i].dcolour){
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)!=PT_BIZR && (r&0xFF)!=PT_BIZRG  && (r&0xFF)!=PT_BIZRS)
					{
						blend = 0.95f;
						tr = (parts[r>>8].dcolour>>16)&0xFF;
						tg = (parts[r>>8].dcolour>>8)&0xFF;
						tb = (parts[r>>8].dcolour)&0xFF;
						ta = (parts[r>>8].dcolour>>24)&0xFF;
						
						mr = (parts[i].dcolour>>16)&0xFF;
						mg = (parts[i].dcolour>>8)&0xFF;
						mb = (parts[i].dcolour)&0xFF;
						ma = (parts[i].dcolour>>24)&0xFF;
						
						nr = (tr*blend) + (mr*(1-blend));
						ng = (tg*blend) + (mg*(1-blend));
						nb = (tb*blend) + (mb*(1-blend));
						na = (ta*blend) + (ma*(1-blend));
						
						parts[r>>8].dcolour = nr<<16 | ng<<8 | nb | na<<24;
					}
				}
	}
	return 0;
}
