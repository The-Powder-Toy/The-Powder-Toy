#include <element.h>

int update_ZAP(UPDATE_FUNC_ARGS) {
	int rx,ry;
    float r = (rand()%128+128)/127.0f;
    float a = (rand()%360)*3.14159f/180.0f;
    parts[i].life = rand()%480+480;
    parts[i].vx = r*cosf(a);
    parts[i].vy = r*sinf(a);
    return 0;
}
