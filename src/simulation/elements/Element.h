#ifndef ELEMENTCLASS_H
#define ELEMENTCLASS_H

#include "simulation/Simulation.h"
#include "graphics/Renderer.h"
#include "simulation/Elements.h"
#include "simulation/StructProperty.h"

class Simulation;
class Renderer;
struct Particle;
class Element
{
public:
	char *Identifier;
	char *Name;
	pixel Colour;
	float Advection;
	float AirDrag;
	float AirLoss;
	float Loss;
	float Collision;
	float Gravity;
	float Diffusion;
	float HotAir;
	int Falldown;
	int Flammable;
	int Explosive;
	int Meltable;
	int Hardness;
	int MenuVisible;
	int Enabled;
	int Weight;
	int MenuSection;
	float Temperature;
	unsigned char HeatConduct;
	char *Description;
	char State;
	unsigned int Properties;
	int (*Update) (UPDATE_FUNC_ARGS);
	int (*Graphics) (GRAPHICS_FUNC_ARGS);
	VideoBuffer * (*IconGenerator)(int, int, int);
	
	float HighPressure;
	int HighPressureTransition;
	float LowPressure;
	int LowPressureTransition;
	float HighTemperature;
	int HighTemperatureTransition;
	float LowTemperature;
	int LowTemperatureTransition;
	
	Element();
	virtual ~Element() {}
	static int defaultGraphics(GRAPHICS_FUNC_ARGS);
	static int legacyUpdate(UPDATE_FUNC_ARGS);

	/** Returns a list of properties, their type and offset within the structure that can be changed
	 by higher-level processes refering to them by name such as Lua or the property tool **/
	static std::vector<StructProperty> GetProperties();
};

#endif
