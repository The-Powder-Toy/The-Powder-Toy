#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FIGH PT_FIGH 158
Element_FIGH::Element_FIGH()
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
	Diffusion = 0.0f;
	HotAir = 0.00f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 50;

	Temperature = R_TEMP+14.6f+273.15f;
	HeatConduct = 0;
	Description = "Fighter. Tries to kill stickmen. You must first give it an element to kill him with.";

	Properties = PROP_NOCTYPEDRAW | PROP_UNLIMSTACKING;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 620.0f;
	HighTemperatureTransition = PT_FIRE;

	Update = &Element_FIGH::update;
	Graphics = &Element_STKM::graphics;
}

//#TPT-Directive ElementHeader Element_FIGH static int update(UPDATE_FUNC_ARGS)
int Element_FIGH::update(UPDATE_FUNC_ARGS)
{
	if (parts[i].tmp < 0 || parts[i].tmp >= MAX_FIGHTERS)
	{
		sim->kill_part(i);
		return 1;
	}
	playerst* figh = &sim->fighters[(unsigned char)parts[i].tmp];
	playerst* parent_s = NULL;

	int tarx, tary, __parent;

	parts[i].tmp2 = 0;
		// tmp2:
		//   0 - stay in place
		//   1 - seek a stick man
		//   2 - from parent command
	
	__parent = figh->parentStickman;
	if (__parent < MAX_FIGHTERS)
		parent_s = &sim->fighters[__parent];
	else if (__parent == MAX_FIGHTERS)
		parent_s = &sim->player;
	else if (__parent == MAX_FIGHTERS + 1)
		parent_s = &sim->player2;
	
	//Set target cords
	if (__parent >= 0)
	{
		switch ((sim->E189_FIGH_pause >> 6) & 3)
		{
		case 1:
			// using parent's command
			parts[i].tmp2 = 2;
			goto FIGH_break1;
		case 2:
			if (__parent >= MAX_FIGHTERS)
				goto FIGH_break1;
		case 3:
			// seek parent stick man
			tarx = (int)(parent_s->legs[2]);
			tary = (int)(parent_s->legs[3]);
			parts[i].tmp2 = 1;
			goto FIGH_break1;
		}
	}
	if (!(sim->E189_FIGH_pause & 1))
	{
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
	}
	FIGH_break1:

	switch (parts[i].tmp2)
	{
	case 1:
		if ((pow(float(tarx-x), 2) + pow(float(tary-y), 2))<600)
		{
			if (!(sim->E189_FIGH_pause & 2) && ((sim->E189_FIGH_pause & 4)
				|| figh->elem == PT_LIGH || figh->elem == PT_NEUT 
			    || sim->elements[figh->elem].Properties&(PROP_DEADLY|PROP_RADIOACTIVE)
			    || sim->elements[figh->elem].Temperature>=323 || sim->elements[figh->elem].Temperature<=243))
				figh->comm = (int)figh->comm | 0x08;
			if (figh->elem == PT_FIGH && (sim->E189_FIGH_pause & 0x0F) == 0x0E)
				figh->comm = (int)figh->comm | 0x08;
		}
		else if (tarx<x)
		{
			if(figh->rocketBoots || !(sim->eval_move(PT_FIGH, figh->legs[4]-10, figh->legs[5]+6, NULL)
			     && sim->eval_move(PT_FIGH, figh->legs[4]-10, figh->legs[5]+3, NULL)))
				figh->comm = 0x01;
			else
				figh->comm = 0x02;

			if (figh->rocketBoots)
			{
				if (tary<y)
					figh->comm = (int)figh->comm | 0x04;
			}
			else if (!sim->eval_move(PT_FIGH, figh->legs[4]-4, figh->legs[5]-1, NULL)
			    || !sim->eval_move(PT_FIGH, figh->legs[12]-4, figh->legs[13]-1, NULL)
			    || sim->eval_move(PT_FIGH, 2*figh->legs[4]-figh->legs[6], figh->legs[5]+5, NULL))
				figh->comm = (int)figh->comm | 0x04;
		}
		else
		{ 
			if (figh->rocketBoots || !(sim->eval_move(PT_FIGH, figh->legs[12]+10, figh->legs[13]+6, NULL)
			      && sim->eval_move(PT_FIGH, figh->legs[12]+10, figh->legs[13]+3, NULL)))
				figh->comm = 0x02;
			else
				figh->comm = 0x01;

			if (figh->rocketBoots)
			{
				if (tary<y)
					figh->comm = (int)figh->comm | 0x04;
			}
			else if (!sim->eval_move(PT_FIGH, figh->legs[4]+4, figh->legs[5]-1, NULL)
			    || !sim->eval_move(PT_FIGH, figh->legs[4]+4, figh->legs[5]-1, NULL)
			    || sim->eval_move(PT_FIGH, 2*figh->legs[12]-figh->legs[14], figh->legs[13]+5, NULL))
				figh->comm = (int)figh->comm | 0x04;
		}
		figh->pcomm = figh->comm;
		break;
	case 2:
		figh->comm = parent_s->comm;
		figh->pcomm = parent_s->pcomm;
		break;
	default:
		figh->comm = 0;
		figh->pcomm = 0;
		break;
	}


	Element_STKM::run_stickman(figh, UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

//#TPT-Directive ElementHeader Element_FIGH static void removeFIGHNode(Simulation *sim, int i)
void Element_FIGH::removeFIGHNode(Simulation *sim, int i)
{
	int prev_f, next_f, parent_f;
	unsigned char tmp;

	playerst* parent_s = NULL;

	tmp = (unsigned char)(sim->parts[i].tmp);
	prev_f = sim->fighters[tmp].prevStickman;
	next_f = sim->fighters[tmp].nextStickman;
	parent_f = sim->fighters[tmp].parentStickman;
	
	if (parent_f >= 0 && parent_f < MAX_FIGHTERS)
		parent_s = &sim->fighters[(unsigned char)parent_f];
	else if (parent_f == MAX_FIGHTERS)
		parent_s = &sim->player;
	else if (parent_f == MAX_FIGHTERS + 1)
		parent_s = &sim->player2;
	
	if (prev_f >= 0) // if previous (non-first) fighter is exist
		sim->fighters[(unsigned char)prev_f].nextStickman = next_f;
	else if (parent_s != NULL)
	{
		parent_s->firstChild = next_f;
	}

	if (next_f >= 0) // if next (non-last) fighter is exist
		sim->fighters[(unsigned char)next_f].prevStickman = prev_f;
	else if (parent_s != NULL)
	{
		parent_s->lastChild = prev_f;
	}
	
	Element_STKM::removeSTKMChilds(sim, &sim->fighters[tmp]);
}

Element_FIGH::~Element_FIGH() {}
