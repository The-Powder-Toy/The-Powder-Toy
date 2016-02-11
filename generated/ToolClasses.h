#ifndef TOOLCLASSES_H
#define TOOLCLASSES_H

#include <vector>

#include "simulation/simtools/SimTool.h"

#define TOOL_AIR 2
#define TOOL_COOL 1
#define TOOL_HEAT 0
#define TOOL_NGRV 5
#define TOOL_PGRV 4
#define TOOL_VAC 3

class Tool_Air: public SimTool
{
public:
	Tool_Air();
	virtual ~Tool_Air();
	virtual int Perform(Simulation * sim, Particle * cpart, int x, int y, float strength);
};

class Tool_Vac: public SimTool
{
public:
	Tool_Vac();
	virtual ~Tool_Vac();
	virtual int Perform(Simulation * sim, Particle * cpart, int x, int y, float strength);
};

class Tool_NGrv: public SimTool
{
public:
	Tool_NGrv();
	virtual ~Tool_NGrv();
	virtual int Perform(Simulation * sim, Particle * cpart, int x, int y, float strength);
};

class Tool_PGrv: public SimTool
{
public:
	Tool_PGrv();
	virtual ~Tool_PGrv();
	virtual int Perform(Simulation * sim, Particle * cpart, int x, int y, float strength);
};

class Tool_Heat: public SimTool
{
public:
	Tool_Heat();
	virtual ~Tool_Heat();
	virtual int Perform(Simulation * sim, Particle * cpart, int x, int y, float strength);
};

class Tool_Cool: public SimTool
{
public:
	Tool_Cool();
	virtual ~Tool_Cool();
	virtual int Perform(Simulation * sim, Particle * cpart, int x, int y, float strength);
};

std::vector<SimTool*> GetTools();

#endif
