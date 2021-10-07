/*

#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_BACT()
{
	Identifier = "DEFAULT_PT_BACT";
	Name = "BACT";
	Colour = PIXPACK(0x990000);
	MenuVisible = 1;
	MenuSection = SC_BIO;
	Enabled = 0;

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
	DefaultProperties.bio.health = 500;
	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 29;
	Description = "Bacteria. Infects living things (genes stored in tmp) (WIP).";

	Properties = TYPE_LIQUID|PROP_NEUTPENETRATE|TYPE_BIO;

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

    // O2 use by blood itself (made very slow for somewhat accuracy)
    if (RNG::Ref().chance(1, 1000)){

		if (parts[i].bio.o2 > 0){
        	parts[i].bio.o2 -= 1;
			parts[i].bio.co2 += 1;
		}
    }

    
    if (BOUNDS_CHECK && (rx || ry))
    {
        r = pmap[y+ry][x+rx];

		int t = TYP(r);
		int ir = ID(r);

		if (r){
			// Oxygen collection
			if (parts[i].bio.o2 < 10 && t == PT_O2){
				parts[i].bio.o2 += 5;
				sim->part_change_type(ID(r), x, y, PT_NONE);
			}
			// Diffusion into surrounding blood
			/*
			else if (t == PT_BLD){
				if (parts[i].bio.o2 > parts[ir].bio.o2){
					parts[i].bio.o2--;
					parts[ir].bio.o2++;
				}
				if (parts[i].bio.co2 > parts[ir].bio.co2){
					parts[i].bio.co2--;
					parts[ir].bio.co2++;
				}
			}
			*\
			// Transfer to biological tissues
			
			else if (sim->elements[t].Properties & TYPE_BIO){
				// Give oxygen
				if (t != PT_LUNG && parts[i].bio.o2 > 0 && parts[ir].bio.o2 < MAX_O2){
					parts[i].bio.o2--;
					parts[ir].bio.o2++;
				}
				// Take co2
				if (parts[i].bio.co2 < MAX_CO2 && parts[ir].bio.co2 > 0){
					parts[i].bio.co2++;
					parts[ir].bio.co2--;
				}
				// Kill foreign biological objects (Immune system), weakened with damage.
				if (t == PT_TUMOR){
					// Real world immune system is bad at handling cancer
					if (RNG::Ref().chance(parts[i].bio.health, 1000000)){
						sim->kill_part(ir);
					}
				}
			}
		}
	}

	// Health management
	if (RNG::Ref().chance(1, 100)){

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
			if (parts[i].bio.health < 500 && parts[i].bio.o2 > 1){
				parts[i].bio.health++;
				parts[i].bio.o2--;
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

	int q = cpart->bio.o2;
	*colr = (int)fmax(9 * o, 75);
	*colg = 0;
	*colb = 8 * c;
	*pixel_mode |= PMODE_BLUR;

	//*colr = int(*colr * (cpart->bio.health) / 100.0f);
	//*colg = int(*colg * (cpart->bio.health) / 100.0f);
	//*colb = int(*colb * (cpart->bio.health) / 100.0f);

	return 0;
}

int evaluateGenes(int gene, int genome)
{
	if (gene == 1)
	{
		geneValue = genome % 256;
		return geneValue;
	}
}

int modifyGenes(int gene, int newValue, int genome)
{
	
}

*/
