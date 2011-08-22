#include <element.h>

int update_SPAWN(UPDATE_FUNC_ARGS) {
	if (!player[27])
		create_part(-1, x, y, PT_STKM);

	return 0;
}

int update_STKM(UPDATE_FUNC_ARGS)
{
	run_stickman(player, UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

int run_stickman(float* playerp, UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	float pp, d;
	float dt = 0.9;///(FPSB*FPSB);  //Delta time in square

	if ((parts[i].ctype>0 && parts[i].ctype<PT_NUM && ptypes[parts[i].ctype].falldown>0) || parts[i].ctype==SPC_AIR || parts[i].ctype == PT_NEUT || parts[i].ctype == PT_PHOT)
		playerp[2] = parts[i].ctype;

	//Tempirature handling
	if (parts[i].temp<243)
		parts[i].life -= 1;
	if ((parts[i].temp<309.6f) && (parts[i].temp>=243))
		parts[i].temp += 1;

	//Death
	if (parts[i].life<1 || (pv[y/CELL][x/CELL]>=4.5f && playerp[2] != SPC_AIR) ) //If his HP is less that 0 or there is very big wind...
	{
		for (r=-2; r<=1; r++)
		{
			create_part(-1, x+r, y-2, playerp[2]);
			create_part(-1, x+r+1, y+2, playerp[2]);
			create_part(-1, x-2, y+r+1, playerp[2]);
			create_part(-1, x+2, y+r, playerp[2]);
		}
		kill_part(i);  //Kill him
		return 1;
	}

	//Follow gravity
	float gvx, gvy;
	gvx = gvy = 0.0f;
	switch (gravityMode)
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
	}

	gvx += gravx[(int)parts[i].y/CELL][(int)parts[i].x/CELL];
	gvy += gravy[(int)parts[i].y/CELL][(int)parts[i].x/CELL];

	parts[i].vx -= gvx*dt;  //Head up!
	parts[i].vy -= gvy*dt;

	//Verlet integration
	pp = 2*playerp[3]-playerp[5]+playerp[19]*dt*dt;
	playerp[5] = playerp[3];
	playerp[3] = pp;
	pp = 2*playerp[4]-playerp[6]+playerp[20]*dt*dt;
	playerp[6] = playerp[4];
	playerp[4] = pp;

	pp = 2*playerp[7]-playerp[9]+(playerp[21]+gvx)*dt*dt;
	playerp[9] = playerp[7];
	playerp[7] = pp;
	pp = 2*playerp[8]-playerp[10]+(playerp[22]+gvy)*dt*dt;
	playerp[10] = playerp[8];
	playerp[8] = pp;

	pp = 2*playerp[11]-playerp[13]+playerp[23]*dt*dt;
	playerp[13] = playerp[11];
	playerp[11] = pp;
	pp = 2*playerp[12]-playerp[14]+playerp[24]*dt*dt;
	playerp[14] = playerp[12];
	playerp[12] = pp;

	pp = 2*playerp[15]-playerp[17]+(playerp[25]+gvx)*dt*dt;
	playerp[17] = playerp[15];
	playerp[15] = pp;
	pp = 2*playerp[16]-playerp[18]+(playerp[26]+gvy)*dt*dt;
	playerp[18] = playerp[16];
	playerp[16] = pp;

	//Setting acceleration to 0
	playerp[19] = 0;
	playerp[20] = 0;

	playerp[21] = 0;
	playerp[22] = 0;

	playerp[23] = 0;
	playerp[24] = 0;

	playerp[25] = 0;
	playerp[26] = 0;

	float gx, gy, dl, dr;

	gx = (playerp[7] + playerp[15])/2 - gvy;
	gy = (playerp[8] + playerp[16])/2 + gvx;
	dl = pow(gx - playerp[7], 2) + pow(gy - playerp[8], 2);
	dr = pow(gx - playerp[15], 2) + pow(gy - playerp[16], 2);
	
	//Go left
	if (((int)(playerp[0])&0x01) == 0x01)
	{
		if (dl>dr)
		{
			if (!eval_move(PT_DUST, playerp[7], playerp[8], NULL))
			{
				playerp[21] = -3*gvy-3*gvx;
				playerp[22] = 3*gvx-3*gvy;
				playerp[19] = -gvy;
				playerp[20] = gvx;
			}
		}
		else
		{
			if (!eval_move(PT_DUST, playerp[15], playerp[16], NULL))
			{
				playerp[25] = -3*gvy-3*gvx;
				playerp[26] = 3*gvx-3*gvy;
				playerp[19] = -gvy;
				playerp[20] = gvx;
			}
		}
	}

	//Go right
	if (((int)(playerp[0])&0x02) == 0x02)
	{
		if (dl<dr)
		{
			if (!eval_move(PT_DUST, playerp[7], playerp[8], NULL))
			{
				playerp[21] = 3*gvy-3*gvx;
				playerp[22] = -3*gvx-3*gvy;
				playerp[19] = gvy;
				playerp[20] = -gvx;
			}
		}
		else
		{
			if (!eval_move(PT_DUST, playerp[15], playerp[16], NULL))
			{
				playerp[25] = 3*gvy-3*gvx;
				playerp[26] = -3*gvx-3*gvy;
				playerp[19] = gvy;
				playerp[20] = -gvx;
			}
		}
	}

	//Jump
	if (((int)(playerp[0])&0x04) == 0x04 && 
			(!eval_move(PT_DUST, playerp[7], playerp[8], NULL) || !eval_move(PT_DUST, playerp[15], playerp[16], NULL)))
	{
		parts[i].vy -= 4*gvy;
		playerp[22] -= gvy;
		playerp[26] -= gvy;
	}

	//Charge detector wall if foot inside
	if (bmap[(int)(playerp[8]+0.5)/CELL][(int)(playerp[7]+0.5)/CELL]==WL_DETECT)
		set_emap((int)playerp[7]/CELL, (int)playerp[8]/CELL);
	if (bmap[(int)(playerp[16]+0.5)/CELL][(int)(playerp[15]+0.5)/CELL]==WL_DETECT)
		set_emap((int)(playerp[15]+0.5)/CELL, (int)(playerp[16]+0.5)/CELL);

	//Searching for particles near head
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r || (r>>8)>=NPART)
					r = photons[y+ry][x+rx];

				if ((!r || (r>>8)>=NPART) && !bmap[(y+ry)/CELL][(x+rx)/CELL])
					continue;
				
				if (ptypes[r&0xFF].falldown!=0 || (r&0xFF) == PT_NEUT || (r&0xFF) == PT_PHOT)
				{
					playerp[2] = r&0xFF;  //Current element
				}
				if ((r&0xFF) == PT_PLNT && parts[i].life<100) //Plant gives him 5 HP
				{
					if (parts[i].life<=95)
						parts[i].life += 5;
					else
						parts[i].life = 100;
					kill_part(r>>8);
				}

				if ((r&0xFF) == PT_NEUT)
				{
					if (parts[i].life<=100) parts[i].life -= (102-parts[i].life)/2;
					else parts[i].life *= 0.9f;
					kill_part(r>>8);
				}
				if (bmap[(ry+y)/CELL][(rx+x)/CELL]==WL_FAN)
					playerp[2] = SPC_AIR;
				if ((r&0xFF)==PT_PRTI)
					STKM_interact(playerp, i, rx, ry);
				if (!parts[i].type)//STKM_interact may kill STKM
					return 1;
			}

	//Head position
	rx = x + 3*((((int)playerp[1])&0x02) == 0x02) - 3*((((int)playerp[1])&0x01) == 0x01);
	ry = y - 3*(playerp[1] == 0);

	//Spawn
	if (((int)(playerp[0])&0x08) == 0x08)
	{
		ry -= 2*(rand()%2)+1;
		r = pmap[ry][rx];
		if (!((r>>8)>=NPART))
		{
			if (ptypes[r&0xFF].state == ST_SOLID)
			{
				create_part(-1, rx, ry, PT_SPRK);
			}
			else
			{
				int np = -1;
				if (playerp[2] == SPC_AIR)
					create_parts(rx + 3*((((int)playerp[1])&0x02) == 0x02) - 3*((((int)playerp[1])&0x01) == 0x01), ry, 4, 4, SPC_AIR, 0);
				else
					np = create_part(-1, rx, ry, playerp[2]);
				if ( (np < NPART) && np>=0 && playerp[2] != PT_PHOT && playerp[2] != SPC_AIR)
				{
					parts[np].vx -= -gvy*(5*((((int)playerp[1])&0x02) == 0x02) - 5*(((int)(playerp[1])&0x01) == 0x01));
					parts[np].vy -= gvx*(5*((((int)playerp[1])&0x02) == 0x02) - 5*(((int)(playerp[1])&0x01) == 0x01));
					parts[i].vx -= (ptypes[(int)playerp[2]].weight*parts[np].vx)/1000;
				}
				if ((np < NPART) && np>=0 && playerp[2] == PT_PHOT)
				{
					int random = abs(rand()%3-1)*3;
					if (random==0)
					{
						kill_part(np);
					}
					else
					{
						parts[np].vy = 0;
						if (((int)playerp[1])&(0x01|0x02))
							parts[np].vx = (((((int)playerp[1])&0x02) == 0x02) - (((int)(playerp[1])&0x01) == 0x01))*random;
						else
							parts[np].vx = random;
					}
				}

			}
		}
	}

	//Simulation of joints
	d = 25/(pow((playerp[3]-playerp[7]), 2) + pow((playerp[4]-playerp[8]), 2)+25) - 0.5;  //Fast distance
	playerp[7] -= (playerp[3]-playerp[7])*d;
	playerp[8] -= (playerp[4]-playerp[8])*d;
	playerp[3] += (playerp[3]-playerp[7])*d;
	playerp[4] += (playerp[4]-playerp[8])*d;

	d = 25/(pow((playerp[11]-playerp[15]), 2) + pow((playerp[12]-playerp[16]), 2)+25) - 0.5;
	playerp[15] -= (playerp[11]-playerp[15])*d;
	playerp[16] -= (playerp[12]-playerp[16])*d;
	playerp[11] += (playerp[11]-playerp[15])*d;
	playerp[12] += (playerp[12]-playerp[16])*d;

	d = 36/(pow((playerp[3]-parts[i].x), 2) + pow((playerp[4]-parts[i].y), 2)+36) - 0.5;
	parts[i].vx -= (playerp[3]-parts[i].x)*d;
	parts[i].vy -= (playerp[4]-parts[i].y)*d;
	playerp[3] += (playerp[3]-parts[i].x)*d;
	playerp[4] += (playerp[4]-parts[i].y)*d;

	d = 36/(pow((playerp[11]-parts[i].x), 2) + pow((playerp[12]-parts[i].y), 2)+36) - 0.5;
	parts[i].vx -= (playerp[11]-parts[i].x)*d;
	parts[i].vy -= (playerp[12]-parts[i].y)*d;
	playerp[11] += (playerp[11]-parts[i].x)*d;
	playerp[12] += (playerp[12]-parts[i].y)*d;

	if (!eval_move(PT_DUST, playerp[7], playerp[8], NULL))
	{
		playerp[7] = playerp[9];
		playerp[8] = playerp[10];
	}

	if (!eval_move(PT_DUST, playerp[15], playerp[16], NULL))
	{
		playerp[15] = playerp[17];
		playerp[16] = playerp[18];
	}

	//This makes stick man "pop" from obstacles
	if (!eval_move(PT_DUST, playerp[7], playerp[8], NULL))
	{
		float t;
		t = playerp[7]; playerp[7] = playerp[9]; playerp[9] = t;
		t = playerp[8]; playerp[8] = playerp[10]; playerp[10] = t;
	}

	if (!eval_move(PT_DUST, playerp[15], playerp[16], NULL))
	{
		float t;
		t = playerp[15]; playerp[15] = playerp[17]; playerp[17] = t;
		t = playerp[16]; playerp[16] = playerp[18]; playerp[18] = t;
	}

	//Keeping legs distance
	if ((pow((playerp[7] - playerp[15]), 2) + pow((playerp[8]-playerp[16]), 2))<16)
	{
		float tvx, tvy;
		tvx = -gvy;
		tvy = gvx;

		if (tvx || tvy)
		{
			playerp[21] -= 0.2*tvx/hypot(tvx, tvy);
			playerp[22] -= 0.2*tvy/hypot(tvx, tvy);

			playerp[25] += 0.2*tvx/hypot(tvx, tvy);
			playerp[26] += 0.2*tvy/hypot(tvx, tvy);
		}
	}

	if ((pow((playerp[3] - playerp[11]), 2) + pow((playerp[4]-playerp[12]), 2))<16)
	{
		float tvx, tvy;
		tvx = -gvy;
		tvy = gvx;

		if (tvx || tvy)
		{
			playerp[19] -= 0.2*tvx/hypot(tvx, tvy);
			playerp[20] -= 0.2*tvy/hypot(tvx, tvy);

			playerp[23] += 0.2*tvx/hypot(tvx, tvy);
			playerp[24] += 0.2*tvy/hypot(tvx, tvy);
		}
	}

	//If legs touch something
	STKM_interact(playerp, i, (int)(playerp[7]+0.5), (int)(playerp[8]+0.5));
	STKM_interact(playerp, i, (int)(playerp[15]+0.5), (int)(playerp[16]+0.5));
	if (!parts[i].type)
		return 1;

	parts[i].ctype = playerp[2];
	return 0;
}

