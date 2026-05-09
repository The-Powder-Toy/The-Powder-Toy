#include "simulation/ElementCommon.h"
#include "STKM.h"

static int update(UPDATE_FUNC_ARGS);
static bool createAllowed(ELEMENT_CREATE_ALLOWED_FUNC_ARGS);
static void changeType(ELEMENT_CHANGETYPE_FUNC_ARGS);
static void Free(Simulation *sim, unsigned char i);

void Element::Element_FIGH()
{
	Identifier = "DEFAULT_PT_FIGH";
	Name = "FIGH";
	Colour = 0xFFE0A0_rgb;
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.5f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.2f;
	Loss = 1.0f;
	Collision = 0.0f;
	Gravity = 0.0f;
	NewtonianGravity = 0.0f;
	Diffusion = 0.0f;
	HotAir = 0.00f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 50;

	DefaultProperties.temp = R_TEMP + 14.6f + 273.15f;
	HeatConduct = 0;
	Description = "Fighter. Tries to kill stickmen. You must first give it an element to kill him with.";

	Properties = PROP_NOCTYPEDRAW;
	CarriesTypeIn = 1U << FIELD_CTYPE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 620.0f;
	HighTemperatureTransition = PT_FIRE;

	DefaultProperties.life = 100;

	Update = &update;
	Graphics = &Element_STKM_graphics;
	CreateAllowed = &createAllowed;
	ChangeType = &changeType;
}

