#include "simulation/Elements.h"
 Element_LSNS::Element_LSNS()
 {
	Identifier = "DEFAULT_PT_LSNS";
 	Name = "LSNS";
 	Colour = PIXPACK(0x336699);
 	MenuVisible = 1;
 	MenuSection = SC_SENSOR;
 	Enabled = 1;
 
 	Advection = 0.0f;
 	AirDrag = 0.00f * CFDS;
 	AirLoss = 0.96f;
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
 
 	Temperature = R_TEMP+0.0f	+273.15f;
 	HeatConduct = 0;
 	Description = "Life sensor. Creates a spark when the life is higher than the temp.";
 
 	Properties = TYPE_SOLID;
 
 	LowPressure = IPL;
 	LowPressureTransition = NT;
 	HighPressure = IPH;
 	HighPressureTransition = NT;
 	LowTemperature = ITL;
 	LowTemperatureTransition = NT;
 	HighTemperature = ITH;
 	HighTemperatureTransition = NT;
 
 	Update = &Element_LSNS::update;
 }
 int Element_LSNS::update(UPDATE_FUNC_ARGS)
   {
   int sensorLife=parts[i].temp;
   if(sensorLife<0)
   {
       sensorLife=0;
   }
 	int r,rx,ry,rt=parts[i].tmp2;
 	for (rx=-1; rx<2; rx++)
 		for (ry=-1; ry<2; ry++)
 			if (BOUNDS_CHECK && (rx || ry))
 			{
 				r = pmap[y+ry][x+rx];
 				if (!r)
 					continue;
                        rt=(r&0xFF);
 			if ((parts[r>>8].life) > sensorLife)
 			{
                            if(sim->elements[rt].Properties&PROP_CONDUCTS)
                            {
 				    sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
                            }
 			};
 			};
 	};
         return 0;
 }
 
 
 Element_LSNS::~Element_LSNS() {}
