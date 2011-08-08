#include <element.h>

int update_RDOT(UPDATE_FUNC_ARGS)
{
    int nb,rx,ry,r;
    int self = parts[i].type;
    int life = 200, speed = 1;
    if (parts[i].life==0)
    {
        for(rx=-2; rx<3; rx++)
            for(ry=-2; ry<3; ry++)
                if(x+rx>=0 && y+ry>0 &&
                        x+rx<XRES && y+ry<YRES && (rx || ry))
                {
                    r = pmap[y+ry][x+rx];
                    if((r>>PS)>=NPART || !r || parts[r>>PS].type!=PT_SPRK && parts[r>>PS].type!=PT_BRAY)
                        continue;
                    parts[i].life = 16;
                    nb = create_part(-1, x, y+1, PT_RDOW);
                    if(nb!=-1)
                    {
                        parts[nb].life = life;
                        parts[nb].vy = speed;
                    }
                    nb = create_part(-1, x+1, y+1, PT_RDOW);
                    if(nb!=-1)
                    {
                        parts[nb].life = life;
                        parts[nb].vy = speed;
                        parts[nb].vx = speed;
                    }
                    nb = create_part(-1, x+1, y, PT_RDOW);
                    if(nb!=-1)
                    {
                        parts[nb].life = life;
                        parts[nb].vx = speed;
                    }
                    nb = create_part(-1, x+1, y-1, PT_RDOW);
                    if(nb!=-1)
                    {
                        parts[nb].life = life;
                        parts[nb].vx = speed;
                        parts[nb].vy = -speed;
                    }
                    nb = create_part(-1, x, y-1, PT_RDOW);
                    if(nb!=-1)
                    {
                        parts[nb].life = life;
                        parts[nb].vy = -speed;
                    }
                    nb = create_part(-1, x-1, y-1, PT_RDOW);
                    if(nb!=-1)
                    {
                        parts[nb].life = life;
                        parts[nb].vy = -speed;
                        parts[nb].vx = -speed;
                    }
                    nb = create_part(-1, x-1, y, PT_RDOW);
                    if(nb!=-1)
                    {
                        parts[nb].life = life;
                        parts[nb].vx = -speed;
                    }
                    nb = create_part(-1, x-1, y+1, PT_RDOW);
                    if(nb!=-1)
                    {
                        parts[nb].life = life;
                        parts[nb].vx = -speed;
                        parts[nb].vy = speed;
                    }
                }
    }
    return 0;
}

int update_RDOR(UPDATE_FUNC_ARGS)
{
    int r,rx,ry,t;
    int self = parts[i].type;
    for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 &&
                    x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if (parts[r>>PS].type==PT_RDOW)
                {
                    parts[i].tmp += parts[r>>PS].life / 20;
                    if (parts[i].tmp==0)
                    {
                        parts[i].tmp = 1;
                    }
                }
                if (parts[r>>PS].properties&PROP_CONDUCTS && parts[i].tmp > 0 && parts[r>>PS].life == 0)
                {
                    parts[r>>PS].ctype = parts[r>>PS].type;
                    part_change_type(r>>PS,rx,ry,PT_SPRK);
                    parts[r>>PS].life = 4;
                    parts[i].tmp--;
                }
            }
    return 0;
}
