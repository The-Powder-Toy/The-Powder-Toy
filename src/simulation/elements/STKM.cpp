#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_STKM PT_STKM 55
Element_STKM::Element_STKM()
{
	Identifier = "DEFAULT_PT_STKM";
	Name = "STKM";
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
	Description = "Stickman. Don't kill him!";
	
	State = ST_NONE;
	Properties = 0;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 620.0f;
	HighTemperatureTransition = PT_FIRE;
	
	Update = &Element_STKM::update;
	Graphics = &Element_STKM::graphics;
}

//#TPT-Directive ElementHeader Element_STKM static int update(UPDATE_FUNC_ARGS)
int Element_STKM::update(UPDATE_FUNC_ARGS)

{
	run_stickman(&sim->player, UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}



//#TPT-Directive ElementHeader Element_STKM static int graphics(GRAPHICS_FUNC_ARGS)
int Element_STKM::graphics(GRAPHICS_FUNC_ARGS)
{	
	*colr = *colg = *colb = *cola = 0;
	*pixel_mode = PSPEC_STICKMAN;
	return 1;
}

#define INBOND(x, y) ((x)>=0 && (y)>=0 && (x)<XRES && (y)<YRES)

//#TPT-Directive ElementHeader Element_STKM static int run_stickman(playerst* playerp, UPDATE_FUNC_ARGS)
int Element_STKM::run_stickman(playerst* playerp, UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	int t = parts[i].type;
	float pp, d;
	float dt = 0.9;///(FPSB*FPSB);  //Delta time in square
	float gvx, gvy;
	float gx, gy, dl, dr;

	if ((parts[i].ctype>0 && parts[i].ctype<PT_NUM && sim->elements[parts[i].ctype].Enabled && sim->elements[parts[i].ctype].Falldown>0) || parts[i].ctype==SPC_AIR || parts[i].ctype == PT_NEUT || parts[i].ctype == PT_PHOT || parts[i].ctype == PT_LIGH)
		playerp->elem = parts[i].ctype;
	playerp->frames++;

	//Temperature handling
	if (parts[i].temp<243)
		parts[i].life -= 1;
	if ((parts[i].temp<309.6f) && (parts[i].temp>=243))
		parts[i].temp += 1;

	//Death
	if (parts[i].life<1 || (sim->pv[y/CELL][x/CELL]>=4.5f && playerp->elem != SPC_AIR) ) //If his HP is less that 0 or there is very big wind...
	{
		for (r=-2; r<=1; r++)
		{
			sim->create_part(-1, x+r, y-2, playerp->elem);
			sim->create_part(-1, x+r+1, y+2, playerp->elem);
			sim->create_part(-1, x-2, y+r+1, playerp->elem);
			sim->create_part(-1, x+2, y+r, playerp->elem);
		}
		sim->kill_part(i);  //Kill him
		return 1;
	}

	//Follow gravity
	gvx = gvy = 0.0f;
	switch (sim->gravityMode)
	{
		default:
		case 0:
			gvy = 1;
			break;
		case 1:
			gvy = gvx = 0.0f;
			break;
		case 2:
			{
				float gravd;
				gravd = 0.01f - hypotf((parts[i].x - XCNTR), (parts[i].y - YCNTR));
				gvx = ((float)(parts[i].x - XCNTR) / gravd);
				gvy = ((float)(parts[i].y - YCNTR) / gravd);
			}
			break;
	}

	gvx += sim->gravx[((int)parts[i].y/CELL)*(XRES/CELL)+((int)parts[i].x/CELL)];
	gvy += sim->gravy[((int)parts[i].y/CELL)*(XRES/CELL)+((int)parts[i].x/CELL)];

	parts[i].vx -= gvx*dt;  //Head up!
	parts[i].vy -= gvy*dt;

	//Verlet integration
	pp = 2*playerp->legs[0]-playerp->legs[2]+playerp->accs[0]*dt*dt;
	playerp->legs[2] = playerp->legs[0];
	playerp->legs[0] = pp;
	pp = 2*playerp->legs[1]-playerp->legs[3]+playerp->accs[1]*dt*dt;
	playerp->legs[3] = playerp->legs[1];
	playerp->legs[1] = pp;

	pp = 2*playerp->legs[4]-playerp->legs[6]+(playerp->accs[2]+gvx)*dt*dt;
	playerp->legs[6] = playerp->legs[4];
	playerp->legs[4] = pp;
	pp = 2*playerp->legs[5]-playerp->legs[7]+(playerp->accs[3]+gvy)*dt*dt;
	playerp->legs[7] = playerp->legs[5];
	playerp->legs[5] = pp;

	pp = 2*playerp->legs[8]-playerp->legs[10]+playerp->accs[4]*dt*dt;
	playerp->legs[10] = playerp->legs[8];
	playerp->legs[8] = pp;
	pp = 2*playerp->legs[9]-playerp->legs[11]+playerp->accs[5]*dt*dt;
	playerp->legs[11] = playerp->legs[9];
	playerp->legs[9] = pp;

	pp = 2*playerp->legs[12]-playerp->legs[14]+(playerp->accs[6]+gvx)*dt*dt;
	playerp->legs[14] = playerp->legs[12];
	playerp->legs[12] = pp;
	pp = 2*playerp->legs[13]-playerp->legs[15]+(playerp->accs[7]+gvy)*dt*dt;
	playerp->legs[15] = playerp->legs[13];
	playerp->legs[13] = pp;

	//Setting acceleration to 0
	playerp->accs[0] = 0;
	playerp->accs[1] = 0;

	playerp->accs[2] = 0;
	playerp->accs[3] = 0;

	playerp->accs[4] = 0;
	playerp->accs[5] = 0;

	playerp->accs[6] = 0;
	playerp->accs[7] = 0;

	gx = (playerp->legs[4] + playerp->legs[12])/2 - gvy;
	gy = (playerp->legs[5] + playerp->legs[13])/2 + gvx;
	dl = pow(gx - playerp->legs[4], 2) + pow(gy - playerp->legs[5], 2);
	dr = pow(gx - playerp->legs[12], 2) + pow(gy - playerp->legs[13], 2);
	
	//Go left
	if (((int)(playerp->comm)&0x01) == 0x01)
	{
		if (dl>dr)
		{
			if (!sim->eval_move(t, playerp->legs[4], playerp->legs[5], NULL))
			{
				playerp->accs[2] = -3*gvy-3*gvx;
				playerp->accs[3] = 3*gvx-3*gvy;
				playerp->accs[0] = -gvy;
				playerp->accs[1] = gvx;
			}
		}
		else
		{
			if (!sim->eval_move(t, playerp->legs[12], playerp->legs[13], NULL))
			{
				playerp->accs[6] = -3*gvy-3*gvx;
				playerp->accs[7] = 3*gvx-3*gvy;
				playerp->accs[0] = -gvy;
				playerp->accs[1] = gvx;
			}
		}
	}

	//Go right
	if (((int)(playerp->comm)&0x02) == 0x02)
	{
		if (dl<dr)
		{
			if (!sim->eval_move(t, playerp->legs[4], playerp->legs[5], NULL))
			{
				playerp->accs[2] = 3*gvy-3*gvx;
				playerp->accs[3] = -3*gvx-3*gvy;
				playerp->accs[0] = gvy;
				playerp->accs[1] = -gvx;
			}
		}
		else
		{
			if (!sim->eval_move(t, playerp->legs[12], playerp->legs[13], NULL))
			{
				playerp->accs[6] = 3*gvy-3*gvx;
				playerp->accs[7] = -3*gvx-3*gvy;
				playerp->accs[0] = gvy;
				playerp->accs[1] = -gvx;
			}
		}
	}

	//Jump
	if (((int)(playerp->comm)&0x04) == 0x04 && 
			(!sim->eval_move(t, playerp->legs[4], playerp->legs[5], NULL) || !sim->eval_move(t, playerp->legs[12], playerp->legs[13], NULL)))
	{
		parts[i].vy -= 4*gvy;
		playerp->accs[3] -= gvy;
		playerp->accs[7] -= gvy;
	}

	//Charge detector wall if foot inside
	if (sim->bmap[(int)(playerp->legs[5]+0.5)/CELL][(int)(playerp->legs[4]+0.5)/CELL]==WL_DETECT)
		sim->set_emap((int)playerp->legs[4]/CELL, (int)playerp->legs[5]/CELL);
	if (sim->bmap[(int)(playerp->legs[13]+0.5)/CELL][(int)(playerp->legs[12]+0.5)/CELL]==WL_DETECT)
		sim->set_emap((int)(playerp->legs[12]+0.5)/CELL, (int)(playerp->legs[13]+0.5)/CELL);

	//Searching for particles near head
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					r = sim->photons[y+ry][x+rx];

				if (!r && !sim->bmap[(y+ry)/CELL][(x+rx)/CELL])
					continue;
				
				if (sim->elements[r&0xFF].Falldown!=0 || sim->elements[r&0xFF].State == ST_GAS
					|| sim->elements[r&0xFF].Properties&TYPE_GAS
					|| sim->elements[r&0xFF].Properties&TYPE_LIQUID
					|| (r&0xFF) == PT_NEUT || (r&0xFF) == PT_PHOT)
				{
					playerp->elem = r&0xFF;  //Current element
				}
				if ((r&0xFF)==PT_TESC || (r&0xFF)==PT_LIGH)
					playerp->elem = PT_LIGH;
				if ((r&0xFF) == PT_PLNT && parts[i].life<100) //Plant gives him 5 HP
				{
					if (parts[i].life<=95)
						parts[i].life += 5;
					else
						parts[i].life = 100;
					sim->kill_part(r>>8);
				}

				if ((r&0xFF) == PT_NEUT)
				{
					if (parts[i].life<=100) parts[i].life -= (102-parts[i].life)/2;
					else parts[i].life *= 0.9f;
					sim->kill_part(r>>8);
				}
				if (sim->bmap[(ry+y)/CELL][(rx+x)/CELL]==WL_FAN)
					playerp->elem = SPC_AIR;
				if ((r&0xFF)==PT_PRTI)
					Element_STKM::STKM_interact(sim, playerp, i, rx, ry);
				if (!parts[i].type)//STKM_interact may kill STKM
					return 1;
			}

	//Head position
	rx = x + 3*((((int)playerp->pcomm)&0x02) == 0x02) - 3*((((int)playerp->pcomm)&0x01) == 0x01);
	ry = y - 3*(playerp->pcomm == 0);

	//Spawn
	if (((int)(playerp->comm)&0x08) == 0x08)
	{
		ry -= 2*(rand()%2)+1;
		r = pmap[ry][rx];
		if (sim->elements[r&0xFF].State == ST_SOLID)
		{
			sim->create_part(-1, rx, ry, PT_SPRK);
			playerp->frames = 0;
		}
		else
		{
			int np = -1;
			if (playerp->elem == SPC_AIR)
				sim->CreateParts(rx + 3*((((int)playerp->pcomm)&0x02) == 0x02) - 3*((((int)playerp->pcomm)&0x01) == 0x01), ry, 4, 4, SPC_AIR, 0);
			else if (playerp->elem==PT_LIGH && playerp->frames<30)//limit lightning creation rate
				np = -1;
			else
				np = sim->create_part(-1, rx, ry, playerp->elem);
			if ( (np < NPART) && np>=0)
			{
				if (playerp->elem == PT_PHOT)
				{
					int random = abs(rand()%3-1)*3;
					if (random==0)
					{
						sim->kill_part(np);
					}
					else
					{
						parts[np].vy = 0;
						if (((int)playerp->pcomm)&(0x01|0x02))
							parts[np].vx = (((((int)playerp->pcomm)&0x02) == 0x02) - (((int)(playerp->pcomm)&0x01) == 0x01))*random;
						else
							parts[np].vx = random;
					}
				}
				else if (playerp->elem == PT_LIGH)
				{
					float angle;
					int power = 100;
					if (gvx!=0 || gvy!=0)
						angle = atan2(gvx, gvy)*180.0f/M_PI;
					else
						angle = rand()%360;
					if (((int)playerp->comm)&0x01)
						angle += 180;
					if (angle>360)
						angle-=360;
					if (angle<0)
						angle+=360;
					parts[np].tmp = angle;
					parts[np].life=rand()%(2+power/15)+power/7;
					parts[np].temp=parts[np].life*power/2.5;
					parts[np].tmp2=1;
				}
				else if (playerp->elem != SPC_AIR)
				{
					parts[np].vx -= -gvy*(5*((((int)playerp->pcomm)&0x02) == 0x02) - 5*(((int)(playerp->pcomm)&0x01) == 0x01));
					parts[np].vy -= gvx*(5*((((int)playerp->pcomm)&0x02) == 0x02) - 5*(((int)(playerp->pcomm)&0x01) == 0x01));
					parts[i].vx -= (sim->elements[(int)playerp->elem].Weight*parts[np].vx)/1000;
				}
				playerp->frames = 0;
			}

		}
	}

	//Simulation of joints
	d = 25/(pow((playerp->legs[0]-playerp->legs[4]), 2) + pow((playerp->legs[1]-playerp->legs[5]), 2)+25) - 0.5;  //Fast distance
	playerp->legs[4] -= (playerp->legs[0]-playerp->legs[4])*d;
	playerp->legs[5] -= (playerp->legs[1]-playerp->legs[5])*d;
	playerp->legs[0] += (playerp->legs[0]-playerp->legs[4])*d;
	playerp->legs[1] += (playerp->legs[1]-playerp->legs[5])*d;

	d = 25/(pow((playerp->legs[8]-playerp->legs[12]), 2) + pow((playerp->legs[9]-playerp->legs[13]), 2)+25) - 0.5;
	playerp->legs[12] -= (playerp->legs[8]-playerp->legs[12])*d;
	playerp->legs[13] -= (playerp->legs[9]-playerp->legs[13])*d;
	playerp->legs[8] += (playerp->legs[8]-playerp->legs[12])*d;
	playerp->legs[9] += (playerp->legs[9]-playerp->legs[13])*d;

	d = 36/(pow((playerp->legs[0]-parts[i].x), 2) + pow((playerp->legs[1]-parts[i].y), 2)+36) - 0.5;
	parts[i].vx -= (playerp->legs[0]-parts[i].x)*d;
	parts[i].vy -= (playerp->legs[1]-parts[i].y)*d;
	playerp->legs[0] += (playerp->legs[0]-parts[i].x)*d;
	playerp->legs[1] += (playerp->legs[1]-parts[i].y)*d;

	d = 36/(pow((playerp->legs[8]-parts[i].x), 2) + pow((playerp->legs[9]-parts[i].y), 2)+36) - 0.5;
	parts[i].vx -= (playerp->legs[8]-parts[i].x)*d;
	parts[i].vy -= (playerp->legs[9]-parts[i].y)*d;
	playerp->legs[8] += (playerp->legs[8]-parts[i].x)*d;
	playerp->legs[9] += (playerp->legs[9]-parts[i].y)*d;

	if (INBOND(playerp->legs[4], playerp->legs[5]) && !sim->eval_move(t, playerp->legs[4], playerp->legs[5], NULL))
	{
		playerp->legs[4] = playerp->legs[6];
		playerp->legs[5] = playerp->legs[7];
	}

	if (INBOND(playerp->legs[12], playerp->legs[13]) && !sim->eval_move(t, playerp->legs[12], playerp->legs[13], NULL))
	{
		playerp->legs[12] = playerp->legs[14];
		playerp->legs[13] = playerp->legs[15];
	}

	//This makes stick man "pop" from obstacles
	if (INBOND(playerp->legs[4], playerp->legs[5]) && !sim->eval_move(t, playerp->legs[4], playerp->legs[5], NULL))
	{
		float t;
		t = playerp->legs[4]; playerp->legs[4] = playerp->legs[6]; playerp->legs[6] = t;
		t = playerp->legs[5]; playerp->legs[5] = playerp->legs[7]; playerp->legs[7] = t;
	}

	if (INBOND(playerp->legs[12], playerp->legs[13]) && !sim->eval_move(t, playerp->legs[12], playerp->legs[13], NULL))
	{
		float t;
		t = playerp->legs[12]; playerp->legs[12] = playerp->legs[14]; playerp->legs[14] = t;
		t = playerp->legs[13]; playerp->legs[13] = playerp->legs[15]; playerp->legs[15] = t;
	}

	//Keeping legs distance
	if ((pow((playerp->legs[4] - playerp->legs[12]), 2) + pow((playerp->legs[5]-playerp->legs[13]), 2))<16)
	{
		float tvx, tvy;
		tvx = -gvy;
		tvy = gvx;

		if (tvx || tvy)
		{
			playerp->accs[2] -= 0.2*tvx/hypot(tvx, tvy);
			playerp->accs[3] -= 0.2*tvy/hypot(tvx, tvy);

			playerp->accs[6] += 0.2*tvx/hypot(tvx, tvy);
			playerp->accs[7] += 0.2*tvy/hypot(tvx, tvy);
		}
	}

	if ((pow((playerp->legs[0] - playerp->legs[8]), 2) + pow((playerp->legs[1]-playerp->legs[9]), 2))<16)
	{
		float tvx, tvy;
		tvx = -gvy;
		tvy = gvx;

		if (tvx || tvy)
		{
			playerp->accs[0] -= 0.2*tvx/hypot(tvx, tvy);
			playerp->accs[1] -= 0.2*tvy/hypot(tvx, tvy);

			playerp->accs[4] += 0.2*tvx/hypot(tvx, tvy);
			playerp->accs[5] += 0.2*tvy/hypot(tvx, tvy);
		}
	}

	//If legs touch something
	Element_STKM::STKM_interact(sim, playerp, i, (int)(playerp->legs[4]+0.5), (int)(playerp->legs[5]+0.5));
	Element_STKM::STKM_interact(sim, playerp, i, (int)(playerp->legs[12]+0.5), (int)(playerp->legs[13]+0.5));
	Element_STKM::STKM_interact(sim, playerp, i, (int)(playerp->legs[4]+0.5), (int)playerp->legs[5]);
	Element_STKM::STKM_interact(sim, playerp, i, (int)(playerp->legs[12]+0.5), (int)playerp->legs[13]);
	if (!parts[i].type)
		return 1;

	parts[i].ctype = playerp->elem;
	return 0;
}

