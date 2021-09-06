  
#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_PSN()
{
	Identifier = "DEFAULT_PT_PSN";
	Name = "PSN";
	Colour = PIXPACK(0x009900);
	MenuVisible = 1;
	MenuSection = SC_BIO;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.10; // sludge
	Loss = 0.5;
	Collision = 0.001f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f  * CFDS;
	Falldown = 2;
	DefaultProperties.tmp = 5; // Potency
	DefaultProperties.life = 500; // Will be able to poison 500 times

	Flammable = 1;
	Explosive = 0;
	Meltable = 0;
	Hardness = 100; //sludge is resistant to acid

	Weight = 75;
	HeatConduct = 1;
	Description = "Poison. Cytotoxic sludge that kills tissue.";

	Properties = TYPE_LIQUID|PROP_NEUTPENETRATE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 500.0f;
	HighTemperatureTransition = PT_POLO; // The poison apparently uses polonium

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;

    rx =  RNG::Ref().between(-2, 2);
    ry =  RNG::Ref().between(-2, 2);
	r = pmap[y+ry][x+rx];
	int ir = ID(r);
	if (sim->elements[TYP(r)].Properties & TYPE_BIO){
		parts[ir].bio.health -= parts[i].tmp;
		parts[i].life--;
	}
    
	if (parts[i].life < 1){
		sim->kill_part(i);
		return 1;
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
    // Oxygen
    int o = cpart->tmp;

    // C02
    int c = cpart->bio.co2;

	int q = cpart->bio.o2;
	*colr = 0;
	*colg = (int)fmax(9 * o, 75);;
	*colb = 0;
	*pixel_mode |= PMODE_BLUR;

	//*colr = int(*colr * (cpart->bio.health) / 100.0f);
	//*colg = int(*colg * (cpart->bio.health) / 100.0f);
	//*colb = int(*colb * (cpart->bio.health) / 100.0f);

	return 0;
}
