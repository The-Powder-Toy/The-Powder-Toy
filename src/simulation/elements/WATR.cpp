#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_WATR PT_WATR 2
Element_WATR::Element_WATR()
{
	Identifier = "DEFAULT_PT_WATR";
	Name = "WATR";
	Colour = PIXPACK(0x2030D0);
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
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 30;

	Temperature = R_TEMP-2.0f	+273.15f;
	HeatConduct = 29;
	Description = "Water. Conducts electricity, freezes, and extinguishes fires.";

	Properties = TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 273.15f;
	LowTemperatureTransition = PT_ICEI;
	HighTemperature = 373.0f;
	HighTemperatureTransition = PT_WTRV;

	Update = &Element_WATR::update;
}

//#TPT-Directive ElementHeader Element_WATR static int update(UPDATE_FUNC_ARGS)
int Element_WATR::update(UPDATE_FUNC_ARGS)
{
	
	if ((parts[i].dcolour & 0xFF000000) == 0xFD000000)
	{
		static int pcol, pcolr, pcolg, pcolb;
		pcolr = ((parts[i].dcolour >> 16) & 0xFF);
		pcolg = ((parts[i].dcolour >> 8) & 0xFF);
		pcolb = ((parts[i].dcolour) & 0xFF);
		pcolr -= 20;
		pcolg -= 20;
		pcolb -= 20;

		if (pcolr < 0) { pcolr = 0; }
		if (pcolg < 0) { pcolg = 0; }
		if (pcolb < 0) { pcolb = 0; }

		parts[i].dcolour = (0xFF000000) | (pcolr << 16) | (pcolg << 8) | (pcolb);
	}
	
	
	
	float color;
	int rx, ry, r, rndstore;
	unsigned long col, cola, col1, col2, lop, lop2, tem, tem2, tem3, tem4;
	static int checkCoordsX[] = { -4, 4, 0, 0 };
	static int checkCoordsY[] = { 0, 0, -4, 4 };
	//Find nearby rusted iron (BMTL with tmp 1+)
	for (int j = 0; j < 8; j++){
		rndstore = rand();
		rx = (rndstore % 9) - 4;
		rndstore >>= 4;
		ry = (rndstore % 9) - 4;
		if ((!rx != !ry) && BOUNDS_CHECK) {
			r = pmap[y + ry][x + rx];
			if (!r) continue;
			if ((r & 0xFF) == PT_WATR) {
				col = parts[i].dcolour;
				if (col & 0xFF000000 != 0xFF000000)
				{
					col = 0xFF2030D0;
					parts[r >> 8].dcolour = col;
					parts[i].dcolour = col;

				}
					
				col = 0;
						col1 = (parts[r >> 8].dcolour);
						col2 = (parts[i].dcolour);

						for (lop = 0; lop < 3; lop++)
						{

							tem = (col1 & 0x000000FF);
							tem2 = (col2 & 0x000000FF);
							color = ((tem + tem2) / 2.0f);
							int tem3 = int(color + 0.5);
							tem3 = (tem3 << (lop * 8));
							col = (col | tem3);
							col1 = (col1 >> 8);
							col2 = (col2 >> 8);

						}

						col = (col | 0xFF000000);
						parts[r >> 8].dcolour = col;
						parts[i].dcolour = col;

					
				}
			}
		}
	


	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_SALT && !(rand()%50))
				{
					sim->part_change_type(i,x,y,PT_SLTW);
					// on average, convert 3 WATR to SLTW before SALT turns into SLTW
					if (!(rand()%3))
						sim->part_change_type(r>>8,x+rx,y+ry,PT_SLTW);
				}
				else if (((r&0xFF)==PT_RBDM||(r&0xFF)==PT_LRBD) && (sim->legacy_enable||parts[i].temp>(273.15f+12.0f)) && !(rand()%100))
				{
					sim->part_change_type(i,x,y,PT_FIRE);
					parts[i].life = 4;
					parts[i].ctype = PT_WATR;
				}
				else if ((r&0xFF)==PT_FIRE && parts[r>>8].ctype!=PT_WATR){
					sim->kill_part(r>>8);
					if(!(rand()%30)){
						sim->kill_part(i);
						return 1;
					}
				}
				else if ((r&0xFF)==PT_SLTW && !(rand()%2000))
				{
					sim->part_change_type(i,x,y,PT_SLTW);
				}
				/*if ((r&0xFF)==PT_CNCT && !(rand()%100))	Concrete+Water to paste, not very popular
				 {
				 part_change_type(i,x,y,PT_PSTE);
				 sim.kill_part(r>>8);
				 }*/
			}
	return 0;
}

Element_WATR::~Element_WATR() {}
