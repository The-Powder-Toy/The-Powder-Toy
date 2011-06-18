#include <element.h>

int update_ARAY(UPDATE_FUNC_ARGS) {
	int r, nxx, nyy, docontinue, nxi, nyi, rx, ry, nr;
	if (parts[i].life==0) {
		int colored =0;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>8)>=NPART || !r)
						continue;
					if ((r&0xFF)==PT_SPRK) {
						int destroy = (parts[r>>8].ctype==PT_PSCN)?1:0;
						int nostop = (parts[r>>8].ctype==PT_INST)?1:0;
						for (docontinue = 1, nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1; docontinue; nyy+=nyi, nxx+=nxi) {
							if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
								break;
							}
							r = pmap[y+nyi+nyy][x+nxi+nxx];
							if (!((r>>8)>=NPART)) {
								if (!r) {
									int nr = create_part(-1, x+nxi+nxx, y+nyi+nyy, PT_BRAY);
									if (nr!=-1) {
										if (destroy) {//if it came from PSCN
											parts[nr].tmp = 2;
											parts[nr].life = 2;
										} else
											parts[nr].ctype = colored;
									}
								} else if (!destroy) {
									if ((r&0xFF)==PT_BRAY&&parts[r>>8].tmp==0) {//if it hits another BRAY that isn't red
										if (nyy!=0 || nxx!=0) {
											parts[r>>8].life = 1020;//makes it last a while
											parts[r>>8].tmp = 1;
											if (!parts[r>>8].ctype)//and colors it if it isn't already
												parts[r>>8].ctype = colored;
										}
										docontinue = 0;//then stop it
									} else if ((r&0xFF)==PT_BRAY&&parts[r>>8].tmp==1) {//if it hits one that already was a long life, reset it
										parts[r>>8].life = 1020;
										//docontinue = 1;
									}
									else if ((r&0xFF)==PT_FILT) {//get color if passed through FILT
										colored = parts[r>>8].ctype;
										//this if prevents BRAY from stopping on certain materials
									} else if ((r&0xFF)!=PT_INWR && (r&0xFF)!=PT_ARAY && (r&0xFF)!=PT_WIFI && !((r&0xFF)==PT_SWCH && parts[r>>8].life>=10)) {
										if (nyy!=0 || nxx!=0) {
											create_part(-1, x+nxi+nxx, y+nyi+nyy, PT_SPRK);
										}
										if (!(nostop && (ptypes[parts[r>>8].ctype].properties&PROP_CONDUCTS))) {
											docontinue = 0;
										} else {
											docontinue = 1;
										}
									}
								} else if (destroy) {
									if ((r&0xFF)==PT_BRAY) {
										parts[r>>8].life = 1;
										docontinue = 1;
										//this if prevents red BRAY from stopping on certain materials
									} else if ((r&0xFF)==PT_INWR || (r&0xFF)==PT_ARAY || (r&0xFF)==PT_WIFI || (r&0xFF)==PT_FILT || ((r&0xFF)==PT_SWCH && parts[r>>8].life>=10)) {
										docontinue = 1;
									} else {
										docontinue = 0;
									}
								}
							}
						}
					}
					//parts[i].life = 4;
				}
	}
	return 0;
}
