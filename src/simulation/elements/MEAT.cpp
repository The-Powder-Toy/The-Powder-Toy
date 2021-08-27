#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_MEAT()
{
	Identifier = "DEFAULT_PT_MEAT";
	Name = "MEAT";
	Colour = PIXPACK(0x990022);
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

    DefaultProperties.life = 100;
	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 29;
	Description = "Meat. Basic biological material.";

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

    // O2 use by meat itself
    if (RNG::Ref().chance(1, 150)){

		if (parts[i].tmp > 0){
        	parts[i].tmp -= 1;
            parts[i].tmp2 += 1;
		}
    }

    
    if (BOUNDS_CHECK && (rx || ry))
    {
        r = pmap[y+ry][x+rx];
        if (r) {
            // Blood interactions
            if (TYP(r) == PT_BLD){

                int ir = ID(r);

                // Give CO2
                if (parts[ir].tmp2 < 10 && parts[i].tmp2 > 0){
                    parts[ir].tmp2 += 1;
                    parts[i].tmp2 -= 1;
                }

                // Take O2
                if (parts[ir].tmp > 0 && parts[i].tmp < 10){
                    parts[i].tmp++;
                    parts[ir].tmp--;
                }
            }
            // Diffuse among self
            else if (TYP(r) == PT_MEAT){
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
    }

	// Health management
	if (RNG::Ref().chance(1, 50)){
		// Damage check
		if (parts[i].tmp2 > 9 || parts[i].tmp < 1){
			parts[i].life--;
		}
		// Otherwise heal
		else{
            if (parts[i].life < 100){
				parts[i].life++;
			}
		}
	}

	// Death check
	if (parts[i].life < 1){
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
	*colb = (int)fmax(10 * o, 30);
	*pixel_mode |= PMODE_BLUR;

	return 0;
}
