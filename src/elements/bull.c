#include <element.h>

int update_BULL(UPDATE_FUNC_ARGS) {
	int r,rx,ry, nx, ny;
    parts[i].vx=3;
    int nb;
    if(parts[i].tmp==1){
        for(nx=-2; nx<3; nx++)
            for(ny=-2; ny<3; ny++)
                if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
                {
                    r = pmap[y+ny][x+nx];
                    if((r>>8)>=NPART || !r)
                        continue;
                    if(parts[r>>8].type!=PT_NONE && parts[r>>8].type!=PT_BULL){
                        parts[i].type = PT_NONE;
                        kill_part(i);
                    }
                }
    } else if(parts[i].tmp==0){
        for(nx=-2; nx<3; nx++)
            for(ny=-2; ny<3; ny++)
                if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES)
                {
                    r = pmap[y+ny][x+nx];
                    if((r>>8)>=NPART || !r)
                        continue;
                    if(parts[r>>8].type!=PT_NONE && parts[r>>8].type!=PT_BULL && parts[r>>8].type!=PT_DMND && parts[r>>8].type!=PT_C0 && parts[r>>8].type!=PT_GOLD && parts[r>>8].type!=PT_BSHL && parts[r>>8].type!=PT_PDCL && parts[r>>8].type!=PT_LQCL && parts[r>>8].type!=PT_GSCL && parts[r>>8].type!=PT_CLNE && parts[r>>8].type!=PT_PCLN && parts[r>>8].type!=PT_BCLN){
                        int rad = 8;
                        int nxi;
                        int nxj;
                        pmap[y][x] = 0;
                        for(nxj=-(rad+1); nxj<=(rad+1); nxj++)
                            for(nxi=-(rad+1); nxi<=(rad+1); nxi++)
                                if((pow(nxi,2))/(pow((rad+1),2))+(pow(nxj,2))/(pow((rad+1),2))<=1){
                                    nb = create_part(-1, x+nxi, y+nxj, PT_BULL);
                                    if(nb!=-1){
                                        parts[nb].tmp = 1;
                                        parts[nb].life = 50;
                                        parts[nb].temp = MAX_TEMP;
                                        parts[nb].vx = rand()%20-10;
                                        parts[nb].vy = rand()%20-10;
                                    }
                                }
                        for(nxj=-rad; nxj<=rad; nxj++)
                            for(nxi=-rad; nxi<=rad; nxi++)
                                if((pow(nxi,2))/(pow(rad,2))+(pow(nxj,2))/(pow(rad,2))<=1)
                                    if((pmap[y+nxj][x+nxi]&0xFF)!=PT_DMND && (pmap[y+nxj][x+nxi]&0xFF)!=PT_GOLD && (pmap[y+nxj][x+nxi]&0xFF)!=PT_CLNE && (pmap[y+nxj][x+nxi]&0xFF)!=PT_PCLN && (pmap[y+nxj][x+nxi]&0xFF)!=PT_BCLN){
                                        delete_part(x+nxi, y+nxj);
                                        pv[(y+nxj)/CELL][(x+nxi)/CELL] += 0.1f;
                                        nb = create_part(-1, x+nxi, y+nxj, PT_BULL);
                                        if(nb!=-1){
                                            parts[nb].tmp = 2;
                                            parts[nb].life = 2;
                                            parts[nb].temp = MAX_TEMP;
                                        }
                                    }
                        //create_parts(x, y, 9, 9, PT_BULL);
                        //create_parts(x, y, 8, 8, PT_NONE);
                        parts[i].type = PT_NONE;
                        kill_part(i);
                    }
                }
    }
    return 0;
}
