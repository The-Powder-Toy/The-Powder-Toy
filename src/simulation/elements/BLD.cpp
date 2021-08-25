#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_BLD()
{
	Identifier = "DEFAULT_PT_BLD";
	Name = "BLD";
	Colour = PIXPACK(0x990000);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f  * CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 35;

	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 29;
	Description = "Blood. Absorbs oxygen and transfers it to other living pixels.";

	Properties = TYPE_LIQUID|PROP_NEUTPENETRATE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;

    rx =  RNG::Ref().between(-2, 2);
    ry =  RNG::Ref().between(-2, 2);

    // O2 use by blood itself
    if (RNG::Ref().chance(1, 200)){

		if (parts[i].tmp > 0){
        	parts[i].tmp -= 1;
		}

		parts[i].tmp2 += 1;
    }

    
    if (BOUNDS_CHECK && (rx || ry))
    {
        r = pmap[y+ry][x+rx];
        if (!r)
            return 0;

		// Oxygen collection
        if (parts[i].tmp < 10 && TYP(r) == PT_O2){
            parts[i].tmp += 5;
            sim->part_change_type(ID(r), x, y, PT_NONE);
        }
		// Diffusion into surrounding blood
		else if (TYP(r) == PT_BLD){

			int ir = ID(r);

			if (parts[i].tmp > parts[ir].tmp){
				parts[i].tmp--;
				parts[ir].tmp++;
			}
			if (parts[i].tmp2 > parts[ir].tmp2){
				parts[i].tmp2--;
				parts[ir].tmp2++;
			}
		}
    }

	// Death check
	if (parts[i].tmp2 > 9){
		sim->part_change_type(i, x, y, PT_DT);
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
    // Oxygen
    int o = cpart->tmp;

    // C02
    int c = cpart->tmp2;

	int q = cpart->tmp;
	*colr = (int)fmax(25 * o, 75);
	*colg = 0;
	*colb = 25 * c;
	*pixel_mode |= PMODE_BLUR;

	return 0;
}
