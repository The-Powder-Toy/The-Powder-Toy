#include "powder.h"
#include "interface.h"


void menu_count(void)
{
    int i=0;
    msections[SC_WALL].itemcount = UI_WALLCOUNT-4;
    msections[SC_SPECIAL].itemcount = 4;
    for(i=0; i<PT_NUM; i++)
    {
        msections[ptypes[i].menusection].itemcount+=ptypes[i].menu;
    }

}