// .tmp = id into fighters[] array

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (parts[i].tmp < 0 || parts[i].tmp >= MAX_FIGHTERS)
	{
		sim->kill_part(i);
		return 1;
	}
	fighterst* figh = &sim->fighters[(unsigned char)parts[i].tmp];

	// We will measure distance not between FIGH's and STKM's heads,
	// but between their knees' midpoints, because STKM's ID isn't available.
	Vec2<float> midLegs = (figh->legs[0] + figh->legs[2])*0.5f;
	Vec2<float> target = midLegs;

	float dist = XRES + YRES; // just an upper bound
	Vec2<float> distVec{0, 0};
	bool anyStkm = false;

	// Determine our target STKM if it exists
	if (sim->player.spwn && sim->player.elem != PT_INVIS)
	{
		auto midLegsStkm = (sim->player.legs[0] + sim->player.legs[2])*0.5f;

		dist = hypot(midLegs.X - midLegsStkm.X, midLegs.Y - midLegsStkm.Y);
		distVec = midLegsStkm - midLegs;
		target = midLegsStkm;
		anyStkm = true;
	}

	if (sim->player2.spwn && sim->player2.elem != PT_INVIS)
	{
		auto midLegsStkm = (sim->player2.legs[0] + sim->player2.legs[2])*0.5f;
		float dist2 = hypot(midLegs.X - midLegsStkm.X, midLegs.Y - midLegsStkm.Y);

		if (!anyStkm || dist2 < dist)
		{
			dist = dist2;
			distVec = midLegsStkm - midLegs;
			target = midLegsStkm;
			anyStkm = true;
		}
	}

	// Gravity is needed to correctly determine if STKM is to the left or to the right of FIGH
	Vec2<float> grav{0, 1};
	sim->GetGravityField(parts[i].x, parts[i].y, 1.0f, 1.0f, grav.X, grav.Y);
	float gravLen = hypot(grav.X, grav.Y);
	if (gravLen > 0.01f)
		grav = grav * (1/gravLen);
	else
		grav = Vec2<float>{0, 1};

	// Since grav is normalized, these are projections
	float horizSign = grav.X*distVec.Y - grav.Y*distVec.X; // > 0 means "left", < 0 means "right"
	float vertSign = grav.X*distVec.X + grav.Y*distVec.Y; // > 0 means "down", < 0 means "up"

	// Check if we have a "dangerous" element.
	// We don't allow energy particles since these shoot in a "laser beam" fashion
	int elem = figh->elem;
	bool dangerous = elem == PT_LIGH
		|| ((elements[elem].Properties & (PROP_DEADLY | PROP_RADIOACTIVE)) && (elements[elem].Properties & (TYPE_GAS | TYPE_PART | TYPE_LIQUID)))
		|| (elements[elem].DefaultProperties.temp >= 373.15f || elements[elem].DefaultProperties.temp <= 173.15);

	float cutoff = 0;
	if (dangerous)
	{
		float loss = elements[elem].Loss;
		float g = elements[elem].Gravity;

		if (loss > 0.99f) loss = 0.95f;
		if (g < 0.01f) g = 1.0f;

		// A completely bogus formula inspired by physics
		cutoff = std::min(70.0f, std::min(4.0f/(1.0f-loss), sqrtf(400.0f/g)));

		// Set a default cutoff for things other than powders and liquids
		if (!(elements[elem].Properties & (TYPE_PART | TYPE_LIQUID)))
			cutoff = 25;

		// Shoot
		float spread = float(sim->rng.between(-5, 5));
		if (anyStkm && fabs(horizSign) < (spread + cutoff) && fabs(horizSign) > (spread + cutoff - 5.0f) &&
			       fabs(vertSign) < 10.0f && figh->pdir == -isign(horizSign))
			figh->comm = 8;
	}

	// This is minimal when dist = cutoff
	float score = fabs(dist - cutoff) + cutoff;

	// Do something if we got hurt.
	// The chance() statement will be always true when life <= 20
	if (parts[i].life < figh->prevLife && parts[i].life > 0 && sim->rng.chance(20, parts[i].life))
	{
		if (sim->rng.chance(1, 2))
		{
			figh->comm = 0;

			// Change direction randomly
			if (sim->rng.chance(4, 5))
				figh->comm = sim->rng.between(1, 2); // Left/right
			if (sim->rng.chance(1, 5))
				figh->comm |= 4; // Add jump
		}
		else
		{
			// Flip direction
			if (figh->pcomm & 1)
				figh->comm = 2;
			else if (figh->pcomm & 2)
				figh->comm = 1;
			else
				figh->comm = 4 + sim->rng.between(1, 2);
		}
	}
	else if (anyStkm) // Seek STKM
	{
		// Try to "hit" a stickman by jumping
		if (!dangerous && elem != PT_DCEL && dist <= 2.0f)
			figh->comm |= 4;

		// Compare rounded values to avoid jitter effects
		if (int(score) >= int(figh->prevScore))
		{
			// Increase a "stuck" counter
			figh->frustration++;

			if (figh->frustration > 10)
			{
				if (sim->rng.chance(2, 3))
				{
					// Move based on relative position
					figh->comm = horizSign > 0 ? 1 : 2;
					if (dist < 20.0f && vertSign < -10.0f)
						figh->comm |= 4;
				}
				else if (sim->rng.chance(2, 3))
					figh->comm |= 4; // Try jumping
				else
				{
					figh->comm = 0;

					// Change direction randomly
					if (sim->rng.chance(4, 5))
						figh->comm = sim->rng.between(0, 1); // Left/right
					if (sim->rng.chance(1, 5))
						figh->comm |= 4; // Add jump
				}

				figh->frustration = 0;
			}
		}
		else
			figh->frustration = 0;
	}
	else if (sim->rng.chance(1, 5))
		figh->comm = 0;

	// Save current score and life for future comparison
	figh->prevScore = score;
	figh->prevLife = parts[i].life;

	figh->pcomm = figh->comm;

	Element_STKM_run_stickman(figh, UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

static bool createAllowed(ELEMENT_CREATE_ALLOWED_FUNC_ARGS)
{
	return Element_FIGH_CanAlloc(sim);
}

static void changeType(ELEMENT_CHANGETYPE_FUNC_ARGS)
{
	if (to == PT_FIGH)
	{
		sim->parts[i].tmp = Element_FIGH_Alloc(sim);
		if (sim->parts[i].tmp >= 0)
			Element_FIGH_NewFighter(sim, sim->parts[i].tmp, i, PT_DUST);
	}
	else
	{
		Free(sim, (unsigned char)sim->parts[i].tmp);
	}
}

bool Element_FIGH_CanAlloc(Simulation *sim)
{
	return sim->fighcount < MAX_FIGHTERS;
}

int Element_FIGH_Alloc(Simulation *sim)
{
	if (sim->fighcount >= MAX_FIGHTERS)
		return -1;
	int i = 0;
	while (i < MAX_FIGHTERS && sim->fighters[i].spwn)
		i++;
	if (i < MAX_FIGHTERS)
	{
		sim->fighters[i].spwn = true;
		sim->fighters[i].elem = PT_DUST;
		sim->fighcount++;
		return i;
	}
	else return -1;
}

static void Free(Simulation *sim, unsigned char i)
{
	if (sim->fighters[i].spwn)
	{
		sim->fighters[i].spwn = false;
		sim->fighcount--;
	}
}

void Element_FIGH_NewFighter(Simulation *sim, int fighterID, int i, int elem)
{
	Element_STKM_init_legs(sim, &sim->fighters[fighterID], i);
	if (elem > 0 && elem < PT_NUM)
		sim->fighters[fighterID].elem = elem;
	sim->fighters[fighterID].spwn = true;

	sim->fighters[fighterID].frustration = 0;
	sim->fighters[fighterID].prevLife = 100;
	sim->fighters[fighterID].prevScore = 0;
}
