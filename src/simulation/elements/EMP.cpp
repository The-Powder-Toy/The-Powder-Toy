#include "simulation/Elements.h"
#include "Probability.h"

//#TPT-Directive ElementClass Element_EMP PT_EMP 134
Element_EMP::Element_EMP()
{
	Identifier = "DEFAULT_PT_EMP";
	Name = "EMP";
	Colour = PIXPACK(0x66AAFF);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.0f;
	HotAir = 0.0f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 3;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 121;
	Description = "Electromagnetic pulse. Breaks activated electronics.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = NULL;
	Graphics = &Element_EMP::graphics;
}

class DeltaTempGenerator
{
protected:
	float stepSize;
	unsigned int maxStepCount;
	Probability::SmallKBinomialGenerator binom;
public:
	DeltaTempGenerator(int n, float p, float tempStep) :
		stepSize(tempStep),
		// hardcoded limit of 10, to avoid massive lag if someone adds a few zeroes to MAX_TEMP
		maxStepCount((MAX_TEMP/stepSize < 10) ? ((unsigned int)(MAX_TEMP/stepSize)+1) : 10),
		binom(n, p, maxStepCount)
	{}
	float getDelta(float randFloat)
	{
		// randFloat should be a random float between 0 and 1
		return binom.calc(randFloat) * stepSize;
	}
	void apply(Simulation *sim, Particle &p)
	{
		p.temp = restrict_flt(p.temp+getDelta(Probability::randFloat()), MIN_TEMP, MAX_TEMP);
	}
};

