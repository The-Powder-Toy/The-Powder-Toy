#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_LUNG()
{
	Identifier = "DEFAULT_PT_LUNG";
	Name = "LUNG";
	Colour = PIXPACK(0x990066);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.97f;
	Loss = 0.50f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 12;

	Weight = 150;

	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 29;
	Description = "Lung. Peforms gas exchange with Blood (BLD).";

	Properties = TYPE_SOLID|PROP_NEUTPENETRATE;

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

    // O2 use by lung itself
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

		// Oxygen collection (more efficient than BLD)
        if (parts[i].tmp < 30 && TYP(r) == PT_O2){
            parts[i].tmp += 8;

			// Replace with CO2 (if present)
			if (parts[i].tmp2 > 0){
				sim->part_change_type(ID(r), x, y, PT_CO2);
				parts[i].tmp2 -= 8;
			}
			else{
				sim->part_change_type(ID(r), x, y, PT_NONE);
			}
        }
		// Blood interactions
		else if (TYP(r) == PT_BLD){

			int ir = ID(r);

			// Give oxygen
			if (parts[ir].tmp < 10 && parts[i].tmp > 0){
				parts[ir].tmp += 1;
				parts[i].tmp -= 1;
			}

			// Take CO2
			if (parts[ir].tmp2 > 0){
				parts[i].tmp2++;
				parts[ir].tmp2--;
			}
		}
		// Diffuse among self
		else if (TYP(r) == PT_LUNG){
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
	if (parts[i].tmp2 > 50){
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
	*colr = (int)fmax(20 * o, 100);
	*colg = 0;
	*colb = (int)fmax(20 * o, 60);
	*pixel_mode |= PMODE_BLUR;

	return 0;
}