//#TPT-Directive ElementHeader Element_STKM static void STKM_interact(Simulation * sim, playerst* playerp, int i, int x, int y)
void Element_STKM::STKM_interact(Simulation * sim, playerst* playerp, int i, int x, int y)
{
	int r;
	if (x<0 || y<0 || x>=XRES || y>=YRES || !sim->parts[i].type)
		return;
	r = sim->pmap[y][x];
	if (r)
	{
		if ((r&0xFF)==PT_SPRK && playerp->elem!=PT_LIGH) //If on charge
		{
			sim->parts[i].life -= (int)(rand()*20/RAND_MAX)+32;
		}

		if (sim->elements[r&0xFF].HeatConduct && ((playerp->elem!=PT_LIGH && sim->parts[r>>8].temp>=323) || sim->parts[r>>8].temp<=243))
		{
			sim->parts[i].life -= 2;
			playerp->accs[3] -= 1;
		}
			
		if (sim->elements[r&0xFF].Properties&PROP_DEADLY)
			switch (r&0xFF)
			{
				case PT_ACID:
					sim->parts[i].life -= 5;
					break;
				default:
					sim->parts[i].life -= 1;
					break;
			}

		if (sim->elements[r&0xFF].Properties&PROP_RADIOACTIVE)
			sim->parts[i].life -= 1;

		if ((r&0xFF)==PT_PRTI && sim->parts[i].type)
		{
			int nnx, count=1;//gives rx=0, ry=1 in update_PRTO
			sim->parts[r>>8].tmp = (int)((sim->parts[r>>8].temp-73.15f)/100+1);
			if (sim->parts[r>>8].tmp>=CHANNELS) sim->parts[r>>8].tmp = CHANNELS-1;
			else if (sim->parts[r>>8].tmp<0) sim->parts[r>>8].tmp = 0;
			for (nnx=0; nnx<80; nnx++)
				if (!sim->portalp[sim->parts[r>>8].tmp][count][nnx].type)
				{
					sim->portalp[sim->parts[r>>8].tmp][count][nnx] = sim->parts[i];
					sim->kill_part(i);
					//stop new STKM/fighters being created to replace the ones in the portal:
					playerp->spwn = 1;
					if (sim->portalp[sim->parts[r>>8].tmp][count][nnx].type==PT_FIGH)
						sim->fighcount++;
					break;
				}
		}
		if (((r&0xFF)==PT_BHOL || (r&0xFF)==PT_NBHL) && sim->parts[i].type)
		{
			if (!sim->legacy_enable)
			{
				sim->parts[r>>8].temp = restrict_flt(sim->parts[r>>8].temp+sim->parts[i].temp/2, MIN_TEMP, MAX_TEMP);
			}
			sim->kill_part(i);
		}
		if (((r&0xFF)==PT_VOID || ((r&0xFF)==PT_PVOD && sim->parts[r>>8].life==10)) && (!sim->parts[r>>8].ctype || (sim->parts[r>>8].ctype==sim->parts[i].type)!=(sim->parts[r>>8].tmp&1)) && sim->parts[i].type)
		{
			sim->kill_part(i);
		}
	}
}

//#TPT-Directive ElementHeader Element_STKM static void STKM_init_legs(Simulation * sim, playerst* playerp, int i)
void Element_STKM::STKM_init_legs(Simulation * sim, playerst* playerp, int i)
{
	int x, y;

	x = (int)(sim->parts[i].x+0.5f);
	y = (int)(sim->parts[i].y+0.5f);

	playerp->legs[0] = x-1;
	playerp->legs[1] = y+6;
	playerp->legs[2] = x-1;
	playerp->legs[3] = y+6;

	playerp->legs[4] = x-3;
	playerp->legs[5] = y+12;
	playerp->legs[6] = x-3;
	playerp->legs[7] = y+12;

	playerp->legs[8] = x+1;
	playerp->legs[9] = y+6;
	playerp->legs[10] = x+1;
	playerp->legs[11] = y+6;

	playerp->legs[12] = x+3;
	playerp->legs[13] = y+12;
	playerp->legs[14] = x+3;
	playerp->legs[15] = y+12;

	for (int i = 0; i < 8; i++)
		playerp->accs[i] = 0;
	playerp->comm = 0;
	playerp->pcomm = 0;
	playerp->frames = 0;
}


Element_STKM::~Element_STKM() {}
