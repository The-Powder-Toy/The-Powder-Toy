#include "simulation/ElementCommon.h"
#include "FILT.h"

static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_FILT()
{
	Identifier = "DEFAULT_PT_FILT";
	Name = "FILT";
	Colour = 0x000056_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
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

	HeatConduct = 251;
	Description = "Filter for photons, changes the color.";

	Properties = TYPE_SOLID | PROP_PHOTPASS | PROP_NOAMBHEAT | PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Graphics = &graphics;
	Create = &create;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int x, wl = Element_FILT_getWavelengths(cpart);
	*colg = 0;
	*colb = 0;
	*colr = 0;
	for (x=0; x<12; x++) {
		*colr += (wl >> (x+18)) & 1;
		*colb += (wl >>  x)     & 1;
	}
	for (x=0; x<12; x++)
		*colg += (wl >> (x+9))  & 1;
	x = 624/(*colr+*colg+*colb+1);
	if (cpart->life>0 && cpart->life<=4)
		*cola = 127+cpart->life*30;
	else
		*cola = 127;
	*colr *= x;
	*colg *= x;
	*colb *= x;
	*pixel_mode &= ~PMODE;
	*pixel_mode |= PMODE_BLEND;
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp = v;
}

// Returns the wavelengths in a particle after FILT interacts with it (e.g. a photon)
// cpart is the FILT particle, origWl the original wavelengths in the interacting particle
int Element_FILT_interactWavelengths(Simulation *sim, Particle* cpart, int origWl)
{
	const int mask = 0x3FFFFFFF;
	int filtWl = Element_FILT_getWavelengths(cpart);
	switch (cpart->tmp)
	{
		case 0:
			return filtWl; //Assign Colour
		case 1:
			return origWl & filtWl; //Filter Colour
		case 2:
			return origWl | filtWl; //Add Colour
		case 3:
			return origWl & (~filtWl); //Subtract colour of filt from colour of photon
		case 4:
		{
			int shift = int((cpart->temp-273.0f)*0.025f);
			if (shift<=0) shift = 1;
			return (origWl << shift) & mask; // red shift
		}
		case 5:
		{
			int shift = int((cpart->temp-273.0f)*0.025f);
			if (shift<=0) shift = 1;
			return (origWl >> shift) & mask; // blue shift
		}
		case 6:
			return origWl; // No change
		case 7:
			return origWl ^ filtWl; // XOR colours
		case 8:
			return (~origWl) & mask; // Invert colours
		case 9:
		{
			int t1 = (origWl & 0x0000FF) + sim->rng.between(-2, 2);
			int t2 = ((origWl & 0x00FF00)>>8) + sim->rng.between(-2, 2);
			int t3 = ((origWl & 0xFF0000)>>16) + sim->rng.between(-2, 2);
			return (origWl & 0xFF000000) | (t3<<16) | (t2<<8) | t1;
		}
		case 10:
		{
			long long int lsb = filtWl & (-filtWl);
			return (origWl * lsb) & 0x3FFFFFFF; //red shift
		}
		case 11:
		{
			long long int lsb = filtWl & (-filtWl);
			return (origWl / lsb) & 0x3FFFFFFF; // blue shift
		}
		default:
			return filtWl;
	}
}

int Element_FILT_getWavelengths(const Particle* cpart)
{
	if (cpart->ctype&0x3FFFFFFF)
	{
		return cpart->ctype;
	}
	else
	{
		int temp_bin = (int)((cpart->temp-273.0f)*0.025f);
		if (temp_bin < 0) temp_bin = 0;
		if (temp_bin > 25) temp_bin = 25;
		return (0x1F << temp_bin);
	}
}

int colourToWavelength(int cr, int cg, int cb)
{
	float vl = std::max({cr, cg, cb});
	if (vl == 0.0f)
		return 0;
	int mt = 5;
	int best = 1000;
	int bestmt = mt;
	int vr, vg, vb;
	for (; mt < 13; mt++)
	{
		vr = (int)(cr / vl * mt + 0.5f);
		vg = (int)(cg / vl * mt + 0.5f);
		vb = (int)(cb / vl * mt + 0.5f);
		if ((mt < 7 || vr + vb >= mt - 6) && (mt < 10 || vg >= std::max(vr - 9, 0) + std::max(vb - 9, 0)))
		{
			int diff = std::abs(cr - vr * vl / mt) + std::abs(cg - vg * vl / mt) + std::abs(cb - vb * vl / mt);
			if (diff <= best)
			{
				best = diff;
				bestmt = mt;
			}
		}
	}
	mt = bestmt;
	vr = (int)(cr / vl * mt + 0.5f);
	vg = (int)(cg / vl * mt + 0.5f);
	vb = (int)(cb / vl * mt + 0.5f);
	int shg = 0;
	if (vg > 6)
	{
		shg = std::max(std::min({vr - vb, vg - 6, 3}), 6 - vg, -3);
		vr -= std::max(shg, 0);
		vb += std::min(shg, 0);
	}
	else
	{
		if (vb > 9)
			vg -= vb - 9;
		if (vr > 9)
			vg -= vr - 9;
	}
	unsigned int mask = ((1 << vr) - 1) << (30 - vr);
	mask |= ((1 << vg) - 1) << (12 + shg);
	mask |= ((1 << vb) - 1);
	return mask &= 0x3FFFFFFF;
}

RGB<uint8_t> wavelengthToColour(int wavelength)
{
	int x, colr, colg, colb;
	for (colr = colg = colb = x = 0; x<12; x++) {
		colr += (wavelength >> (x+18)) & 1;
		colg += (wavelength >> (x+9))  & 1;
		colb += (wavelength >>  x)	    & 1;
	}
	double xl = 255.0 / std::max({colr,colg,colb});
	colr *= xl;
	colg *= xl;
	colb *= xl;
	return RGB<uint8_t>(colr, colg, colb);
}
