#include "simulation/ToolCommon.h"

static int perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength);

void SimTool::Tool_AMBP() {
    Identifier = "DEFAULT_TOOL_AMBP";
    Name = "AMB+";
    Colour = PIXPACK(0xFFDD00);
    Description = "Air heater. (Ambient heat required).";
    Perform = &perform;
}

static int perform(Simulation *sim, Particle *cpart, int x, int y, int brushX, int brushY, float strength) {
    if (!sim->aheat_enable)
        return 0;
    
    sim->hv[y / CELL][x / CELL] += strength * 2.0f;
    if (sim->hv[y / CELL][x / CELL] > MAX_TEMP)
        sim->hv[y / CELL][x / CELL] = MAX_TEMP;
    else if (sim->hv[y / CELL][x / CELL] < 0)
        sim->hv[y / CELL][x / CELL] = 0;
    return 1;
}