//#TPT-Directive ElementHeader Element_EMP static int Trigger(Simulation *sim, int triggerCount)
int Element_EMP::Trigger(Simulation *sim, int triggerCount)
{
	/* Known differences from original one-particle-at-a-time version:
	 * - SPRK that disappears during a frame (such as SPRK with life==0 on that frame) will not cause destruction around it.
	 * - SPRK neighbour effects are calculated assuming the SPRK exists and causes destruction around it for the entire frame (so was not turned into BREL/NTCT partway through). This means mass EMP will be more destructive.
	 * - The chance of a METL particle near sparked semiconductor turning into BRMT within 1 frame is different if triggerCount>2. See comment for prob_breakMETLMore.
	 * - Probability of centre isElec particle breaking is slightly different (1/48 instead of 1-(1-1/80)*(1-1/120) = just under 1/48).
	 */

	Particle *parts = sim->parts;

	float prob_changeCenter = Probability::binomial_gte1(triggerCount, 1.0f/48);
	DeltaTempGenerator temp_center(triggerCount, 1.0f/100, 3000.0f);

	float prob_breakMETL = Probability::binomial_gte1(triggerCount, 1.0f/300);
	float prob_breakBMTL = Probability::binomial_gte1(triggerCount, 1.0f/160);
	DeltaTempGenerator temp_metal(triggerCount, 1.0f/280, 3000.0f);
	/* Probability of breaking from BMTL to BRMT, given that the particle has just broken from METL to BMTL. There is no mathematical reasoning for the numbers used, other than:
	 * - larger triggerCount should make this more likely, so it should depend on triggerCount instead of being a constant probability
	 * - triggerCount==1 should make this a chance of 0 (matching previous behaviour)
	 * - triggerCount==2 should make this a chance of 1/160 (matching previous behaviour)
	 */
	// TODO: work out in a more mathematical way what this should be?
	float prob_breakMETLMore = Probability::binomial_gte1(triggerCount/2, 1.0f/160);

	float prob_randWIFI = Probability::binomial_gte1(triggerCount, 1.0f/8);
	float prob_breakWIFI = Probability::binomial_gte1(triggerCount, 1.0f/16);

	float prob_breakSWCH = Probability::binomial_gte1(triggerCount, 1.0f/100);
	DeltaTempGenerator temp_SWCH(triggerCount, 1.0f/100, 2000.0f);

	float prob_breakARAY = Probability::binomial_gte1(triggerCount, 1.0f/60);

	float prob_randDLAY = Probability::binomial_gte1(triggerCount, 1.0f/70);

	for (int r = 0; r <=sim->parts_lastActiveIndex; r++)
	{
		int t = parts[r].type;
		int rx = parts[r].x;
		int ry = parts[r].y;
		if (t==PT_SPRK || (t==PT_SWCH && parts[r].life!=0 && parts[r].life!=10) || (t==PT_WIRE && parts[r].ctype>0))
		{
			bool is_elec = false;
			if (parts[r].ctype==PT_PSCN || parts[r].ctype==PT_NSCN || parts[r].ctype==PT_PTCT ||
			    parts[r].ctype==PT_NTCT || parts[r].ctype==PT_INST || parts[r].ctype==PT_SWCH || t==PT_WIRE || t==PT_SWCH)
			{
				is_elec = true;
				temp_center.apply(sim, parts[r]);
				if (Probability::randFloat() < prob_changeCenter)
				{
					if (rand()%5 < 2)
						sim->part_change_type(r, rx, ry, PT_BREC);
					else
						sim->part_change_type(r, rx, ry, PT_NTCT);
				}
			}
			for (int nx =-2; nx <= 3; nx++)
				for (int ny =-2; ny <= 2; ny++)
					if (rx+nx>=0 && ry+ny>=0 && rx+nx<XRES && ry+ny<YRES && (rx || ry))
					{
						int n = sim->pmap[ry+ny][rx+nx];
						if (!n)
							continue;
						int ntype = TYP(n);
						n = ID(n);
						//Some elements should only be affected by wire/swch, or by a spark on inst/semiconductor
						//So not affected by spark on metl, watr etc
						if (is_elec)
						{
							switch (ntype)
							{
							case PT_METL:
								temp_metal.apply(sim, parts[n]);
								if (Probability::randFloat() < prob_breakMETL)
								{
									sim->part_change_type(n, rx+nx, ry+ny, PT_BMTL);
									if (Probability::randFloat() < prob_breakMETLMore)
									{
										sim->part_change_type(n, rx+nx, ry+ny, PT_BRMT);
										parts[n].temp = restrict_flt(parts[n].temp+1000.0f, MIN_TEMP, MAX_TEMP);
									}
								}
								break;
							case PT_BMTL:
								temp_metal.apply(sim, parts[n]);
								if (Probability::randFloat() < prob_breakBMTL)
								{
									sim->part_change_type(n, rx+nx, ry+ny, PT_BRMT);
									parts[n].temp = restrict_flt(parts[n].temp+1000.0f, MIN_TEMP, MAX_TEMP);
								}
								break;
							case PT_WIFI:
								if (Probability::randFloat() < prob_randWIFI)
								{
									// Randomize channel
									parts[n].temp = rand()%MAX_TEMP;
								}
								if (Probability::randFloat() < prob_breakWIFI)
								{
									sim->create_part(n, rx+nx, ry+ny, PT_BREC);
									parts[n].temp = restrict_flt(parts[n].temp+1000.0f, MIN_TEMP, MAX_TEMP);
								}
								continue;
							default:
								break;
							}
						}
						switch (ntype)
						{
						case PT_SWCH:
							if (Probability::randFloat() < prob_breakSWCH)
								sim->part_change_type(n, rx+nx, ry+ny, PT_BREC);
							temp_SWCH.apply(sim, parts[n]);
							break;
						case PT_ARAY:
							if (Probability::randFloat() < prob_breakARAY)
							{
								sim->create_part(n, rx+nx, ry+ny, PT_BREC);
								parts[n].temp = restrict_flt(parts[n].temp+1000.0f, MIN_TEMP, MAX_TEMP);
							}
							break;
						case PT_DLAY:
							if (Probability::randFloat() < prob_randDLAY)
							{
								// Randomize delay
								parts[n].temp = (rand()%256) + 273.15f;
							}
							break;
						default:
							break;
						}
					}
		}
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_EMP static int graphics(GRAPHICS_FUNC_ARGS)
int Element_EMP::graphics(GRAPHICS_FUNC_ARGS)

{
	if(cpart->life)
	{
		*colr = cpart->life*1.5;
		*colg = cpart->life*1.5;
		*colb = 200-(cpart->life);
	}
	return 0;
}


Element_EMP::~Element_EMP() {}
