#include <element.h>

int update_SPAWN2(UPDATE_FUNC_ARGS) {
	ISSPAWN2 = 1;
	if (death2)
	{
		player2spawn = create_part(-1,x,y,PT_STKM2);
		isplayer2 = 1;
		death2 = 0;
	}
	return 0;
}

int update_STKM2(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	float pp, d;
	float dt = 0.9;///(FPSB*FPSB);  //Delta time in square

	if ((parts[i].ctype>0 && parts[i].ctype<PT_NUM && ptypes[parts[i].ctype].falldown>0) || parts[i].ctype==SPC_AIR || parts[i].ctype == PT_NEUT || parts[i].ctype == PT_PHOT)
		player2[2] = parts[i].ctype;

	//Tempirature handling
	if (parts[i].temp<243)
		parts[i].life -= 1;
	if ((parts[i].temp<309.6f) && (parts[i].temp>=243))
		parts[i].temp += 1;

	if (isplayer2) { //Already a stickman2 in the simulation
		death2 = 1;
		parts[i].type = PT_NONE;
	}

	//Death
	if (parts[i].life<1 || death2 || (pv[y/CELL][x/CELL]>=4.5f && player2[2] != SPC_AIR) ) //If his HP is less that 0 or there is very big wind...
	{
		for (r=-2; r<=1; r++)
		{
			create_part(-1, x+r, y-2, player2[2]);
			create_part(-1, x+r+1, y+2, player2[2]);
			create_part(-1, x-2, y+r+1, player2[2]);
			create_part(-1, x+2, y+r, player2[2]);
		}
		kill_part(i);  //Kill him
		return 1;
	}

	parts[i].vy += -0.7*dt;  //Head up!

	//Verlet integration
	pp = 2*player2[3]-player2[5]+player2[19]*dt*dt;;
	player2[5] = player2[3];
	player2[3] = pp;
	pp = 2*player2[4]-player2[6]+player2[20]*dt*dt;;
	player2[6] = player2[4];
	player2[4] = pp;

	pp = 2*player2[7]-player2[9]+player2[21]*dt*dt;;
	player2[9] = player2[7];
	player2[7] = pp;
	pp = 2*player2[8]-player2[10]+(player2[22]+1)*dt*dt;;
	player2[10] = player2[8];
	player2[8] = pp;

	pp = 2*player2[11]-player2[13]+player2[23]*dt*dt;;
	player2[13] = player2[11];
	player2[11] = pp;
	pp = 2*player2[12]-player2[14]+player2[24]*dt*dt;;
	player2[14] = player2[12];
	player2[12] = pp;

	pp = 2*player2[15]-player2[17]+player2[25]*dt*dt;;
	player2[17] = player2[15];
	player2[15] = pp;
	pp = 2*player2[16]-player2[18]+(player2[26]+1)*dt*dt;;
	player2[18] = player2[16];
	player2[16] = pp;

	//Setting acceleration to 0
	player2[19] = 0;
	player2[20] = 0;

	player2[21] = 0;
	player2[22] = 0;

	player2[23] = 0;
	player2[24] = 0;

	player2[25] = 0;
	player2[26] = 0;

	//Go left
	r = pmap[(int)(parts[i].y+10)][(int)(parts[i].x)];
	if (((int)(player2[0])&0x01) == 0x01 && (((r&0xFF)>=PT_NUM) || ptypes[r&0xFF].state != ST_GAS))
	{
		if (r>=PT_NUM || (ptypes[r&0xFF].state != ST_LIQUID
			&& (r&0xFF) != PT_LNTG))
		{
			if (pmap[(int)(player2[8]-1)][(int)(player2[7])])
			{
				player2[21] = -3;
				player2[22] = -2;
				player2[19] = -2;
			}

			if (pmap[(int)(player2[16]-1)][(int)(player2[15])])
			{
				player2[25] = -3;
				player2[26] = -2;
				player2[23] = -2;
			}
		}
		else
		{
			if (pmap[(int)(player2[8]-1)][(int)(player2[7])])  //It should move another way in liquids
			{
				player2[21] = -1;
				player2[22] = -1;
				player2[19] = -1;
			}

			if (pmap[(int)(player2[16]-1)][(int)(player2[15])])
			{
				player2[25] = -1;
				player2[26] = -1;
				player2[23] = -1;
			}
		}
	}

	//Go right
	r = pmap[(int)(parts[i].y+10)][(int)(parts[i].x)];
	if (((int)(player2[0])&0x02) == 0x02 && (((r&0xFF)>=PT_NUM) || ptypes[r&0xFF].state != ST_GAS))
	{
		if (r>=PT_NUM || (ptypes[r&0xFF].state != ST_LIQUID
			&& (r&0xFF) != PT_LNTG))
		{
			if (pmap[(int)(player2[8]-1)][(int)(player2[7])])
			{
				player2[21] = 3;
				player2[22] = -2;
				player2[19] = 2;
			}

			if (pmap[(int)(player2[16]-1)][(int)(player2[15])])
			{
				player2[25] = 3;
				player2[26] = -2;
				player2[23] = 2;
			}
		}
		else
		{
			if (pmap[(int)(player2[8]-1)][(int)(player2[7])])
			{
				player2[21] = 1;
				player2[22] = -1;
				player2[19] = 1;
			}

			if (pmap[(int)(player2[16]-1)][(int)(player2[15])])
			{
				player2[25] = 1;
				player2[26] = -1;
				player2[23] = 1;
			}

		}
	}

	//Jump
	if (((int)(player2[0])&0x04) == 0x04 && (
		(pmap[(int)(player2[8]-0.5)][(int)(player2[7])]&0xFF)>=PT_NUM ||
		ptypes[pmap[(int)(player2[8]-0.5)][(int)(player2[7])]&0xFF].state != ST_GAS ||
		(pmap[(int)(player2[16]-0.5)][(int)(player2[15])]&0xFF)>=PT_NUM ||
		ptypes[pmap[(int)(player2[16]-0.5)][(int)(player2[15])]&0xFF].state != ST_GAS))
	{
		if (pmap[(int)(player2[8]-0.5)][(int)(player2[7])] || pmap[(int)(player2[16]-0.5)][(int)(player2[15])])
		{
			parts[i].vy = -5;
			player2[22] -= 1;
			player2[26] -= 1;
		}
	}

	//Charge detector wall if foot inside
	if (bmap[(int)(player2[8]+0.5)/CELL][(int)(player2[7]+0.5)/CELL]==WL_DETECT)
		set_emap((int)player2[7]/CELL, (int)player2[8]/CELL);
	if (bmap[(int)(player2[16]+0.5)/CELL][(int)(player2[15]+0.5)/CELL]==WL_DETECT)
		set_emap((int)(player2[15]+0.5)/CELL, (int)(player2[16]+0.5)/CELL);

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
					player2[2] = r&0xFF;  //Current element
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
					player2[2] = SPC_AIR;
			}

	//Head position
	rx = x + 3*((((int)player2[1])&0x02) == 0x02) - 3*((((int)player2[1])&0x01) == 0x01);
	ry = y - 3*(player2[1] == 0);

	//Spawn
	if (((int)(player2[0])&0x08) == 0x08)
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
				if (player2[2] == SPC_AIR)
					create_parts(rx + 3*((((int)player2[1])&0x02) == 0x02) - 3*((((int)player2[1])&0x01) == 0x01), ry, 4, 4, SPC_AIR);
				else
					np = create_part(-1, rx, ry, player2[2]);
				if ((np < NPART) && np>=0 && player2[2] != PT_PHOT && player2[2] != SPC_AIR)
					parts[np].vx = parts[np].vx + 5*((((int)player2[1])&0x02) == 0x02) - 5*(((int)(player2[1])&0x01) == 0x01);
				if ((np < NPART) && np>=0 && player2[2] == PT_PHOT)
				{
					int random = abs(rand()%3-1)*3;
					if (random==0)
					{
						kill_part(np);
					}
					else
					{
						parts[np].vy = 0;
						if (((int)player2[1])&(0x01|0x02))
							parts[np].vx = (((((int)player2[1])&0x02) == 0x02) - (((int)(player2[1])&0x01) == 0x01))*random;
						else
							parts[np].vx = random;
					}
				}

			}
		}
	}

	//Simulation of joints
	d = 25/(pow((player2[3]-player2[7]), 2) + pow((player2[4]-player2[8]), 2)+25) - 0.5;  //Fast distance
	player2[7] -= (player2[3]-player2[7])*d;
	player2[8] -= (player2[4]-player2[8])*d;
	player2[3] += (player2[3]-player2[7])*d;
	player2[4] += (player2[4]-player2[8])*d;

	d = 25/(pow((player2[11]-player2[15]), 2) + pow((player2[12]-player2[16]), 2)+25) - 0.5;
	player2[15] -= (player2[11]-player2[15])*d;
	player2[16] -= (player2[12]-player2[16])*d;
	player2[11] += (player2[11]-player2[15])*d;
	player2[12] += (player2[12]-player2[16])*d;

	d = 36/(pow((player2[3]-parts[i].x), 2) + pow((player2[4]-parts[i].y), 2)+36) - 0.5;
	parts[i].vx -= (player2[3]-parts[i].x)*d;
	parts[i].vy -= (player2[4]-parts[i].y)*d;
	player2[3] += (player2[3]-parts[i].x)*d;
	player2[4] += (player2[4]-parts[i].y)*d;

	d = 36/(pow((player2[11]-parts[i].x), 2) + pow((player2[12]-parts[i].y), 2)+36) - 0.5;
	parts[i].vx -= (player2[11]-parts[i].x)*d;
	parts[i].vy -= (player2[12]-parts[i].y)*d;
	player2[11] += (player2[11]-parts[i].x)*d;
	player2[12] += (player2[12]-parts[i].y)*d;

	//Side collisions checking
	for (rx = -3; rx <= 3; rx++)
	{
		r = pmap[(int)(player2[16]-2)][(int)(player2[15]+rx)];
		if (r && ((r&0xFF)>=PT_NUM || (ptypes[r&0xFF].state != ST_GAS && ptypes[r&0xFF].state != ST_LIQUID)))
			player2[15] -= rx;

		r = pmap[(int)(player2[8]-2)][(int)(player2[7]+rx)];
		if (r && ((r&0xFF)>=PT_NUM || (ptypes[r&0xFF].state != ST_GAS && ptypes[r&0xFF].state != ST_LIQUID)))
			player2[7] -= rx;
	}

	//Collision checks
	for (ry = -2-(int)parts[i].vy; ry<=0; ry++)
	{
		r = pmap[(int)(player2[8]+ry)][(int)(player2[7]+0.5)];  //This is to make coding more pleasant :-)

		//For left leg
		if (r && (r&0xFF)!=PT_STKM2)
		{
			if ((r&0xFF)<PT_NUM && (ptypes[r&0xFF].state == ST_LIQUID || (r&0xFF) == PT_LNTG)) //Liquid checks
			{
				if (parts[i].y<(player2[8]-10))
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
					player2[8] += ry-1;
					parts[i].vy -= 0.5*parts[i].vy*dt;
				}
			}
			player2[9] = player2[7];
		}

		r = pmap[(int)(player2[16]+ry)][(int)(player2[15]+0.5)];

		//For right leg
		if (r && (r&0xFF)!=PT_STKM2)
		{
			if ((r&0xFF)<PT_NUM && (ptypes[r&0xFF].state == ST_LIQUID || (r&0xFF) == PT_LNTG))
			{
				if (parts[i].y<(player2[16]-10))
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
					player2[16] += ry-1;
					parts[i].vy -= 0.5*parts[i].vy*dt;
				}
			}
			player2[17] = player2[15];
		}

		//If it falls too fast
		if (parts[i].vy>=30)
		{
			parts[i].y -= (10+ry)*dt;
			parts[i].vy = -10*dt;
		}

	}

	//Keeping legs distance
	if (pow((player2[7] - player2[15]), 2)<16 && pow((player2[8]-player2[16]), 2)<1)
	{
		player2[21] -= 0.2;
		player2[25] += 0.2;
	}

	if (pow((player2[3] - player2[11]), 2)<16 && pow((player2[4]-player2[12]), 2)<1)
	{
		player2[19] -= 0.2;
		player2[23] += 0.2;
	}

	//If legs touch something
	r = pmap[(int)(player2[8]+0.5)][(int)(player2[7]+0.5)];
	if ((r&0xFF)==PT_SPRK && r && (r>>8)<NPART) //If on charge
	{
		parts[i].life -= (int)(rand()/1000)+38;
	}

	if (r>0 && (r>>8)<NPART)
	{
		if (parts[r>>8].temp>=323 || parts[r>>8].temp<=243) //If hot or cold
		{
			parts[i].life -= 2;
			player2[26] -= 1;
		}
	}

	if ((r&0xFF)==PT_ACID)  //If on acid
		parts[i].life -= 5;

	if ((r&0xFF)==PT_PLUT)  //If on plut
		parts[i].life -= 1;

	r = pmap[(int)(player2[16]+0.5)][(int)(player2[15]+0.5)];
	if ((r&0xFF)==PT_SPRK && r && (r>>8)<NPART) //If on charge
	{
		parts[i].life -= (int)(rand()/1000)+38;
	}

	if (r>0 && (r>>8)<NPART) //If hot or cold
	{
		if (parts[r>>8].temp>=323 || parts[r>>8].temp<=243)
		{
			parts[i].life -= 2;
			player2[22] -= 1;
		}
	}

	if ((r&0xFF)==PT_ACID)  //If on acid
		parts[i].life -= 5;

	if ((r&0xFF)==PT_PLUT)  //If on plut
		parts[i].life -= 1;

	isplayer2 = 1;
	parts[i].ctype = player2[2];
	return 0;
}

