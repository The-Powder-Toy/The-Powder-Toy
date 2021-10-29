#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_TUMOR()
{
	Identifier = "DEFAULT_PT_TUMOR";
	Name = "TUMR";
	Colour = PIXPACK(0x554040);
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

	DefaultProperties.bio.health = 150; // In real life, cancer survives better than regular tissue (citation: HeLa)
	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 29;
	Description = "Tumor. Certain death/blobification for biology";

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

    // O2 use by tumor itself (Increased due to increased biological activity)
    if (RNG::Ref().chance(1, 100)){

		if (parts[i].bio.o2 > 0){
        	parts[i].bio.o2 -= 1;
                parts[i].bio.co2 += 1;
		}
    }

    
    if (BOUNDS_CHECK && (rx || ry))
    {
        r = pmap[y+ry][x+rx];
        if (r) {
			if (RNG::Ref().chance(1, 2)){
				// Diffuse among tumor
				if (TYP(r) == PT_TUMOR){
					int ir = ID(r);

					if (parts[i].bio.o2 > parts[ir].bio.o2){
						parts[i].bio.o2--;
						parts[ir].bio.o2++;
					}
					if (parts[i].bio.co2 > parts[ir].bio.co2){
						parts[i].bio.co2--;
						parts[ir].bio.co2++;
					}
				}
				// steal o2 from bio, offload co2 to bio (tumor is greedy)
				if (sim->elements[TYP(r)].Properties & TYPE_BIO){
					int ir = ID(r);
					
					if (parts[ir].bio.o2 > 1){
						parts[i].bio.o2++;
						parts[ir].bio.o2--;
					}
					if (parts[i].bio.co2 > 0){
						parts[i].bio.co2--;
						parts[ir].bio.co2++;
					}
				}
				if (TYP(r) == PT_BLD && RNG::Ref().chance(1, 1000)){
					int ir = ID(r);
					parts[ir].ctype = PT_TUMOR;
				}
			}
        }
    }
	if (RNG::Ref().chance(1, 100)){
		// convert biology to tumor (grow)
		if (sim->elements[TYP(r)].Properties & TYPE_BIO && TYP(r) != PT_TUMOR){
			int ir = ID(r);
			sim->part_change_type(ir, parts[ir].x, parts[ir].y, PT_TUMOR);
			parts[i].bio.o2--;
			parts[i].bio.co2++;
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
		if (parts[i].bio.co2 > MAX_CO2 || parts[i].bio.o2 < 1){
			parts[i].bio.health--;
		}
		// Otherwise heal
		else{
			// Tumors aren't the most healthy to start with.
                        if (parts[i].bio.health < 200){ 
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

	*colr = (int)fmax(3 * o, 77);
	*colg = (int)fmax(3 * o, 62);
	*colb = (int)fmax(3 * o, 62);
	*pixel_mode |= PMODE_BLUR;

	// Life mix
	*colr = int(*colr * (cpart->bio.health) / 100.0f);
	*colg = int(*colg * (cpart->bio.health) / 100.0f);
	*colb = int(*colb * (cpart->bio.health) / 100.0f);

	return 0;
}
