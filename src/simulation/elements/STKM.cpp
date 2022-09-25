#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
int Element_STKM_graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);
static bool createAllowed(ELEMENT_CREATE_ALLOWED_FUNC_ARGS);
static void changeType(ELEMENT_CHANGETYPE_FUNC_ARGS);
void Element_STKM_init_legs(Simulation * sim, playerst *playerp, int i);
int Element_STKM_run_stickman(playerst *playerp, UPDATE_FUNC_ARGS);
void Element_STKM_set_element(Simulation *sim, playerst *playerp, int element);
void Element_STKM_interact(Simulation *sim, playerst *playerp, int i, int x, int y);

void Element::Element_STKM()
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
	Description = "Stickman. Don't kill him! Control with the arrow keys.";

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
	Create = &create;
	CreateAllowed = &createAllowed;
	ChangeType = &changeType;
}

static int update(UPDATE_FUNC_ARGS)
{
	Element_STKM_run_stickman(&sim->player, UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

int Element_STKM_graphics(GRAPHICS_FUNC_ARGS)
{
	*colr = *colg = *colb = *cola = 0;
	*pixel_mode = PSPEC_STICKMAN;
	return 1;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	int spawnID = sim->create_part(-3, x, y, PT_SPAWN);
	if (spawnID >= 0)
		sim->player.spawnID = spawnID;
}

static bool createAllowed(ELEMENT_CREATE_ALLOWED_FUNC_ARGS)
{
	return sim->elementCount[PT_STKM] <= 0 && !sim->player.spwn;
}

static void changeType(ELEMENT_CHANGETYPE_FUNC_ARGS)
{
	if (to == PT_STKM)
	{
		Element_STKM_init_legs(sim, &sim->player, i);
		sim->player.spwn = 1;
	}
	else
		sim->player.spwn = 0;
}

#define INBOND(x, y) ((x)>=0 && (y)>=0 && (x)<XRES && (y)<YRES)

int Element_STKM_run_stickman(playerst *playerp, UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	int t = parts[i].type;
	float pp, d;
	float dt = 0.9f;///(FPSB*FPSB);  //Delta time in square
	float gvx, gvy;
	float gx, gy, dl, dr;
	float rocketBootsHeadEffect = 0.35f;
	float rocketBootsFeetEffect = 0.15f;
	float rocketBootsHeadEffectV = 0.3f;// stronger acceleration vertically, to counteract gravity
	float rocketBootsFeetEffectV = 0.45f;

	if (!playerp->fan && parts[i].ctype && sim->IsElementOrNone(parts[i].ctype))
		Element_STKM_set_element(sim, playerp, parts[i].ctype);
	playerp->frames++;

	//Temperature handling
	if (parts[i].temp<243)
		parts[i].life -= 1;
	if ((parts[i].temp<309.6f) && (parts[i].temp>=243))
		parts[i].temp += 1;

	//Death
	if (parts[i].life<1 || (sim->pv[y/CELL][x/CELL]>=4.5f && !playerp->fan) ) //If his HP is less than 0 or there is very big wind...
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
		case 3:
			gvx = sim->customGravityX;
			gvy = sim->customGravityY;
			break;
	}

	gvx += sim->gravx[((int)parts[i].y/CELL)*(XRES/CELL)+((int)parts[i].x/CELL)];
	gvy += sim->gravy[((int)parts[i].y/CELL)*(XRES/CELL)+((int)parts[i].x/CELL)];

	float mvx = gvx;
	float mvy = gvy;
	bool rbLowGrav = false;
	float tmp = fabsf(mvx) > fabsf(mvy)?fabsf(mvx):fabsf(mvy);
	if (tmp < 0.001f)
	{
		rbLowGrav = true;
		mvx = -parts[i].vx;
		mvy = -parts[i].vy;
		tmp = fabsf(mvx) > fabsf(mvy)?fabsf(mvx):fabsf(mvy);
	}
	if (tmp < 0.001f)
	{
		mvx = 0;
		mvy = 1.0f;
		tmp = 1.0f;
	}
	else
		tmp = 1.0f/sqrtf(mvx*mvx+mvy*mvy);
	mvx *= tmp;// scale to a unit vector
	mvy *= tmp;
	if (rbLowGrav)
	{
		rocketBootsHeadEffectV = rocketBootsHeadEffect;
		rocketBootsFeetEffectV = rocketBootsFeetEffect;
	}

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
		bool moved = false;
		if (dl>dr)
		{
			if (INBOND(playerp->legs[4], playerp->legs[5]) && !sim->eval_move(t, int(playerp->legs[4]), int(playerp->legs[5]), NULL))
			{
				playerp->accs[2] = -3*mvy-3*mvx;
				playerp->accs[3] = 3*mvx-3*mvy;
				playerp->accs[0] = -mvy;
				playerp->accs[1] = mvx;
				moved = true;
			}
		}
		else
		{
			if (INBOND(playerp->legs[12], playerp->legs[13]) && !sim->eval_move(t, int(playerp->legs[12]), int(playerp->legs[13]), NULL))
			{
				playerp->accs[6] = -3*mvy-3*mvx;
				playerp->accs[7] = 3*mvx-3*mvy;
				playerp->accs[0] = -mvy;
				playerp->accs[1] = mvx;
				moved = true;
			}
		}
		if (!moved && playerp->rocketBoots)
		{
			parts[i].vx -= rocketBootsHeadEffect*mvy;
			parts[i].vy += rocketBootsHeadEffect*mvx;
			playerp->accs[2] -= rocketBootsFeetEffect*mvy;
			playerp->accs[6] -= rocketBootsFeetEffect*mvy;
			playerp->accs[3] += rocketBootsFeetEffect*mvx;
			playerp->accs[7] += rocketBootsFeetEffect*mvx;
			for (int leg=0; leg<2; leg++)
			{
				if (leg==1 && (((int)(playerp->comm)&0x02) == 0x02))
					continue;
				int footX = int(playerp->legs[leg*8+4]), footY = int(playerp->legs[leg*8+5]);
				int np = sim->create_part(-1, footX, footY, PT_PLSM);
				if (np>=0)
				{
					parts[np].vx = parts[i].vx+mvy*25;
					parts[np].vy = parts[i].vy-mvx*25;
					parts[np].life += 30;
				}
			}
		}
	}

	//Go right
	if (((int)(playerp->comm)&0x02) == 0x02)
	{
		bool moved = false;
		if (dl<dr)
		{
			if (INBOND(playerp->legs[4], playerp->legs[5]) && !sim->eval_move(t, int(playerp->legs[4]), int(playerp->legs[5]), NULL))
			{
				playerp->accs[2] = 3*mvy-3*mvx;
				playerp->accs[3] = -3*mvx-3*mvy;
				playerp->accs[0] = mvy;
				playerp->accs[1] = -mvx;
				moved = true;
			}
		}
		else
		{
			if (INBOND(playerp->legs[12], playerp->legs[13]) && !sim->eval_move(t, int(playerp->legs[12]), int(playerp->legs[13]), NULL))
			{
				playerp->accs[6] = 3*mvy-3*mvx;
				playerp->accs[7] = -3*mvx-3*mvy;
				playerp->accs[0] = mvy;
				playerp->accs[1] = -mvx;
				moved = true;
			}
		}
		if (!moved && playerp->rocketBoots)
		{
			parts[i].vx += rocketBootsHeadEffect*mvy;
			parts[i].vy -= rocketBootsHeadEffect*mvx;
			playerp->accs[2] += rocketBootsFeetEffect*mvy;
			playerp->accs[6] += rocketBootsFeetEffect*mvy;
			playerp->accs[3] -= rocketBootsFeetEffect*mvx;
			playerp->accs[7] -= rocketBootsFeetEffect*mvx;
			for (int leg=0; leg<2; leg++)
			{
				if (leg==0 && (((int)(playerp->comm)&0x01) == 0x01))
					continue;
				int footX = int(playerp->legs[leg*8+4]), footY = int(playerp->legs[leg*8+5]);
				int np = sim->create_part(-1, footX, footY, PT_PLSM);
				if (np>=0)
				{
					parts[np].vx = parts[i].vx-mvy*25;
					parts[np].vy = parts[i].vy+mvx*25;
					parts[np].life += 30;
				}
			}
		}
	}

	if (playerp->rocketBoots && ((int)(playerp->comm)&0x03) == 0x03)
	{
		// Pressing left and right simultaneously with rocket boots on slows the stickman down
		// Particularly useful in zero gravity
		parts[i].vx *= 0.5f;
		parts[i].vy *= 0.5f;
		playerp->accs[2] = playerp->accs[6] = 0;
		playerp->accs[3] = playerp->accs[7] = 0;
	}

	//Jump
	if (((int)(playerp->comm)&0x04) == 0x04)
	{
		if (playerp->rocketBoots)
		{
			parts[i].vx -= rocketBootsHeadEffectV*mvx;
			parts[i].vy -= rocketBootsHeadEffectV*mvy;
			playerp->accs[2] -= rocketBootsFeetEffectV*mvx;
			playerp->accs[6] -= rocketBootsFeetEffectV*mvx;
			playerp->accs[3] -= rocketBootsFeetEffectV*mvy;
			playerp->accs[7] -= rocketBootsFeetEffectV*mvy;
			for (int leg=0; leg<2; leg++)
			{
				int footX = int(playerp->legs[leg*8+4]), footY = int(playerp->legs[leg*8+5]);
				int np = sim->create_part(-1, footX, footY+1, PT_PLSM);
				if (np>=0)
				{
					parts[np].vx = parts[i].vx+mvx*30;
					parts[np].vy = parts[i].vy+mvy*30;
					parts[np].life += 10;
				}
			}
		}
		else if ((INBOND(playerp->legs[4], playerp->legs[5]) && !sim->eval_move(t, int(playerp->legs[4]), int(playerp->legs[5]), NULL)) ||
				 (INBOND(playerp->legs[12], playerp->legs[13]) && !sim->eval_move(t, int(playerp->legs[12]), int(playerp->legs[13]), NULL)))
		{
			parts[i].vx -= 4*mvx;
			parts[i].vy -= 4*mvy;
			playerp->accs[2] -= mvx;
			playerp->accs[6] -= mvx;
			playerp->accs[3] -= mvy;
			playerp->accs[7] -= mvy;
		}
	}

	//Charge detector wall if foot inside
	if (INBOND((int)(playerp->legs[4]+0.5)/CELL, (int)(playerp->legs[5]+0.5)/CELL) &&
	       sim->bmap[(int)(playerp->legs[5]+0.5)/CELL][(int)(playerp->legs[4]+0.5)/CELL]==WL_DETECT)
		sim->set_emap((int)playerp->legs[4]/CELL, (int)playerp->legs[5]/CELL);
	if (INBOND((int)(playerp->legs[12]+0.5)/CELL, (int)(playerp->legs[13]+0.5)/CELL) &&
	        sim->bmap[(int)(playerp->legs[13]+0.5)/CELL][(int)(playerp->legs[12]+0.5)/CELL]==WL_DETECT)
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

				Element_STKM_set_element(sim, playerp, TYP(r));
				if (TYP(r) == PT_PLNT && parts[i].life<100) //Plant gives him 5 HP
				{
					if (parts[i].life<=95)
						parts[i].life += 5;
					else
						parts[i].life = 100;
					sim->kill_part(ID(r));
				}

				if (TYP(r) == PT_NEUT)
				{
					if (parts[i].life<=100) parts[i].life -= (102-parts[i].life)/2;
					else parts[i].life = int(parts[i].life * 0.9f);
					sim->kill_part(ID(r));
				}
				if (sim->bmap[(ry+y)/CELL][(rx+x)/CELL]==WL_FAN)
					playerp->fan = true;
				else if (sim->bmap[(ry+y)/CELL][(rx+x)/CELL]==WL_EHOLE)
					playerp->rocketBoots = false;
				else if (sim->bmap[(ry+y)/CELL][(rx+x)/CELL]==WL_GRAV /* && parts[i].type!=PT_FIGH */)
					playerp->rocketBoots = true;
				if (TYP(r)==PT_PRTI)
					Element_STKM_interact(sim, playerp, i, rx, ry);
				if (!parts[i].type)//STKM_interact may kill STKM
					return 1;
			}

	//Head position
	rx = x + 3*((((int)playerp->pcomm)&0x02) == 0x02) - 3*((((int)playerp->pcomm)&0x01) == 0x01);
	ry = y - 3*(playerp->pcomm == 0);

	//Spawn
	if (((int)(playerp->comm)&0x08) == 0x08)
	{
		ry -= 2 * RNG::Ref().between(0, 1) + 1;
		r = pmap[ry][rx];
		if (sim->elements[TYP(r)].Properties&TYPE_SOLID)
		{
			sim->create_part(-1, rx, ry, PT_SPRK);
			playerp->frames = 0;
		}
		else
		{
			int np = -1;
			if (playerp->fan)
			{
				for(int j = -4; j < 5; j++)
					for (int k = -4; k < 5; k++)
					{
						int airx = rx + 3*((((int)playerp->pcomm)&0x02) == 0x02) - 3*((((int)playerp->pcomm)&0x01) == 0x01)+j;
						int airy = ry+k;
						sim->pv[airy/CELL][airx/CELL] += 0.03f;
						if (airy + CELL < YRES)
							sim->pv[airy/CELL+1][airx/CELL] += 0.03f;
						if (airx + CELL < XRES)
						{
							sim->pv[airy/CELL][airx/CELL+1] += 0.03f;
							if (airy + CELL < YRES)
								sim->pv[airy/CELL+1][airx/CELL+1] += 0.03f;
						}
					}
			}
			else if (playerp->elem==PT_LIGH && playerp->frames<30)//limit lightning creation rate
				np = -1;
			else
				np = sim->create_part(-1, rx, ry, playerp->elem);
			if ( (np < NPART) && np>=0)
			{
				if (playerp->elem == PT_PHOT)
				{
					int random = abs((RNG::Ref().between(-1, 1)))*3;
					if (random==0)
					{
						sim->kill_part(np);
					}
					else
					{
						parts[np].vy = 0;
						if (((int)playerp->pcomm)&(0x01|0x02))
							parts[np].vx = float((((((int)playerp->pcomm)&0x02) == 0x02) - (((int)(playerp->pcomm)&0x01) == 0x01))*random);
						else
							parts[np].vx = float(random);
					}
				}
				else if (playerp->elem == PT_LIGH)
				{
					int angle;
					int power = 100;
					if (gvx!=0 || gvy!=0)
						angle = int(atan2(mvx, mvy)*180.0f/M_PI);
					else
						angle = RNG::Ref().between(0, 359);
					if (((int)playerp->pcomm)&0x01)
						angle += 180;
					if (angle>360)
						angle-=360;
					if (angle<0)
						angle+=360;
					parts[np].tmp = angle;
					parts[np].life = RNG::Ref().between(0, 1+power/15) + power/7;
					parts[np].temp = parts[np].life*power/2.5;
					parts[np].tmp2 = 1;
				}
				else if (!playerp->fan)
				{
					parts[np].vx -= -mvy*(5*((((int)playerp->pcomm)&0x02) == 0x02) - 5*(((int)(playerp->pcomm)&0x01) == 0x01));
					parts[np].vy -= mvx*(5*((((int)playerp->pcomm)&0x02) == 0x02) - 5*(((int)(playerp->pcomm)&0x01) == 0x01));
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

	if (INBOND(playerp->legs[4], playerp->legs[5]) && !sim->eval_move(t, int(playerp->legs[4]), int(playerp->legs[5]), NULL))
	{
		playerp->legs[4] = playerp->legs[6];
		playerp->legs[5] = playerp->legs[7];
	}

	if (INBOND(playerp->legs[12], playerp->legs[13]) && !sim->eval_move(t, int(playerp->legs[12]), int(playerp->legs[13]), NULL))
	{
		playerp->legs[12] = playerp->legs[14];
		playerp->legs[13] = playerp->legs[15];
	}

	//This makes stick man "pop" from obstacles
	if (INBOND(playerp->legs[4], playerp->legs[5]) && !sim->eval_move(t, int(playerp->legs[4]), int(playerp->legs[5]), NULL))
	{
		float t;
		t = playerp->legs[4]; playerp->legs[4] = playerp->legs[6]; playerp->legs[6] = t;
		t = playerp->legs[5]; playerp->legs[5] = playerp->legs[7]; playerp->legs[7] = t;
	}

	if (INBOND(playerp->legs[12], playerp->legs[13]) && !sim->eval_move(t, int(playerp->legs[12]), int(playerp->legs[13]), NULL))
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
	Element_STKM_interact(sim, playerp, i, (int)(playerp->legs[4]+0.5), (int)(playerp->legs[5]+0.5));
	Element_STKM_interact(sim, playerp, i, (int)(playerp->legs[12]+0.5), (int)(playerp->legs[13]+0.5));
	Element_STKM_interact(sim, playerp, i, (int)(playerp->legs[4]+0.5), (int)playerp->legs[5]);
	Element_STKM_interact(sim, playerp, i, (int)(playerp->legs[12]+0.5), (int)playerp->legs[13]);
	if (!parts[i].type)
		return 1;

	parts[i].ctype = playerp->elem;
	return 0;
}

void Element_STKM_interact(Simulation *sim, playerst *playerp, int i, int x, int y)
{
	int r;
	if (x<0 || y<0 || x>=XRES || y>=YRES || !sim->parts[i].type)
		return;
	r = sim->pmap[y][x];
	if (r)
	{
		if (TYP(r)==PT_SPRK && playerp->elem!=PT_LIGH) //If on charge
		{
			sim->parts[i].life -= RNG::Ref().between(32, 51);
		}

		if (sim->elements[TYP(r)].HeatConduct && (TYP(r)!=PT_HSWC||sim->parts[ID(r)].life==10) && ((playerp->elem!=PT_LIGH && sim->parts[ID(r)].temp>=323) || sim->parts[ID(r)].temp<=243) && (!playerp->rocketBoots || TYP(r)!=PT_PLSM))
		{
			sim->parts[i].life -= 2;
			playerp->accs[3] -= 1;
		}

		if (sim->elements[TYP(r)].Properties&PROP_DEADLY)
			switch (TYP(r))
			{
				case PT_ACID:
					sim->parts[i].life -= 5;
					break;
				default:
					sim->parts[i].life -= 1;
					break;
			}

		if (sim->elements[TYP(r)].Properties&PROP_RADIOACTIVE)
			sim->parts[i].life -= 1;

		if (TYP(r)==PT_PRTI && sim->parts[i].type)
		{
			int nnx, count=1;//gives rx=0, ry=1 in update_PRTO
			sim->parts[ID(r)].tmp = (int)((sim->parts[ID(r)].temp-73.15f)/100+1);
			if (sim->parts[ID(r)].tmp>=CHANNELS) sim->parts[ID(r)].tmp = CHANNELS-1;
			else if (sim->parts[ID(r)].tmp<0) sim->parts[ID(r)].tmp = 0;
			for (nnx=0; nnx<80; nnx++)
				if (!sim->portalp[sim->parts[ID(r)].tmp][count][nnx].type)
				{
					sim->portalp[sim->parts[ID(r)].tmp][count][nnx] = sim->parts[i];
					sim->kill_part(i);
					//stop new STKM/fighters being created to replace the ones in the portal:
					playerp->spwn = 1;
					if (sim->portalp[sim->parts[ID(r)].tmp][count][nnx].type==PT_FIGH)
						sim->fighcount++;
					break;
				}
		}
		if ((TYP(r)==PT_BHOL || TYP(r)==PT_NBHL) && sim->parts[i].type)
		{
			if (!sim->legacy_enable)
			{
				sim->parts[ID(r)].temp = restrict_flt(sim->parts[ID(r)].temp+sim->parts[i].temp/2, MIN_TEMP, MAX_TEMP);
			}
			sim->kill_part(i);
		}
		if ((TYP(r)==PT_VOID || (TYP(r)==PT_PVOD && sim->parts[ID(r)].life==10)) && (!sim->parts[ID(r)].ctype || (sim->parts[ID(r)].ctype==sim->parts[i].type)!=(sim->parts[ID(r)].tmp&1)) && sim->parts[i].type)
		{
			sim->kill_part(i);
		}
	}
}

void Element_STKM_init_legs(Simulation * sim, playerst *playerp, int i)
{
	int x, y;

	x = (int)(sim->parts[i].x+0.5f);
	y = (int)(sim->parts[i].y+0.5f);

	playerp->legs[0] = float(x-1);
	playerp->legs[1] = float(y+6);
	playerp->legs[2] = float(x-1);
	playerp->legs[3] = float(y+6);

	playerp->legs[4] = float(x-3);
	playerp->legs[5] = float(y+12);
	playerp->legs[6] = float(x-3);
	playerp->legs[7] = float(y+12);

	playerp->legs[8] = float(x+1);
	playerp->legs[9] = float(y+6);
	playerp->legs[10] = float(x+1);
	playerp->legs[11] = float(y+6);

	playerp->legs[12] = float(x+3);
	playerp->legs[13] = float(y+12);
	playerp->legs[14] = float(x+3);
	playerp->legs[15] = float(y+12);

	for (int i = 0; i < 8; i++)
		playerp->accs[i] = 0;
	playerp->comm = 0;
	playerp->pcomm = 0;
	playerp->frames = 0;
	playerp->spwn = 0;
	playerp->fan = false;
	playerp->rocketBoots = false;
}

void Element_STKM_set_element(Simulation *sim, playerst *playerp, int element)
{
	if (sim->elements[element].Falldown != 0
	    || sim->elements[element].Properties&TYPE_GAS
	    || sim->elements[element].Properties&TYPE_LIQUID
	    || sim->elements[element].Properties&TYPE_ENERGY
	    || element == PT_LOLZ || element == PT_LOVE)
	{
		if (!playerp->rocketBoots || element != PT_PLSM)
		{
			playerp->elem = element;
			playerp->fan = false;
		}
	}
	if (element == PT_TESC || element == PT_LIGH)
	{
		playerp->elem = PT_LIGH;
		playerp->fan = false;
	}
}
