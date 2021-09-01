#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_LUNG()
{
	Identifier = "DEFAULT_PT_LUNG";
	Name = "LUNG";
	Colour = PIXPACK(0x990066);
	MenuVisible = 1;
	MenuSection = SC_BIO;
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

	DefaultProperties.bio.health = 100;
	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 29;
	Description = "Lung. Peforms gas exchange with Blood (BLD).";

	Properties = TYPE_SOLID|PROP_NEUTPENETRATE|TYPE_BIO;

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

		if (parts[i].bio.o2 > 0){
        	parts[i].bio.o2 -= 1;
			parts[i].bio.co2 += 1;
		}
    }

    
    if (BOUNDS_CHECK && (rx || ry))
    {
        r = pmap[y+ry][x+rx];
		int ir = ID(r);

        if (r) {
			// Oxygen collection (more efficient than BLD)
			if (parts[i].bio.o2 < 75 && TYP(r) == PT_O2){
				parts[i].bio.o2 += 20;

				// Replace with CO2 (if present)
				if (parts[i].bio.co2 > 0){
					sim->part_change_type(ID(r), x, y, PT_CO2);
					parts[i].bio.co2 -= 25;
				}
				else{
					sim->part_change_type(ID(r), x, y, PT_NONE);
				}
			}
			// Blood interactions
			else if (TYP(r) == PT_BLD || parts[ir].ctype == PT_BLD){
				// Give oxygen
				if (parts[ir].bio.o2 < MAX_O2 && parts[i].bio.o2 > 0 && parts[i].bio.o2 > parts[ir].bio.o2){
					parts[ir].bio.o2 += 2;
					parts[i].bio.o2 -= 2;
				}

				// Take CO2
				if (parts[ir].bio.co2 > 0){
					parts[i].bio.co2 += 2;
					parts[ir].bio.co2 -= 2;
				}
			}
			// Diffuse among self
			else if (TYP(r) == PT_LUNG){

				for (int zz = 0; zz < 3; zz++){
					if (parts[i].bio.o2 > parts[ir].bio.o2){
						parts[i].bio.o2--;
						parts[ir].bio.o2++;
					}
					if (parts[i].bio.co2 > parts[ir].bio.co2){
						parts[i].bio.co2--;
						parts[ir].bio.co2++;
					}
				}
			}
		}
    }

	// Health management
	if (RNG::Ref().chance(1, 50)){
		// Temp check
		if (parts[i].temp > 323.15){
			int damage = (parts[i].temp - 315) / 5;
			parts[i].bio.health -= damage;
		}
		// Damage check
		if (parts[i].bio.co2 > 100 || parts[i].bio.o2 < 1){
			parts[i].bio.health--;
		}
		// Otherwise heal
		else{
			if (parts[i].bio.health < 100){
				parts[i].bio.health++;
			}
		}
	}

	// Death check
	if (parts[i].bio.health < 1){
		sim->part_change_type(i, x, y, PT_DT);
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
    // Oxygen
    int o = cpart->bio.o2;

    // C02
    int c = cpart->bio.co2;
	
	*colr = (int)fmax(7 * o, 100);
	*colg = 0;
	*colb = (int)fmax(7 * o, 60);
	*pixel_mode |= PMODE_BLUR;

	*colr = int(*colr * (cpart->bio.health) / 100.0f);
	*colg = int(*colg * (cpart->bio.health) / 100.0f);
	*colb = int(*colb * (cpart->bio.health) / 100.0f);

	return 0;
}
