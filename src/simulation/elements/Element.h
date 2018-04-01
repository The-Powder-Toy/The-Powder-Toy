#ifndef ELEMENTCLASS_H
#define ELEMENTCLASS_H

#include "graphics/Pixel.h"
#include "simulation/Simulation.h"
#include "simulation/Elements.h"
#include "simulation/StructProperty.h"

class Simulation;
class Renderer;
class VideoBuffer;
struct Particle;
class Element
{
public:
	const char *Identifier;
	const char *Name;
	pixel Colour;
	int MenuVisible;
	int MenuSection;
	int Enabled;

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
	// Photon wavelengths are ANDed with this value when a photon hits an element, meaning that only wavelengths present in both this value and the original photon will remain in the reflected photon
	unsigned int PhotonReflectWavelengths;
	int Weight;
	float Temperature;
	unsigned char HeatConduct;
	const char *Description;
	unsigned int Properties;

	float LowPressure;
	int LowPressureTransition;
	float HighPressure;
	int HighPressureTransition;
	float LowTemperature;
	int LowTemperatureTransition;
	float HighTemperature;
	int HighTemperatureTransition;

	int (*Update) (UPDATE_FUNC_ARGS);
	int (*Graphics) (GRAPHICS_FUNC_ARGS);
	VideoBuffer * (*IconGenerator)(int, int, int);

	Element();
	virtual ~Element() {}
	static int defaultGraphics(GRAPHICS_FUNC_ARGS);
	static int legacyUpdate(UPDATE_FUNC_ARGS);

	/** Returns a list of properties, their type and offset within the structure that can be changed
	 by higher-level processes referring to them by name such as Lua or the property tool **/
	static std::vector<StructProperty> GetProperties();
};

#endif
