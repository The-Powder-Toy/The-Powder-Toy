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

	parts[i].vy += -0.7*dt;  //Head up!

	//Verlet integration
	pp = 2*playerp[3]-playerp[5]+playerp[19]*dt*dt;;
	playerp[5] = playerp[3];
	playerp[3] = pp;
	pp = 2*playerp[4]-playerp[6]+playerp[20]*dt*dt;;
	playerp[6] = playerp[4];
	playerp[4] = pp;

	pp = 2*playerp[7]-playerp[9]+playerp[21]*dt*dt;;
	playerp[9] = playerp[7];
	playerp[7] = pp;
	pp = 2*playerp[8]-playerp[10]+(playerp[22]+1)*dt*dt;;
	playerp[10] = playerp[8];
	playerp[8] = pp;

	pp = 2*playerp[11]-playerp[13]+playerp[23]*dt*dt;;
	playerp[13] = playerp[11];
	playerp[11] = pp;
	pp = 2*playerp[12]-playerp[14]+playerp[24]*dt*dt;;
	playerp[14] = playerp[12];
	playerp[12] = pp;

	pp = 2*playerp[15]-playerp[17]+playerp[25]*dt*dt;;
	playerp[17] = playerp[15];
	playerp[15] = pp;
	pp = 2*playerp[16]-playerp[18]+(playerp[26]+1)*dt*dt;;
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

	//Go left
	r = pmap[(int)(parts[i].y+10)][(int)(parts[i].x)];
	if (((int)(playerp[0])&0x01) == 0x01 && (((r&0xFF)>=PT_NUM) || ptypes[r&0xFF].state != ST_GAS))
	{
		if (r>=PT_NUM || (ptypes[r&0xFF].state != ST_LIQUID
		        && (r&0xFF) != PT_LNTG))
		{
			if (pmap[(int)(playerp[8]-1)][(int)(playerp[7])])
			{
				playerp[21] = -3;
				playerp[22] = -2;
				playerp[19] = -2;
			}

			if (pmap[(int)(playerp[16]-1)][(int)(playerp[15])])
			{
				playerp[25] = -3;
				playerp[26] = -2;
				playerp[23] = -2;
			}
		}
		else
		{
			if (pmap[(int)(playerp[8]-1)][(int)(playerp[7])])  //It should move another way in liquids
			{
				playerp[21] = -1;
				playerp[22] = -1;
				playerp[19] = -1;
			}

			if (pmap[(int)(playerp[16]-1)][(int)(playerp[15])])
			{
				playerp[25] = -1;
				playerp[26] = -1;
				playerp[23] = -1;
			}
		}
	}

	//Go right
	r = pmap[(int)(parts[i].y+10)][(int)(parts[i].x)];
	if (((int)(playerp[0])&0x02) == 0x02 && (((r&0xFF)>=PT_NUM) || ptypes[r&0xFF].state != ST_GAS))
	{
		if (r>=PT_NUM || (ptypes[r&0xFF].state != ST_LIQUID
		        && (r&0xFF) != PT_LNTG))
		{
			if (pmap[(int)(playerp[8]-1)][(int)(playerp[7])])
			{
				playerp[21] = 3;
				playerp[22] = -2;
				playerp[19] = 2;
			}

			if (pmap[(int)(playerp[16]-1)][(int)(playerp[15])])
			{
				playerp[25] = 3;
				playerp[26] = -2;
				playerp[23] = 2;
			}
		}
		else
		{
			if (pmap[(int)(playerp[8]-1)][(int)(playerp[7])])
			{
				playerp[21] = 1;
				playerp[22] = -1;
				playerp[19] = 1;
			}

			if (pmap[(int)(playerp[16]-1)][(int)(playerp[15])])
			{
				playerp[25] = 1;
				playerp[26] = -1;
				playerp[23] = 1;
			}

		}
	}

	//Jump
	if (((int)(playerp[0])&0x04) == 0x04 && (
		(pmap[(int)(playerp[8]-0.5)][(int)(playerp[7])]&0xFF)>=PT_NUM ||
		ptypes[pmap[(int)(playerp[8]-0.5)][(int)(playerp[7])]&0xFF].state != ST_GAS ||
		(pmap[(int)(playerp[16]-0.5)][(int)(playerp[15])]&0xFF)>=PT_NUM ||
		ptypes[pmap[(int)(playerp[16]-0.5)][(int)(playerp[15])]&0xFF].state != ST_GAS))
	{
		if (pmap[(int)(playerp[8]-0.5)][(int)(playerp[7])] || pmap[(int)(playerp[16]-0.5)][(int)(playerp[15])])
		{
			parts[i].vy = -5;
			playerp[22] -= 1;
			playerp[26] -= 1;
		}
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
					create_parts(rx + 3*((((int)playerp[1])&0x02) == 0x02) - 3*((((int)playerp[1])&0x01) == 0x01), ry, 4, 4, SPC_AIR);
				else
					np = create_part(-1, rx, ry, playerp[2]);
				if ( (np < NPART) && np>=0 && playerp[2] != PT_PHOT && playerp[2] != SPC_AIR)
					parts[np].vx = parts[np].vx + 5*((((int)playerp[1])&0x02) == 0x02) - 5*(((int)(playerp[1])&0x01) == 0x01);
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

	//Side collisions checking
	for (rx = -3; rx <= 3; rx++)
	{
		r = pmap[(int)(playerp[16]-2)][(int)(playerp[15]+rx)];
		if (r && ((r&0xFF)>=PT_NUM || (ptypes[r&0xFF].state != ST_GAS && ptypes[r&0xFF].state != ST_LIQUID)))
			playerp[15] -= rx;

		r = pmap[(int)(playerp[8]-2)][(int)(playerp[7]+rx)];
		if (r && ((r&0xFF)>=PT_NUM || (ptypes[r&0xFF].state != ST_GAS && ptypes[r&0xFF].state != ST_LIQUID)))
			playerp[7] -= rx;
	}

	//Collision checks
	for (ry = -2-(int)parts[i].vy; ry<=0; ry++)
	{
		r = pmap[(int)(playerp[8]+ry)][(int)(playerp[7]+0.5)];  //This is to make coding more pleasant :-)

		//For left leg
		if (r && (r&0xFF)!=PT_STKM)
		{
			if ((r&0xFF)<PT_NUM && (ptypes[r&0xFF].state == ST_LIQUID || (r&0xFF) == PT_LNTG)) //Liquid checks
			{
				if (parts[i].y<(playerp[8]-10))
					parts[i].vy = 1*dt;
				else
					parts[i].vy = 0;
				if (abs(parts[i].vx)>1)
					parts[i].vx *= 0.5*dt;
			}
			else
			{
				if ((r&0xFF)>=PT_NUM || ptypes[r&0xFF].state != ST_GAS)
				{
					playerp[8] += ry-1;
					parts[i].vy -= 0.5*parts[i].vy*dt;
				}
			}
			playerp[9] = playerp[7];
		}

		r = pmap[(int)(playerp[16]+ry)][(int)(playerp[15]+0.5)];

		//For right leg
		if (r && (r&0xFF)!=PT_STKM)
		{
			if ((r&0xFF)<PT_NUM && (ptypes[r&0xFF].state == ST_LIQUID || (r&0xFF) == PT_LNTG))
			{
				if (parts[i].y<(playerp[16]-10))
					parts[i].vy = 1*dt;
				else
					parts[i].vy = 0;
				if (abs(parts[i].vx)>1)
					parts[i].vx *= 0.5*dt;
			}
			else
			{
				if ((r&0xFF)>=PT_NUM || ptypes[r&0xFF].state != ST_GAS)
				{
					playerp[16] += ry-1;
					parts[i].vy -= 0.5*parts[i].vy*dt;
				}
			}
			playerp[17] = playerp[15];
		}

		//If it falls too fast
		if (parts[i].vy>=30)
		{
			parts[i].y -= (10+ry)*dt;
			parts[i].vy = -10*dt;
		}

	}

	//Keeping legs distance
	if (pow((playerp[7] - playerp[15]), 2)<16 && pow((playerp[8]-playerp[16]), 2)<1)
	{
		playerp[21] -= 0.2;
		playerp[25] += 0.2;
	}

	if (pow((playerp[3] - playerp[11]), 2)<16 && pow((playerp[4]-playerp[12]), 2)<1)
	{
		playerp[19] -= 0.2;
		playerp[23] += 0.2;
	}

	//If legs touch something
	r = pmap[(int)(playerp[8]+0.5)][(int)(playerp[7]+0.5)];
	if ((r&0xFF)==PT_SPRK && r && (r>>8)<NPART) //If on charge
	{
		parts[i].life -= (int)(rand()/1000)+38;
	}

	if (r>0 && (r>>8)<NPART)
	{
		if (parts[r>>8].temp>=323 || parts[r>>8].temp<=243) //If hot or cold
		{
			parts[i].life -= 2;
			playerp[26] -= 1;
		}
	}

	if ((r&0xFF)==PT_ACID)  //If on acid
		parts[i].life -= 5;

	if ((r&0xFF)==PT_PLUT)  //If on plut
		parts[i].life -= 1;

	r = pmap[(int)(playerp[16]+0.5)][(int)(playerp[15]+0.5)];
	if ((r&0xFF)==PT_SPRK && r && (r>>8)<NPART) //If on charge
	{
		parts[i].life -= (int)(rand()/1000)+38;
	}

	if (r>0 && (r>>8)<NPART) //If hot or cold
	{
		if (parts[r>>8].temp>=323 || parts[r>>8].temp<=243)
		{
			parts[i].life -= 2;
			playerp[22] -= 1;
		}
	}

	if ((r&0xFF)==PT_ACID)  //If on acid
		parts[i].life -= 5;

	if ((r&0xFF)==PT_PLUT)  //If on plut
		parts[i].life -= 1;

	parts[i].ctype = playerp[2];
	return 0;
}
