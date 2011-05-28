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
    if(parts[pmap[y][x]>>PS].temp<MAX_TEMP)
    {
        if((pmap[y][x]&TYPE)==PT_PUMP)
            parts[pmap[y][x]>>PS].temp = restrict_flt(parts[pmap[y][x]>>PS].temp + 0.1f, MIN_TEMP, MAX_TEMP);
        else
            parts[pmap[y][x]>>PS].temp = restrict_flt(parts[pmap[y][x]>>PS].temp + 1000.0f, MIN_TEMP, MAX_TEMP);
    }
    parts[i].type=PT_NONE;
    return -1;
}
