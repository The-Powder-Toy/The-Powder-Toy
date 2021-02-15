#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static bool createAllowed(ELEMENT_CREATE_ALLOWED_FUNC_ARGS);
static void changeType(ELEMENT_CHANGETYPE_FUNC_ARGS);
static void Free(Simulation *sim, unsigned char i);
bool Element_FIGH_CanAlloc(Simulation *sim);
int Element_FIGH_Alloc(Simulation *sim);
void Element_FIGH_NewFighter(Simulation *sim, int fighterID, int i, int elem);
int Element_STKM_graphics(GRAPHICS_FUNC_ARGS);
void Element_STKM_init_legs(Simulation * sim, playerst *playerp, int i);
int Element_STKM_run_stickman(playerst *playerp, UPDATE_FUNC_ARGS);

void Element::Element_FIGH()
{
	Identifier = "DEFAULT_PT_FIGH";
	Name = "FIGH";
	Colour = PIXPACK(0xFFE0A0);
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

static int update(UPDATE_FUNC_ARGS)
{
	if (parts[i].tmp < 0 || parts[i].tmp >= MAX_FIGHTERS)
	{
		sim->kill_part(i);
		return 1;
	}
	playerst* figh = &sim->fighters[(unsigned char)parts[i].tmp];

	int tarx, tary;

	parts[i].tmp2 = 0; //0 - stay in place, 1 - seek a stick man

	//Set target cords
	if (sim->player2.spwn)
	{
		if (sim->player.spwn && (pow((float)sim->player.legs[2]-x, 2) + pow((float)sim->player.legs[3]-y, 2))<=
		   (pow((float)sim->player2.legs[2]-x, 2) + pow((float)sim->player2.legs[3]-y, 2)))
		{
			tarx = (int)sim->player.legs[2];
			tary = (int)sim->player.legs[3];
		}
		else
		{
			tarx = (int)sim->player2.legs[2];
			tary = (int)sim->player2.legs[3];
		}
		parts[i].tmp2 = 1;
	}
	else if (sim->player.spwn)
	{
		tarx = (int)sim->player.legs[2];
		tary = (int)sim->player.legs[3];
		parts[i].tmp2 = 1;
	}

	switch (parts[i].tmp2)
	{
	case 1:
		if ((pow(float(tarx-x), 2) + pow(float(tary-y), 2))<600)
		{
			if (figh->elem == PT_LIGH || figh->elem == PT_NEUT
			    || sim->elements[figh->elem].Properties & (PROP_DEADLY | PROP_RADIOACTIVE)
			    || sim->elements[figh->elem].DefaultProperties.temp >= 323 || sim->elements[figh->elem].DefaultProperties.temp <= 243)
				figh->comm = (int)figh->comm | 0x08;
		}
		else if (tarx<x)
		{
			if(figh->rocketBoots || !(sim->eval_move(PT_FIGH, int(figh->legs[4])-10, int(figh->legs[5])+6, NULL)
			     && sim->eval_move(PT_FIGH, int(figh->legs[4])-10, int(figh->legs[5])+3, NULL)))
				figh->comm = 0x01;
			else
				figh->comm = 0x02;

			if (figh->rocketBoots)
			{
				if (tary<y)
					figh->comm = (int)figh->comm | 0x04;
			}
			else if (!sim->eval_move(PT_FIGH, int(figh->legs[4])-4, int(figh->legs[5])-1, NULL)
			    || !sim->eval_move(PT_FIGH, int(figh->legs[12])-4, int(figh->legs[13])-1, NULL)
			    || sim->eval_move(PT_FIGH, 2*int(figh->legs[4])-int(figh->legs[6]), int(figh->legs[5])+5, NULL))
				figh->comm = (int)figh->comm | 0x04;
		}
		else
		{
			if (figh->rocketBoots || !(sim->eval_move(PT_FIGH, int(figh->legs[12])+10, int(figh->legs[13])+6, NULL)
			      && sim->eval_move(PT_FIGH, int(figh->legs[12])+10, int(figh->legs[13])+3, NULL)))
				figh->comm = 0x02;
			else
				figh->comm = 0x01;

			if (figh->rocketBoots)
			{
				if (tary<y)
					figh->comm = (int)figh->comm | 0x04;
			}
			else if (!sim->eval_move(PT_FIGH, int(figh->legs[4])+4, int(figh->legs[5])-1, NULL)
			    || !sim->eval_move(PT_FIGH, int(figh->legs[4])+4, int(figh->legs[5])-1, NULL)
			    || sim->eval_move(PT_FIGH, 2*int(figh->legs[12])-int(figh->legs[14]), int(figh->legs[13])+5, NULL))
				figh->comm = (int)figh->comm | 0x04;
		}
		break;
	default:
		figh->comm = 0;
		break;
	}

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
	while (i < MAX_FIGHTERS && sim->fighters[i].spwn==1)
		i++;
	if (i < MAX_FIGHTERS)
	{
		sim->fighters[i].spwn = 1;
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
		sim->fighters[i].spwn = 0;
		sim->fighcount--;
	}
}

void Element_FIGH_NewFighter(Simulation *sim, int fighterID, int i, int elem)
{
	Element_STKM_init_legs(sim, &sim->fighters[fighterID], i);
	if (elem > 0 && elem < PT_NUM)
		sim->fighters[fighterID].elem = elem;
	sim->fighters[fighterID].spwn = 1;
}
