#include <element.h>

int update_CPPA(UPDATE_FUNC_ARGS)
{
    int r, rx, ry,tmp;
    tmp = parts[i].tmp;
    if (tmp == PT_NONE || tmp == PT_VIRS || tmp == PT_STKM ||
            tmp == PT_STKM2 || tmp == PT_CPPA || tmp == PT_PLAN ||
            tmp == PT_NUM || tmp == PT_PHOT || tmp==PT_NEUT ||
            tmp > PT_NUM || ptypes[tmp].menusection==SC_DRAWING ||
            tmp == PT_SPAWN || tmp == PT_SPAWN2 || ptypes[tmp].enabled==0 ||
            tmp == PT_ZAP || tmp == PT_PRTN || tmp == PT_LIFE || tmp == PT_SPRK)
    {
        tmp = 1;
    }
    part_change_type(i, x, y, tmp);
    return 0;
}
