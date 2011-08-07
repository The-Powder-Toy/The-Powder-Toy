#include <element.h>

int update_RDOT(UPDATE_FUNC_ARGS)
{
    int nb;
    int self = parts[i].type;
    int life = 200, speed = 1;
    if (parts[i].life==0)
    {
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
    return 0;
}
