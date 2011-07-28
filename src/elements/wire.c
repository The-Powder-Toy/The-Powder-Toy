#include <element.h>
 
int update_WIRE(UPDATE_FUNC_ARGS) {
    int s,r,rx,ry,count;
    printf("FUCK");
    s=pmap[y][x];
    /*
    0:  wire
    1:  spark head
    2:  spark tail
    
    tmp is previous state, ctype is current state
    */
    parts[s>>8].tmp=parts[s>>8].ctype;
    if(parts[s>>8].tmp==1){parts[s>>8].ctype=2;}
    if(parts[s>>8].tmp==2){parts[s>>8].ctype=0;}
    
    for(rx=-1; rx<2; rx++)
        for(ry=-1; ry<2; ry++)
            {
            count=0;
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && pmap[y+ry][x+rx] && rx==0 && ry==0 && (pmap[y+ry][x+rx]&0xFF)!=0xFF)
                {
                r = pmap[y+ry][x+rx];
                if((r&0xFF)==PT_SPRK){parts[s>>8].ctype=1;}
                if((r&0xFF)==PT_WIRE && parts[r>>8].tmp==1){count++;}
                }
            if(count==1 || count==2)
                parts[s>>8].ctype=1;
            }
	return 0;
}
