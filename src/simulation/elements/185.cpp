#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_E185 PT_E185 185

/*
TODO: 
	- Molten version
	- "muted version" a powder created by mixing E185 with ? that is weaker
*/

Element_E185::Element_E185()
{
	Identifier = "DEFAULT_PT_E185";
	Name = "E185";
	Colour = PIXPACK(0x80C030);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
#if defined(DEBUG) || defined(SNAPSHOT)
	Enabled = 1;
#else
	Enabled = 0;
#endif

	Advection = 0.4f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.4f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 90;

	Temperature = R_TEMP+4.0f	+273.15f;
	HeatConduct = 251;
	Description = "Experimental element. Some kind of nuclear fuel, or replicating powder";

	Properties = TYPE_PART|PROP_NEUTPASS|PROP_RADIOACTIVE|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_E185::update;
	Graphics = &Element_E185::graphics;
}

//#TPT-Directive ElementHeader Element_E185 static int update(UPDATE_FUNC_ARGS)
int Element_E185::update(UPDATE_FUNC_ARGS)
{
	int r, s, rx, ry, rr, sctype, stmp, trade, exot_id, exot_pos_x, exot_pos_y, prev_type = 0;
	int rrx, rry, rrr;
	const int cooldown = 15;
	const int limit = 20;
	float tempTemp, tempPress;
	rr = sim->photons[y][x];
	stmp = parts[i].tmp;
	if (parts[i].tmp2 & 1)
	{
		tempTemp = parts[i].temp;
		if (parts[i].life <= 1) {
			for (s = parts[i].tmp; s > 0; s--)
			{
				rr = sim->create_part(-1, x + rand()%7-3, y + rand()%7-3, PT_E185);
				if (rr >= 0)
				{
					parts[rr].temp = tempTemp;
					parts[rr].tmp  = rand() % 5 + 2;
					parts[rr].tmp2 = 1;
					parts[rr].life = rand() % 16 + 3;
				}
			}
			sim->kill_part(i);
		}
	}
	else
	{
		if (stmp < limit && !parts[i].life)
		{
			sctype = parts[i].ctype & 0xFF; // don't create SPC_AIR
			if (!(rand()%140) && !(rand()%100) && !stmp)
			{
				if (!sctype)
					s = sim->create_part(-3, x, y, PT_ELEC);
				else if (sim->elements[sctype].Properties & TYPE_ENERGY)
					s = sim->create_part(-3, x, y, sctype);
				else
				{
					rx = rand() % 5 - 2;
					ry = rand() % 5 - 2;
					if (sim->IsWallBlocking(x+rx, y+ry, sctype))
						s = sim->create_part(-3, x, y, sctype);
				}
				if (s >= 0)
				{
					parts[i].life = cooldown;
					parts[i].tmp = 1;
					parts[i].temp += 10;
					parts[s].temp = parts[i].temp;
					if (sctype == PT_GRVT)
						parts[s].tmp = 0;
					else if (sctype == PT_WARP)
						parts[s].tmp2 = 3000 + rand() % 10000;
				}
			}
			if (rr && (rr & 0xFF) != PT_NEUT && !(rand()%80) && ((stmp - 11) < rand() % 10))
			{
				if (rand() % 10)
				{
					if (!sctype)
						s = sim->create_part(-3, x, y, PT_ELEC);
					else if (sim->elements[sctype].Properties & TYPE_ENERGY)
						s = sim->create_part(-3, x, y, sctype);
				}
				else
					s = sim->create_part(-3, x, y, PT_E186);
				parts[i].life = cooldown;
				parts[i].tmp ++;
				parts[i].temp += (stmp >= 10) ? (stmp - 8) * 10 : 10;

				parts[rr>>8].temp = parts[i].temp;
				if (s >= 0)
				{
					parts[s].ctype = sctype;
					parts[s].temp = parts[i].temp;
					if (sctype == PT_GRVT)
						parts[s].tmp = 0;
				}
			}
		}
		for (trade = 0; trade < 6; trade ++)
		{
			rx = rand()%5-2; ry = rand()%5-2;
			r = pmap[y+ry][x+rx];
			exot_pos_x = x + rx;
			exot_pos_y = y + ry;
			exot_id = r >> 8;
			if ((r & 0xFF) == PT_EXOT)
			{
				goto E185_HasExot;
			}
			switch (r & 0xFF)
			{
			case PT_CO2:
				prev_type = PT_CO2;
				break;
			case PT_YEST:
				if (stmp >= 16)
				{
					if (!(rand() % 10))
					{
						if (rand() % 100)
						{
							sim->part_change_type(i, x, y, PT_YEST);
							parts[i].temp = parts[exot_id].temp;
						}
						else
						{
							sim->part_change_type(exot_id, exot_pos_x, exot_pos_y, PT_E185); // not E189
							parts[exot_id].tmp = 5;
							parts[exot_id].tmp2 = 1;
						}
					}
				}
				else
				{
					sim->part_change_type(exot_id, exot_pos_x, exot_pos_y, PT_DYST);
				}
			case PT_LAVA:
				switch (parts[r>>8].ctype)
				{
					case PT_SWCH:
						for (trade = 0; trade < 6; trade ++)
						{
							rx = rand()%5-2; ry = rand()%5-2; rrr = pmap[y+ry][x+rx];
							if ((rrr & 0xFF) == PT_LAVA && parts[rrr>>8].ctype == PT_BMTL)
							{
								prev_type = PT_BMTL;
								parts[rrr>>8].ctype = PT_WIFI;
							}
						}
						if (prev_type == PT_BMTL)
							parts[r>>8].ctype = PT_WIFI;
					break;
					case PT_WIFI:
						tempPress = sim->pv[y/CELL][x/CELL];
						if (tempPress >= 30)
							parts[r>>8].ctype = PT_PRTO;
						else if (tempPress <= -30)
							parts[r>>8].ctype = PT_PRTI;
					break;
				}
			}
		}
		if (prev_type == PT_CO2)
		{
			for (trade = 0; trade < 6; trade ++)
			{
				rx = rand()%5-2; ry = rand()%5-2;
				r = pmap[y+ry][x+rx];
				switch (r & 0xFF)
				{
				case PT_SALT:
					sim->part_change_type(exot_id, exot_pos_x, exot_pos_y, PT_SOAP);
					sim->part_change_type(r >> 8, x + rx, y + ry, PT_SOAP);
					goto E185_eol_1;
				case PT_SOAP:
					if (sim->pv[y/CELL][x/CELL] >= 10)
					{
						sim->part_change_type(exot_id, exot_pos_x, exot_pos_y, PT_SPNG);
						sim->part_change_type(r >> 8, x + rx, y + ry, PT_SPNG);
					}
					goto E185_eol_1;
				case PT_CAUS:
					sim->part_change_type(r >> 8, x + rx, y + ry, PT_BOYL);
					break;
				}
			}
			E185_eol_1:
			;
		}
		else if ((rr & 0xFF) == PT_NEUT && !(rand()%10))
		{
			s = parts[i].tmp;
			parts[i].tmp -= s > 0 ? (s >> 3) + 1 : 0;
		}
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r & 0xFF) == PT_E182 && !(rand()%40))
					{
						if (rand()%4)
							parts[i].tmp = 0;
						parts[r>>8].tmp = 0;
					}
				}
		return 0;
		E185_HasExot:
		for (trade = 0; trade < 6; trade ++)
		{
			rx = rand()%5-2; ry = rand()%5-2;
			r = pmap[y+ry][x+rx];
			switch (r & 0xFF)
			{
			case PT_DUST:
				sim->part_change_type(r>>8, x+rx, y+ry, PT_FWRK);
				break;
			case PT_LAVA:
				stmp = parts[r>>8].ctype;
				switch (stmp) // LAVA's ctype
				{
				case PT_CLST:
					parts[r>>8].ctype = PT_CNCT;
					break;
				case PT_GLAS:
					switch (rand() % 3)
					{
						case 0: parts[r>>8].ctype = PT_QRTZ; break;
						case 1: parts[r>>8].ctype = PT_FILT; break;
						case 2: parts[r>>8].ctype = PT_LCRY; break;
					}
					break;
				case PT_SALT:
					sim->create_part(r>>8, x+rx, y+ry, PT_ACID);
					break;
				case 0: // actual is PT_NONE
				case PT_STNE:
				case PT_PLUT:
					parts[r>>8].ctype = PT_BRCK;
					break;
				}
				break;
			case PT_BCOL:
				sim->create_part(r>>8, x+rx, y+ry, PT_LAVA);
				parts[r>>8].ctype = PT_DMND;
				break;
			// case PT_EXOT: 2 exotic matters and E185
			case PT_EMP:
				if (!(rand() % 1000) && parts[i].temp >= 2000)
				{
					sim->create_part(r>>8, x+rx, y+ry, PT_E189, 8);
					parts[r>>8].tmp = 21000;
				}
				return 0;
			case PT_ETRD:
				tempPress = sim->pv[y/CELL][x/CELL];
				if (!(rand() % 10) && parts[i].temp >= 7000 && tempPress < -10.0f && tempPress > -20.0f)
				{
					sim->part_change_type(r>>8, x+rx, y+ry, PT_LAVA);
					parts[r>>8].ctype = PT_CONV;
				}
				return 0;
			case PT_GAS:
			case PT_OIL:
				sim->create_part(r>>8, x+rx, y+ry, PT_NITR);
				break;
			case PT_GEL:
				sim->part_change_type(r>>8, x+rx, y+ry, PT_SPNG);
				parts[r>>8].life = parts[r>>8].tmp;
				break;
			case PT_ISOZ:
			case PT_ISZS:
				sim->create_part(r>>8, x+rx, y+ry, PT_EXOT);
				break;
			case PT_NITR:
				if (parts[r>>8].temp < 75)
					sim->part_change_type(r>>8, x+rx, y+ry, PT_LNTG);
				break;
			case PT_PLNT:
				sim->part_change_type(exot_id, exot_pos_x, exot_pos_y, PT_VIRS);
				parts[exot_id].tmp2 = PT_EXOT;
				return 0;
			case PT_PLSM:
				sim->create_part(i, x, y, PT_PLSM);
				return 0;
			case PT_SAND:
				if (parts[r>>8].temp >= 1750)
				{
					sim->part_change_type(r>>8, x+rx, y+ry, PT_LAVA);
					switch (rand() % 3)
					{
						case 0: parts[r>>8].ctype = PT_PSCN; break;
						case 1: parts[r>>8].ctype = PT_NSCN; break;
						case 2: parts[r>>8].ctype = PT_SWCH; break;
					}
				}
				break;
			case PT_TESC:
				sim->create_part(r>>8, x+rx, y+ry, PT_EMP);
				break;
			case PT_URAN:
				sim->create_part(i, x, y, PT_PLUT);
				sim->create_part(r>>8, x+rx, y+ry, PT_PLUT);
				return 0;
				
			// particle's type is PT_DMND and PT_INDI are indestructible.
			}
		}
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_E185 static int graphics(GRAPHICS_FUNC_ARGS)
int Element_E185::graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->tmp2 & 1) {
		*colr = 0xFF;
		*colg = 0xA0;
		*colb = 0x00;
	} else if (cpart->tmp >= 20) {
		*colr = 0x70;
		*colg = 0x70;
		*colb = 0x70;
	} else if (cpart->tmp >= 10) {
		*colr = 0x78;
		*colg = 0x98;
		*colb = 0x50;
	}
	return 0;
}

Element_E185::~Element_E185() {}
