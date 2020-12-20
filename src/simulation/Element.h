#ifndef ELEMENTCLASS_H
#define ELEMENTCLASS_H

#include "graphics/Pixel.h"
#include "ElementDefs.h"
#include "Particle.h"
#include "StructProperty.h"

class Simulation;
class Renderer;
class VideoBuffer;
struct Particle;
class Element
{
public:
	ByteString Identifier;
	String Name;
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
	float NewtonianGravity;
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
	unsigned char HeatConduct;
	String Description;
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

	void (*Create)(ELEMENT_CREATE_FUNC_ARGS) = nullptr;
	bool (*CreateAllowed)(ELEMENT_CREATE_ALLOWED_FUNC_ARGS) = nullptr;
	void (*ChangeType)(ELEMENT_CHANGETYPE_FUNC_ARGS) = nullptr;

	bool (*CtypeDraw) (CTYPEDRAW_FUNC_ARGS);

	VideoBuffer * (*IconGenerator)(int, int, int);

	Particle DefaultProperties;

	Element();
	static int defaultGraphics(GRAPHICS_FUNC_ARGS);
	static int legacyUpdate(UPDATE_FUNC_ARGS);
	static bool basicCtypeDraw(CTYPEDRAW_FUNC_ARGS);
	static bool ctypeDrawVInTmp(CTYPEDRAW_FUNC_ARGS);
	static bool ctypeDrawVInCtype(CTYPEDRAW_FUNC_ARGS);

	/** Returns a list of properties, their type and offset within the structure that can be changed
	 by higher-level processes referring to them by name such as Lua or the property tool **/
	static std::vector<StructProperty> const &GetProperties();

#define ELEMENT_NUMBERS_DECLARE
#include "ElementNumbers.h"
#undef ELEMENT_NUMBERS_DECLARE
};

#endif
