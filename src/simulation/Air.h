#pragma once
#include "SimulationConfig.h"

class Simulation;

class Air
{
public:
	Simulation & sim;
	int airMode;
	float ambientAirTemp;
	//Arrays from the simulation
	unsigned char (*bmap)[XCELLS];
	unsigned char (*emap)[XCELLS];
	float (*fvx)[XCELLS];
	float (*fvy)[XCELLS];
	//
	float vx[YCELLS][XCELLS];
	float ovx[YCELLS][XCELLS];
	float vy[YCELLS][XCELLS];
	float ovy[YCELLS][XCELLS];
	float pv[YCELLS][XCELLS];
	float opv[YCELLS][XCELLS];
	float hv[YCELLS][XCELLS];
	float ohv[YCELLS][XCELLS]; // Ambient Heat
	unsigned char bmap_blockair[YCELLS][XCELLS];
	unsigned char bmap_blockairh[YCELLS][XCELLS];
	float kernel[9];
	void make_kernel(void);
	void update_airh(void);
	void update_air(void);
	void Clear();
	void ClearAirH();
	void Invert();
	void ApproximateBlockAirMaps();
	Air(Simulation & sim);
};
