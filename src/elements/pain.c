#include <element.h>

int update_PAIN(UPDATE_FUNC_ARGS) {
	int r,rx,ry;
    pv[y/CELL][x/CELL] += 10.0f;
    if(y+CELL<YRES)
        pv[y/CELL+1][x/CELL] += 10.0f;
    if(x+CELL<XRES)
    {
        pv[y/CELL][x/CELL+1] += 10.0f;
        if(y+CELL<YRES)
            pv[y/CELL+1][x/CELL+1] += 10.0f;
    }
    if(parts[pmap[y][x]>>8].temp<MAX_TEMP)
    {
        if((pmap[y][x]&0xFF)==PT_PUMP)
            parts[pmap[y][x]>>8].temp = restrict_flt(parts[pmap[y][x]>>8].temp + 0.1f, MIN_TEMP, MAX_TEMP);
        else 
            parts[pmap[y][x]>>8].temp = restrict_flt(parts[pmap[y][x]>>8].temp + 1000.0f, MIN_TEMP, MAX_TEMP);
    }
    parts[i].type=PT_NONE;
    return -1;
}