void STKM_interact(float* playerp, int i, int x, int y)
{
	int r;
	if (x<0 || y<0 || x>=XRES || y>=YRES || !parts[i].type)
		return;
	r = pmap[y][x];
	if (r && (r>>8)<NPART)
	{
		if ((r&0xFF)==PT_SPRK && r && (r>>8)<NPART) //If on charge
		{
			parts[i].life -= (int)(rand()*20/RAND_MAX)+32;
		}

		if (ptypes[r&0xFF].hconduct && (parts[r>>8].temp>=323 || parts[r>>8].temp<=243))
		{
			parts[i].life -= 2;
			playerp[22] -= 1;
		}

		if ((r&0xFF)==PT_ACID)  //If on acid
			parts[i].life -= 5;

		if ((r&0xFF)==PT_PLUT)  //If on plut
			parts[i].life -= 1;
			
		if (ptypes[r&0xFF].properties&PROP_DEADLY)
			parts[i].life -= 1;

		if ((r&0xFF)==PT_PRTI && parts[i].type)
		{
			int nnx, count=1;//gives rx=0, ry=1 in update_PRTO
			parts[r>>8].tmp = (int)((parts[r>>8].temp-73.15f)/100+1);
			if (parts[r>>8].tmp>=CHANNELS) parts[r>>8].tmp = CHANNELS-1;
			else if (parts[r>>8].tmp<0) parts[r>>8].tmp = 0;
			for (nnx=0; nnx<80; nnx++)
				if (!portalp[parts[r>>8].tmp][count][nnx].type)
				{
					portalp[parts[r>>8].tmp][count][nnx] = parts[i];
					kill_part(i);
					playerp[27] = 1;//stop SPWN creating a new STKM while he is in portal
				}
		}
	}
}

void STKM_init_legs(float* playerp, int i)
{
	int x, y;

	x = (int)(parts[i].x+0.5f);
	y = (int)(parts[i].y+0.5f);

	playerp[3] = x-1;
	playerp[4] = y+6;
	playerp[5] = x-1;
	playerp[6] = y+6;

	playerp[7] = x-3;
	playerp[8] = y+12;
	playerp[9] = x-3;
	playerp[10] = y+12;

	playerp[11] = x+1;
	playerp[12] = y+6;
	playerp[13] = x+1;
	playerp[14] = y+6;

	playerp[15] = x+3;
	playerp[16] = y+12;
	playerp[17] = x+3;
	playerp[18] = y+12;
}
