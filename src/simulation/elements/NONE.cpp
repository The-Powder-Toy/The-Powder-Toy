#include "simulation/ElementCommon.h"

static VideoBuffer *iconGen(int wallID, int width, int height);

void Element::Element_NONE()
{
	Identifier = "DEFAULT_PT_NONE";
	Name = "NONE";
	Colour = PIXPACK(0x000000);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	HeatConduct = 0;
	Description = "Erases particles.";

	Properties = 0;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	IconGenerator = &iconGen;
}

static VideoBuffer *iconGen(int wallID, int width, int height)
{
	VideoBuffer * newTexture = new VideoBuffer(width, height);

	for (int j=3; j<(width-4)/2; j++)
	{
		newTexture->SetPixel(j+6, j, 0xFF, 0, 0, 255);
		newTexture->SetPixel(j+7, j, 0xFF, 0, 0, 255);
		newTexture->SetPixel(-j+19, j, 0xFF, 0, 0, 255);
		newTexture->SetPixel(-j+20, j, 0xFF, 0, 0, 255);
	}

	return newTexture;
}
