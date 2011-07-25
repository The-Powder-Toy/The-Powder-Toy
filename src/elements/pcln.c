#include <element.h>

int update_PCLN(UPDATE_FUNC_ARGS)
{
    int r, rx, ry;
    int self = parts[i].type;
    if (parts[i].life>0 && parts[i].life!=10)
        parts[i].life--;
    for (rx=-2; rx<3; rx++)
        for (ry=-2; ry<3; ry++)
            if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if ((r>>PS)>=NPART || !r)
                    continue;
                if (parts[r>>PS].type==PT_SPRK)
                {
                    if (parts[r>>PS].ctype==PT_PSCN)
                        parts[i].life = 10;
                    else if (parts[r>>PS].ctype==PT_NSCN)
                        parts[i].life = 9;
                }
                if (parts[r>>PS].type==self)
                {
                    if (parts[i].life==10&&parts[r>>PS].life<10&&parts[r>>PS].life>0)
                        parts[i].life = 9;
                    else if (parts[i].life==0&&parts[r>>PS].life==10)
                        parts[i].life = 10;
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
                    if (parts[r>>PS].type!=PT_CLNE && parts[r>>PS].type!=PT_PCLN &&
                            parts[r>>PS].type!=PT_BCLN &&  parts[r>>PS].type!=PT_SPRK &&
                            parts[r>>PS].type!=PT_NSCN && parts[r>>PS].type!=PT_PSCN &&
                            parts[r>>PS].type!=PT_STKM && parts[r>>PS].type!=PT_STKM2 &&
                            parts[r>>PS].type!=self && parts[r>>PS].type<PT_NUM)
                    {
                        parts[i].ctype = r&TYPE;
                        if ((r&TYPE)==PT_LIFE)
                            parts[i].tmp = parts[r>>PS].ctype;
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
