#ifndef AIR_H
#define AIR_H
#include "Config.h"

class Simulation;

class Air
{
public:
	Simulation & sim;
	int airMode;
	float ambientAirTemp;
	//Arrays from the simulation
	unsigned char (*bmap)[XRES/CELL];
	unsigned char (*emap)[XRES/CELL];
	float (*fvx)[XRES/CELL];
	float (*fvy)[XRES/CELL];
	//
	float vx[YRES/CELL][XRES/CELL];
	float ovx[YRES/CELL][XRES/CELL];
	float vy[YRES/CELL][XRES/CELL];
	float ovy[YRES/CELL][XRES/CELL];
	float pv[YRES/CELL][XRES/CELL];
	float opv[YRES/CELL][XRES/CELL];
	float hv[YRES/CELL][XRES/CELL];
	float ohv[YRES/CELL][XRES/CELL]; // Ambient Heat
	unsigned char bmap_blockair[YRES/CELL][XRES/CELL];
	unsigned char bmap_blockairh[YRES/CELL][XRES/CELL];
	float kernel[9];
	void make_kernel(void);
	void update_airh(void);
	void update_air(void);
	void Clear();
	void ClearAirH();
	void Invert();
	void RecalculateBlockAirMaps();
	Air(Simulation & sim);
};

#endif
