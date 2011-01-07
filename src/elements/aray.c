#include <powder.h>

int update_ARAY(UPDATE_FUNC_ARGS) {
	int r, nxx, nyy, docontinue, nxi, nyi;
	if (parts[i].life==0) {
		int colored =0;
		for (nx=-1; nx<2; nx++) {
			for (ny=-1; ny<2; ny++) {
				if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny)) {
					r = pmap[y+ny][x+nx];
					if ((r>>8)>=NPART || !r)
						continue;
					if ((r&0xFF)==PT_SPRK) {
						int destroy = (parts[r>>8].ctype==PT_PSCN)?1:0;
						int nostop = (parts[r>>8].ctype==PT_INST)?1:0;
						for (docontinue = 1, nxx = 0, nyy = 0, nxi = nx*-1, nyi = ny*-1; docontinue; nyy+=nyi, nxx+=nxi) {
							if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
								break;
							}
							r = pmap[y+nyi+nyy][x+nxi+nxx];
							if (!((r>>8)>=NPART)) {
								if (!r) {
									int nr = create_part(-1, x+nxi+nxx, y+nyi+nyy, PT_BRAY);
									if (nr!=-1) {
										if (destroy) {
											parts[nr].tmp = 2;
											parts[nr].life = 2;
										} else
											parts[nr].ctype = colored;
									}
								} else if (!destroy) {
									if (parts[r>>8].type==PT_BRAY&&parts[r>>8].tmp==0) {
										if (nyy!=0 || nxx!=0) {
											parts[r>>8].type = PT_BRAY;
											parts[r>>8].life = 1020;
											parts[r>>8].tmp = 1;
											if (!parts[r>>8].ctype)
												parts[r>>8].ctype = colored;
										}
										docontinue = 0;
									} else if (parts[r>>8].type==PT_BRAY&&parts[r>>8].tmp==1) {
										parts[r>>8].life = 1020;
										//docontinue = 1;
									}
									else if (parts[r>>8].type==PT_FILT) {
										colored = parts[r>>8].ctype;
									} else if (parts[r>>8].type!=PT_INWR && parts[r>>8].type!=PT_ARAY && parts[r>>8].type!=PT_WIFI) {
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
									if (parts[r>>8].type==PT_BRAY) {
										parts[r>>8].life = 1;
										docontinue = 1;
									} else if (parts[r>>8].type==PT_INWR || parts[r>>8].type==PT_ARAY || parts[r>>8].type==PT_WIFI) {
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
		}
	}
	return 0;
}
