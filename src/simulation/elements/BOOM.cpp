#include "simulation/ElementCommon.h"
#include "simulation/Simulation.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_BOOM()
{
	Identifier = "DEFAULT_PT_BOOM";
	Name = "BOOM";
	Colour = PIXPACK(0xFF3311);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = -0.05f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.95f;
	Collision = -0.1f;

	//I think this controls how much it is affected by gravity. If so, making it zero should do the trick.
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 101;

	HeatConduct = 150;
	Description = "More destructive Bomb, can break through virtually anything.";

	Properties = TYPE_ENERGY;

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
    int r, rx, ry, rt, nb, proton;

    int c = 0;

    sim->pv[y/CELL][x/CELL]-=256.0f;

	for (rx=-1; rx<=1; rx++) {
		for (ry=-1; ry<=1; ry++) {
            if (BOUNDS_CHECK && (rx || ry)) {
                int r = pmap[y+ry][x+rx];
                rt = TYP(r);

                if (!r || rt == PT_INDE2)
		            continue;

                if (rt == PT_VOID) {
                    int rx2, ry2;

                    for (rx2=-1; rx2<=1; rx2++) {
		                for (ry2=-1; ry2<=1; ry2++) {
                            sim->create_part(-1, x+rx2, y+ry2, PT_BOOM);
                        }
                    }
                }

                while (r && TYP(r) != PT_BOOM && c < 20) {
					sim->kill_part(ID(r));
					r = pmap[y + ry][x + rx];
					c++;
				}

                
                rx = RNG::Ref().between(-3, 3);
	            ry = RNG::Ref().between(-3, 3);

                sim->create_part(-1, x+rx, y+ry, PT_BOOM);
                

                sim->pv[y/CELL][x/CELL]+=10000.0f;

                parts[i].temp = 1000000.0f;
            }
        }
    }


    /*if (RNG::Ref().chance(1, 20)) {
        rx = RNG::Ref().between(-3, 3);
	    ry = RNG::Ref().between(-3, 3);

        sim->create_part(-1, x+rx, y+ry, PT_PLSM);
    }*/

    if (RNG::Ref().chance(1, 10)) {
        rx = RNG::Ref().between(-3, 3);
	    ry = RNG::Ref().between(-3, 3);

        proton = sim->create_part(-1, x+rx, y+ry, PT_PROT);
        
        if (proton > -1)
            sim->parts[proton].temp = MAX_TEMP;
    }
    

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= PMODE_FLARE;
	return 1;
}
