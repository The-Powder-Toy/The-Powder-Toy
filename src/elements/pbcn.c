#include <element.h>

int update_PBCN(UPDATE_FUNC_ARGS)
{
    int r, rx, ry;
    int self = parts[i].type;
    if (parts[i].life>0 && parts[i].life!=10)
        parts[i].life--;
    if (!parts[i].tmp2 && pv[y/CELL][x/CELL]>4.0f)
        parts[i].tmp2 = rand()%40+80;
    if (parts[i].tmp2)
    {
        float advection = 0.1f;
        parts[i].vx += advection*vx[y/CELL][x/CELL];
        parts[i].vy += advection*vy[y/CELL][x/CELL];
        parts[i].tmp2--;
        if(!parts[i].tmp2)
        {
            kill_part(i);
            return 1;
        }
    }
    if (parts[i].ctype<=0 || parts[i].ctype>=PT_NUM || (parts[i].ctype==PT_LIFE && (parts[i].tmp<0 || parts[i].tmp>=NGOLALT)))
        for (rx=-1; rx<2; rx++)
            for (ry=-1; ry<2; ry++)
                if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
                {
                    r = photons[y+ry][x+rx];
                    if (!r || (r>>PS)>=NPART)
                        r = pmap[y+ry][x+rx];
                    if (!r || (r>>PS)>=NPART)
                        continue;
                    if ((r&TYPE)!=PT_CLNE && (r&TYPE)!=PT_PCLN &&
                            (r&TYPE)!=PT_BCLN &&  (r&TYPE)!=PT_SPRK &&
                            (r&TYPE)!=PT_NSCN && (r&TYPE)!=PT_PSCN &&
                            (r&TYPE)!=PT_STKM && (r&TYPE)!=PT_STKM2 &&
                            (r&TYPE)!=PT_PBCN && (r&TYPE)!=self && (r&TYPE)<PT_NUM)
                        {
                            parts[i].ctype = r&TYPE;
                            if ((r&TYPE)==PT_LIFE)
                                parts[i].tmp = parts[r>>PS].ctype;
                        }
                }
    if (parts[i].life==10)
    {

        for (rx=-2; rx<3; rx++)
            for (ry=-2; ry<3; ry++)
                if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
                {
                    r = pmap[y+ry][x+rx];
                    if ((r>>PS)>=NPART || !r)
                        continue;
                    if ((r&TYPE)==self)
                    {
                        if (parts[r>>PS].life<10&&parts[r>>PS].life>0)
                            parts[i].life = 9;
                        else if (parts[r>>PS].life==0)
                            parts[r>>PS].life = 10;
                    }
                }
    }
    if (parts[i].ctype>0 && parts[i].ctype<PT_NUM && parts[i].life==10)
    {
        if (parts[i].ctype==PT_PHOT)  //create photons a different way
        {
            for (rx=-1; rx<2; rx++)
            {
                for (ry=-1; ry<2; ry++)
                {
                    int r = create_part(-1, x+rx, y+ry, parts[i].ctype);
                    if (r!=-1)
                    {
                        parts[r].vx = rx*3;
                        parts[r].vy = ry*3;
                    }
                }
            }
        }
        else if (parts[i].ctype==PT_LIFE)//create life a different way
        {
            for (rx=-1; rx<2; rx++)
            {
                for (ry=-1; ry<2; ry++)
                {
                    create_part(-1, x+rx, y+ry, parts[i].ctype|(parts[i].tmp<<PS));
                }
            }
        }
        else
        {
            create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
        }
    }
    return 0;
}
