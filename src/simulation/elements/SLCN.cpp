#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_SLCN()
{
	Identifier = "DEFAULT_PT_SLCN";
	Name = "SLCN";
	Colour = PIXPACK(0xbccddf);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.27f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 90;

	HeatConduct = 3;
	Description = "Powdered Silicon. A key element in multiple materials.";

	Properties = TYPE_PART | PROP_CONDUCTS | PROP_HOT_GLOW | PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 3538.15f;
	HighTemperatureTransition = PT_LAVA;
	//PhotonReflectWavelengths = TODO;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

// This list provides the color wheel used for SLCN's sparkle effect.
// Colors sampled from a public domain picture of pure silicon.
// https://upload.wikimedia.org/wikipedia/commons/e/e9/SiliconCroda.jpg
static int SLCN_SPARKLE[16] = {
	PIXPACK(0x5a6679),
	PIXPACK(0x6878a1),
	PIXPACK(0xabbfdd),
	PIXPACK(0x838490),
	
	PIXPACK(0xbccddf),
	PIXPACK(0x82a0d2),
	PIXPACK(0x5b6680),
	PIXPACK(0x232c3b),

	PIXPACK(0x485067),
	PIXPACK(0x8b9ab6),
	PIXPACK(0xadb1c1),
	PIXPACK(0xc3c6d1),
	
	PIXPACK(0x8594ad),
	PIXPACK(0x262f47),
	PIXPACK(0xa9aebc),
	PIXPACK(0xc2e1f7),
};


static int SLCN_SPARKLE_FX[16] = {
	0,
	0,
	PMODE_FLARE | PMODE_GLOW,
	0,

	0,
	0,
	0,
	0,

	0,
	0,
	0,
	0,

	0,
	0,
	0,
	PMODE_SPARK,
};

float SPARKLE_RATE = 0.01f;
float VELOCITY_MULTIPLIER = 1.2f;
float PHASE_THRESHOLD = 60.0f;

static int update(UPDATE_FUNC_ARGS)
{
	// update code here
	Particle &self = parts[i];

	float velocity = std::sqrt(std::pow(self.vx, 2) + std::pow(self.vy, 2));

	//FROM: GOLD.cpp:74 but prettied up a bit.
	if (self.life == 0 && self.temp < 373.15f)
	{
		for (int j = 0; j < 4; j++)
		{ 
			int checkCoordsX[] = { -4, 4, 0, 0 };
			int checkCoordsY[] = { 0, 0, -4, 4 };

			int neighbour_x = checkCoordsX[j];
			int neighbour_y = checkCoordsY[j];

			int neighbour_id = pmap[y + neighbour_y][x + neighbour_x];

			if (neighbour_id == 0) continue;

			int neighbour_type = TYP(neighbour_id);
			Particle &neighbour = parts[ID(neighbour_id)];

			if (neighbour_type == PT_SPRK && neighbour.life != 0 && neighbour.life < 4)
			{
				sim->part_change_type(i, x, y, PT_SPRK);
				self.life = 4;
				self.ctype = PT_SLCN;
			}
		}
	}

	int gt_phase_count = 0;

	for (int neighbour_x=-2; neighbour_x<3; neighbour_x++)
		for (int neighbour_y=-2; neighbour_y<3; neighbour_y++) 
		{
			int neighbour_id = pmap[y + neighbour_y][x + neighbour_x];

			if (neighbour_id == 0) continue;

			int neighbour_type = TYP(neighbour_id);
			Particle &neighbour = parts[ID(neighbour_id)];
			if (neighbour_type = PT_SLCN)
			{
				if (self.tmp == neighbour.tmp)
				{
					gt_phase_count += 1;
				}
			}
		}

	// Phase change logic.
	int current_color_phase = self.tmp & 31; // limited to 0 through 31. using bitlogic for simplicity.
	int current_spark_phase = self.tmp & 15; // limited to 0 through 15.

	float &clr_phse_transition_cnt = self.pavg[0];
	float &sprk_phse_transition_cnt = self.pavg[1];

	clr_phse_transition_cnt += (velocity * VELOCITY_MULTIPLIER) + SPARKLE_RATE + (RNG::Ref().uniform01() * SPARKLE_RATE);
	self.pavg[1] += (velocity * VELOCITY_MULTIPLIER) + SPARKLE_RATE + (RNG::Ref().uniform01() * SPARKLE_RATE);

	// evil trick to combat syncronization.
	if (gt_phase_count > 3) {
		clr_phse_transition_cnt -= 0.2;
		if (RNG::Ref().chance(1, 30)) {
			self.tmp = RNG::Ref().between(0, 31);
			self.tmp2 = RNG::Ref().between(0, 15);
			clr_phse_transition_cnt = (float)RNG::Ref().between(0, (int)PHASE_THRESHOLD);
			sprk_phse_transition_cnt = (float)RNG::Ref().between(0, (int)PHASE_THRESHOLD);
		}
	}

	if (clr_phse_transition_cnt > PHASE_THRESHOLD) {
		clr_phse_transition_cnt -= PHASE_THRESHOLD;
		self.tmp = (current_color_phase + 1) & 31;
	}

	// check if currently in a FX phase, and if so, greatly accelerate phase change.
	if (SLCN_SPARKLE_FX[self.tmp2] != 0) {
		sprk_phse_transition_cnt += RNG::Ref().uniform01() * PHASE_THRESHOLD * 2; // good chance of skipping the phase.
	}

	if (self.pavg[1] > PHASE_THRESHOLD) {
		sprk_phse_transition_cnt -= PHASE_THRESHOLD;
		self.tmp2 = (current_spark_phase + 1) & 15;
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// graphics code here
	// return 1 if nothing dymanic happens here
	int phase = cpart->tmp & 31;

	int selected_color = 0xFF00FF; // Debug color. will be shown in case of error.
	if ((phase & 1) == 0) {
		selected_color = SLCN_SPARKLE[phase >> 1];
	} else {
		int color_a = SLCN_SPARKLE[phase >> 1];
		int color_b = SLCN_SPARKLE[((phase + 1) >> 1) & 15];
		selected_color = 
			PIXRGB(
				(PIXR(color_a) + PIXR(color_b)) / 2,
				(PIXG(color_a) + PIXG(color_b)) / 2,
				(PIXB(color_a) + PIXB(color_b)) / 2
			);
	}
	*colr = PIXR(selected_color);
	*colg = PIXG(selected_color);
	*colb = PIXB(selected_color);

	*pixel_mode |= SLCN_SPARKLE_FX[cpart->tmp2 & 15];

	return 0;
}


static void create(ELEMENT_CREATE_FUNC_ARGS) {
	sim->parts[i].tmp = RNG::Ref().between(0, 31);
	sim->parts[i].tmp2 = RNG::Ref().between(0, 15);
	sim->parts[i].pavg[0] = (float)RNG::Ref().between(0, (int)PHASE_THRESHOLD);
	sim->parts[i].pavg[1] = (float)RNG::Ref().between(0, (int)PHASE_THRESHOLD);
}