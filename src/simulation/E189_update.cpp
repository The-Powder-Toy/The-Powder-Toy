#include "simulation/Elements.h"
#include "simulation/Air.h"
#include "simulation/E189_update.h"

E189_Update::E189_Update()
{
	// null constructor
}

int E189_Update::update(UPDATE_FUNC_ARGS)
{
	static int tron_rx[4] = {-1, 0, 1, 0};
	static int tron_ry[4] = { 0,-1, 0, 1};
	static int osc_r1 [4] = { 1,-1, 2,-2};
	int rx, ry, ttan = 0, rlife = parts[i].life, direction, r, ri, rtmp, rctype;
	int rr, rndstore, trade, transfer, rt, rii, rrx, rry, nx, ny, pavg;
	float rvx, rvy, rdif;
	rtmp = parts[i].tmp;
	
	switch (rlife)
	{
	case 0: // acts like TTAN [压力绝缘体]
	case 1:
		if (nt<=2)
			ttan = 2;
		else if (rlife)
			ttan = 2;
		else if (nt<=6)
			for (rx=-1; rx<2; rx++) {
				for (ry=-1; ry<2; ry++) {
					if ((!rx != !ry) && BOUNDS_CHECK) {
						if((pmap[y+ry][x+rx]&0xFF)==PT_E189)
							ttan++;
					}
				}
			}
		break;
	case 2: // TRON input ["智能粒子" 的传送门入口]
		if (rtmp & 0x04)
			rtmp &= ~0x04;
		else if (rtmp & 0x01)
		{
			rr = (rtmp >> 5) & ((rtmp >> 19 & 1) - 1);
			direction = (rr + (rtmp >> 17)) & 0x3;
			rx = x + tron_rx[direction];
			ry = y + tron_ry[direction];
			r = pmap[ry][rx];
			rii = parts[r >> 8].life;
			rrx = rii >> 1;
			if ((r & 0xFF) == PT_E189 && (rrx == 1 || rrx == 15))
			{
				ri = r >> 8;
				if (rii == 31) // delay
				{
					if (parts[ri].tmp3)
						goto break1c;
					else
						parts[ri].tmp3 = parts[ri].ctype;
				}
				parts[ri].tmp &= (rii == 30 ? 0x700000 : 0) | 0xE0000;
				parts[ri].tmp |= (rtmp & 0x1FF9F) | (direction << 5);
				if (ri > i)
					sim->parts[ri].tmp |= 0x04;
				parts[ri].tmp2 = parts[i].tmp2;
			}
			else if ((r & 0xFF) == PT_METL || (r & 0xFF) == PT_PSCN || (r & 0xFF) == PT_NSCN)
			{
				conductTo (sim, r, rx, ry, parts);
			}
		break1c:
			rtmp &= 0xE0000;
		}
		parts[i].tmp = rtmp;
		break;
	case 3: // TRON output ["智能粒子" 的传送门出口]
		if (rtmp & 0x04)
			rtmp &= ~0x04;
		else if (rtmp & 0x01)
		{
			direction = (rtmp >> 5) & 0x3;
			ry = y + tron_ry[direction];
			rx = x + tron_rx[direction];
			r = pmap[ry][rx];
			if (r)
			{
				direction = (direction + (rand()%2) * 2 + 1) % 4;
				ry = y + tron_ry[direction];
				rx = x + tron_rx[direction];
				r = pmap[ry][rx];
				if (r)
				{
					direction = direction ^ 0x2; // bitwise xor
					ry = y + tron_ry[direction];
					rx = x + tron_rx[direction];
					r = pmap[ry][rx];
				}
				if (r)
				{
					parts[i].tmp = 0;
					break;
				}
			}
			if (!r)
			{
				ri = sim->create_part(-1, rx, ry, PT_TRON);
				if (ri >= 0)
				{
					parts[ri].life = 5;
					parts[ri].tmp  = rtmp & 0x1FF9F | (direction << 5);
					if (ri > i)
						parts[ri].tmp |= 0x04;
					parts[ri].tmp2 = parts[i].tmp2;
				}
			}
			rtmp = 0;
		}
		parts[i].tmp = rtmp;
		break;
	case 4: // photon laser [激光器]
		if (!rtmp)
			break;

		rvx = (float)(((rtmp ^ 0x08) & 0x0F) - 0x08);
		rvy = (float)((((rtmp >> 4) ^ 0x08) & 0x0F) - 0x08);
		rdif = (float)((((rtmp >> 8) ^ 0x80) & 0xFF) - 0x80);

		ri = sim->create_part(-3, x + (int)rvx, y + (int)rvy, PT_PHOT);
		if (ri < 0)
			break;
		if (ri > i)
			parts[ri].flags |= FLAG_SKIPMOVE;
		parts[ri].vx = rvx * rdif / 16.0f;
		parts[ri].vy = rvy * rdif / 16.0f;
		rctype = parts[i].ctype;
		rtmp = rctype & 0x3FFFFFFF;
		rctype >>= 30;
		if (rtmp)
			parts[ri].ctype = rtmp;
		parts[ri].temp = parts[i].temp;
		parts[ri].life = parts[i].tmp2;
		parts[ri].tmp = rctype & 3;
		
		break;
	case 5: // reserved for Simulation.cpp
	case 7: // reserved for Simulation.cpp
	case 13: // decoration only, no update function
	case 15: // reserved for Simulation.cpp
	case 17: // reserved for 186.cpp and Simulation.cpp
	case 18: // decoration only, no update function
	case 22: // reserved for Simulation.cpp
	case 23: // reserved for stickmans
	case 25: // reserved for E189's life = 16, ctype = 10.
	case 27: // reserved for stickmans
	case 32: // reserved for ARAY / BRAY
		break;
	case 6: // heater
		if (!sim->legacy_enable) //if heat sim is on
		{
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if ((!rx != !ry) && BOUNDS_CHECK)
					{
						r = pmap[y+ry][x+rx];
						if (!r)
							continue;
						if (sim->elements[r&0xFF].HeatConduct > 0)
							parts[r>>8].temp = parts[i].temp;
					}
		}
		break;
	case 8: // acts like VIBR [振金]
		rr = parts[i].tmp2;
		if (parts[i].tmp > 20000)
		{
			sim->emp_trigger_count += 2;
			sim->emp_decor += 3;
			if (sim->emp_decor > 40)
				sim->emp_decor = 40;
			parts[i].life = 9;
			parts[i].temp = 0;
		}
		r = sim->photons[y][x];
		rndstore = rand();
		if (r)
		{
			parts[i].tmp += 2;
			if (parts[r>>8].temp > 370.0f)
				parts[i].tmp += (int)parts[r>>8].temp - 369;
			if (3 > (rndstore & 0xF))
				sim->kill_part(r>>8);
			rndstore >>= 4;
		}
		// Pressure absorption code
		if (sim->pv[y/CELL][x/CELL] > 2.5)
		{
			parts[i].tmp += 10;
			sim->pv[y/CELL][x/CELL]--;
		}
		else if (sim->pv[y/CELL][x/CELL] < -2.5)
		{
			sim->pv[y/CELL][x/CELL]++;
		}
		// Neighbor check loop
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (!rx != !ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (sim->elements[r&0xFF].HeatConduct > 0)
					{
						transfer = (int)(parts[r>>8].temp - 273.15f);
						parts[i].tmp += transfer;
						parts[r>>8].temp -= (float)transfer;
					}
				}
		for (trade = 0; trade < 9; trade++)
		{
			if (trade%2)
				rndstore = rand();
			rx = rndstore%7-3;
			rndstore >>= 3;
			ry = rndstore%7-3;
			rndstore >>= 3;
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				bool not_self = !((r&0xFF) == PT_E189 && parts[r>>8].life == 8);
				if ((r&0xFF) != PT_VIBR && (r&0xFF) != PT_BVBR && not_self)
					continue;
				if (not_self)
				{
					if (rr & 1)
					{ // VIBR2 <- VIBR
						parts[i].tmp += parts[r>>8].tmp;
						parts[r>>8].tmp = 0;
					}
					else
					{ // VIBR2 -> VIBR
						parts[r>>8].tmp += parts[i].tmp;
						parts[i].tmp = 0;
					}
					break;
				}
				if (parts[i].tmp > parts[r>>8].tmp)
				{
					transfer = parts[i].tmp - parts[r>>8].tmp;
					parts[r>>8].tmp += transfer/2;
					parts[i].tmp -= transfer/2;
					break;
				}
			}
		}
		if (parts[i].tmp < 0)
			parts[i].tmp = 0; // only preventing because negative tmp doesn't save
		break;
	case 9: // VIBR-like explosion
		if (parts[i].temp >= 9600)
		{
			sim->part_change_type(i, x, y, PT_VIBR);
			parts[i].temp = MAX_TEMP;
			parts[i].life = 750;
			parts[i].tmp2 = 0;
			sim->emp2_trigger_count ++;
		}
		parts[i].temp += 12;
		trade = 5;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					if (trade >= 5)
					{
						rndstore = rand(); trade = 0;
					}
					r = pmap[y+ry][x+rx];
					rt = r & 0xFF;
					if (!r || (sim->elements[rt].Properties & PROP_NODESTRUCT) || rt == PT_VIBR || rt == PT_BVBR || rt == PT_WARP)
						continue;
					if (rt == PT_E189)
					{
						if (parts[r>>8].life == 8)
							parts[r>>8].tmp += 1000;
						continue;
					}
					if (!(rndstore & 0x7))
					{
						sim->part_change_type(r>>8, x+rx, y+ry, PT_E189);
						parts[r>>8].life = 8;
						parts[r>>8].tmp = 21000;
					}
					trade++; rndstore >>= 3;
				}
		break;
	case 10: // electronics debugger [电子产品调试]
		for (rx = -1; rx <= 1; rx++)
			for (ry = -1; ry <= 1; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF) == PT_SPRK && parts[r>>8].life == 3)
					{
						switch (rtmp & 0x3F)
						{
							case 0: sim->E189_pause |=  0x01; break;
							case 1: sim->E189_pause |=  0x02; break;
							case 2: sim->E189_pause |=  0x08; break;
							case 3: sim->E189_pause &= ~0x08; break;
							case 4: sim->E189_pause |=  0x10; break;
							case 5: sim->E189_pause |=  0x20; break;
							case 6: sim->E189_pause |=  0x40; break;
						}
						if ((rtmp & 0x7E) == 0x40 && (rx != ry))
							E189_Update::InsertText(sim, i, x, y, -rx, -ry);
					}
				}
		break;
	case 11: // photons emitter
		for (rx = -1; rx <= 1; rx++)
			for (ry = -1; ry <= 1; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF) == PT_SPRK && parts[r>>8].life == 3)
					{
						ri = sim->create_part(-3, x-rx, y-ry, PT_PHOT);
						parts[ri].vx = (float)(-3 * rx);
						parts[ri].vy = (float)(-3 * ry);
						parts[ri].life = parts[i].tmp2;
						parts[ri].temp = parts[i].temp;

						rtmp = parts[i].ctype & 0x3FFFFFFF;
						if (rtmp)
							parts[ri].ctype = rtmp;

						if (ri > i)
							parts[ri].flags |= FLAG_SKIPMOVE;
					}
				}
		break;
	case 12: // SPRK reflector
		if (!(rtmp & 0x4))
		{
			for (rx = -1; rx <= 1; rx++)
				for (ry = -1; ry <= 1; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						rt = r & 0xFF;
						rtmp = parts[i].tmp;
						if (!r)
							continue;
						if (rt == PT_SPRK && ( !(rtmp & 8) == !(sim->elements[parts[r>>8].ctype].Properties & PROP_INSULATED) ) && parts[r>>8].life == 3)
						{
							switch (rtmp & 0x3)
							{
							case 0: parts[i].tmp = 1; break;
							case 1: parts[i].tmp = 0; break;
							case 2:
								rr = pmap[y-ry][x-rx];
								if ((rr & 0xFF) == PT_E189)
									sim->kill_part(rr >> 8);
								else
								{
									ri = sim->create_part(-1,x-rx,y-ry,PT_E189,12);
									if (ri >= 0)
										parts[ri].tmp = 3;
								}
								break;
							}
						}
						if ((rtmp & 0x1) && (sim->elements[rt].Properties & (PROP_CONDUCTS|PROP_INSULATED)) == PROP_CONDUCTS)
						{
							conductTo (sim, r, x+rx, y+ry, parts);
						}
					}
		}
		break;
	case 14: // dynamic decoration (DECO2)
		switch (parts[i].tmp2 >> 24)
		{
		case 0:
			rtmp = (parts[i].tmp & 0xFFFF) + (parts[i].tmp2 & 0xFFFF);
			rctype = (parts[i].ctype >> 16) + (parts[i].tmp >> 16) + (rtmp >> 16);
			parts[i].tmp2 = (parts[i].tmp2 & ~0xFFFF) | (rtmp & 0xFFFF);
			parts[i].ctype = (parts[i].ctype & 0xFFFF) | ((rctype % 0x0600) << 16);
			break;
		case 1:
			rtmp  = (parts[i].ctype & 0x7F7F7F7F) + (parts[i].tmp & 0x7F7F7F7F);
			rtmp ^= (parts[i].ctype ^ parts[i].tmp) & 0x80808080;
			parts[i].ctype = rtmp;
			break;
		case 2:
			rtmp = parts[i].tmp2 & 0x00FFFFFF;
			rtmp ++;
			if (parts[i].tmp3 <= rtmp)
			{
				rtmp = parts[i].tmp;
				parts[i].tmp = parts[i].ctype;
				parts[i].ctype = rtmp;
				rtmp = 0;
			}
			parts[i].tmp2 = 0x02000000 | rtmp;
			break;
		}
		break;
	case 16:
		int PSCNCount;
		switch (rctype = parts[i].ctype)
		{
		case 0: // logic gate

			if (!(parts[i].tmp & 4) == (parts[i].tmp2 > 0))
			{
				for (rx = -2; rx <= 2; rx++)
					for (ry = -2; ry <= 2; ry++)
						if (BOUNDS_CHECK && (rx || ry))
						{
							r = pmap[y+ry][x+rx];
							if ((r & 0xFF) == PT_NSCN) /* && parts[r>>8].life == 0 */
								conductTo (sim, r, x+rx, y+ry, parts);
						}
			}

			PSCNCount = 0;
			for (rx = -2; rx <= 2; rx++)
				for (ry = -2; ry <= 2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((r & 0xFF) == PT_SPRK && parts[r>>8].ctype == PT_PSCN && parts[r>>8].life == 3)
							PSCNCount ++;
						rr = ((r>>8) > i) ? (parts[r>>8].tmp) : (parts[r>>8].tmp2);
						if ((r & 0xFF) == PT_E189 && parts[r>>8].life == 19 && !rr)
							PSCNCount ++;
					}
			rtmp = parts[i].tmp;
			if ((rtmp & 3) != 3)
			{
				if (PSCNCount > (rtmp & 3)) // N-input logic gate
					parts[i].tmp2 = 9;
			}
			else if (PSCNCount & 1)
				parts[i].tmp2 = 9; // XOR gate
			break;
		case 1: // conduct->insulate counter
			if (parts[i].tmp)
			{
				if (parts[i].tmp2 == 1)
				{
					for (rx = -2; rx <= 2; rx++)
						for (ry = -2; ry <= 2; ry++)
							if (BOUNDS_CHECK && (rx || ry))
							{
								r = pmap[y+ry][x+rx];
								if ((r & 0xFF) == PT_NSCN)
									conductTo (sim, r, x+rx, y+ry, parts);
							}
					parts[i].tmp--;
				}
				for (rx = -2; rx <= 2; rx++)
					for (ry = -2; ry <= 2; ry++)
						if (BOUNDS_CHECK && (rx || ry))
						{
							r = pmap[y+ry][x+rx];
							if ((r & 0xFF) == PT_SPRK && parts[r>>8].ctype == PT_PSCN && parts[r>>8].life == 3)
							{
								parts[i].tmp2 = 2;
								goto break1a;
							}
						}
			}
			break1a:
			break;
		case 2: // insulate->conduct counter
			if (parts[i].tmp2)
			{
				if (parts[i].tmp2 == 1)
					parts[i].tmp--;
			}
			else if (!parts[i].tmp)
				sim->create_part(i, x, y, PT_METL);
			for (rx = -2; rx <= 2; rx++)
				for (ry = -2; ry <= 2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((r & 0xFF) == PT_SPRK && parts[r>>8].ctype == PT_PSCN && parts[r>>8].life == 3)
						{
							parts[i].tmp2 = 6;
							goto break1a;
						}
					}
			break;
		case 3: // flip-flop
			if (parts[i].tmp >= 2)
			{
				for (rx = -2; rx <= 2; rx++)
					for (ry = -2; ry <= 2; ry++)
						if (BOUNDS_CHECK && (rx || ry))
						{
							r = pmap[y+ry][x+rx];
							if ((r & 0xFF) == PT_NSCN)
								conductTo (sim, r, x+rx, y+ry, parts);
						}
				parts[i].tmp = 0;
			}
			for (rx = -2; rx <= 2; rx++)
				for (ry = -2; ry <= 2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((r & 0xFF) == PT_SPRK && parts[r>>8].ctype == PT_PSCN && parts[r>>8].life == 3)
						{
							parts[i].tmp ++;
							goto break1a;
						}
					}
			break;
		case 4: // virus curer
			for (rx = -1; rx < 2; rx++)
				for (ry = -1; ry < 2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((r & 0xFF) == PT_E189 && parts[r>>8].life == 16)
							r = pmap[y+2*ry][x+2*rx];
						if ((r & 0xFF) == PT_SPRK)
							goto break2a;
					}
			break;
		break2a:
			for (rtmp = 0; rtmp < 4; rtmp++)
			{
				if (BOUNDS_CHECK)
				{
					rx = tron_rx[rtmp];
					ry = tron_ry[rtmp];
					r = pmap[y+ry][x+rx];
					if ((r&0xFF) == PT_VIRS || (r&0xFF) == PT_VRSS || (r&0xFF) == PT_VRSG) // if is virus
						parts[r>>8].pavg[0] = 10;
				}
			}
			break;
		case 5:
			for (rx = -1; rx < 2; rx++)
				for (ry = -1; ry < 2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((r & 0xFF) == PT_SPRK && parts[r>>8].life == 3)
						{
							parts[i].tmp2 = 3;
							goto break2b;
						}
					}
			// break;
		break2b:
			if (parts[i].tmp2 == 2)
			{
				for (rtmp = 0; rtmp < 4; rtmp++)
				{
					if (BOUNDS_CHECK)
					{
						rx = tron_rx[rtmp];
						ry = tron_ry[rtmp];
						r = pmap[y+ry][x+rx];
						if ((r&0xFF) == PT_E189 && parts[r>>8].life == 16 && parts[r>>8].ctype == 5 && !parts[r>>8].tmp2)
							parts[r>>8].tmp2 = 3;
					}
				}
				parts[i].tmp = (parts[i].tmp & ~0x3F) | ((parts[i].tmp >> 3) & 0x7) | ((parts[i].tmp & 0x7) << 3);
			}
			break;
		case 6: // wire crossing
		case 7:
			{
				if (parts[i].tmp2 == 2)
				{
					for (rii = 0; rii < 4; rii++)
					{
						if (BOUNDS_CHECK)
						{
							r = osc_r1[rii], rtmp = parts[i].tmp;
							if (rtmp & 1 << (rctype & 1))
							{
								rx = pmap[y][x+r];
								if (sim->elements[rx&0xFF].Properties&PROP_CONDUCTS)
									conductTo(sim, rx, x+r, y, parts);
							}
							if (rtmp & 2 >> (rctype & 1))
							{
								ry = pmap[y+r][x];
								if (sim->elements[ry&0xFF].Properties&PROP_CONDUCTS)
									conductTo(sim, ry, x, y+r, parts);
							}
						}
					}
				}
				for (rr = rii = 0; rii < 4; rii++)
				{
					if (BOUNDS_CHECK)
					{
						r = osc_r1[rii];
						rx = pmap[y][x+r];
						ry = pmap[y+r][x];
						if ((rx & 0xFF) == PT_SPRK && parts[rx>>8].life == 3) rr |= 1;
						if ((ry & 0xFF) == PT_SPRK && parts[ry>>8].life == 3) rr |= 2;
					}
				}
				if (rr && !((rctype & 1) && parts[i].tmp2))
				{
					parts[i].tmp = rr; parts[i].tmp2 = 3;
				}
			}
			break;
		case 8: // FIGH stopper
			for (rx = -1; rx < 2; rx++)
				for (ry = -1; ry < 2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((r & 0xFF) == PT_SPRK && parts[r>>8].life == 3)
						{
							rtmp = parts[i].tmp;
							if (rtmp >= 0)
								sim->E189_FIGH_pause_check |= 1 << (rtmp < 31 ? (rtmp > 0 ? rtmp : 0) : 31);
							else
								sim->E189_pause |= 4;
							goto break1a;
						}
					}
			break;
		case 9:
			if (rtmp >= 0)
				rt = 1 & (sim->E189_FIGH_pause >> (rtmp & 0x1F));
			else
				rt = (int)(sim->no_generating_BHOL);
			for (rr = 0; rr < 4; rr++)
				if (BOUNDS_CHECK)
				{
					rx = tron_rx[rr];
					ry = tron_ry[rr];
					r = pmap[y+ry][x+rx];
					if ((r & 0xFF) == PT_SWCH)
					{
						rtmp = parts[r>>8].life;
						if (rt)
							parts[r>>8].life = 10;
						else if (rtmp >= 10)
							parts[r>>8].life = 9;
					}
					else if ((r & 0xFF) == PT_LCRY)
					{
						rtmp = parts[r>>8].tmp;
						if (rt && rtmp == 0)
							parts[r>>8].tmp = 2;
						if (!rt && rtmp == 3)
							parts[r>>8].tmp = 1;
					}
				}
			break;
		case 10:
			for (rr = 0; rr < 4; rr++)
				if (BOUNDS_CHECK)
				{
					rx = tron_rx[rr];
					ry = tron_ry[rr];
					r = pmap[y+ry][x+rx];
					if ((r & 0xFF) == PT_SPRK && parts[r>>8].life == 3)
					{
						direction = rr;
						for (rr = 0; rr < 4; rr++) // reset "rr" variable
							if (BOUNDS_CHECK)
							{
								rrx = tron_rx[rr];
								rry = tron_ry[rr];
								ri = pmap[y+rry][x+rrx];
								if ((ri & 0xFF) == PT_E189 && parts[ri>>8].life == 17)
								{
									rii = sim->create_part(-1, x-rx, y-ry, PT_E189, 24);
									rtmp = (direction << 2) | rr;
									if (rii >= 0)
									{
										parts[rii].ctype = rtmp;
										parts[rii].tmp = parts[ri>>8].tmp;
										parts[rii].tmp2 = parts[ri>>8].tmp2;
										parts[rii].tmp3 = parts[i].tmp;
										if (rii > i)
											parts[rii].flags |= FLAG_SKIPMOVE; // set wait flag
									}
									r = pmap[y-rry][x-rrx]; // variable "r" value override
									if ((r & 0xFF) == PT_E189 && parts[r>>8].life == 25)
									{
										rii = sim->create_part(-1, x-rx-rrx, y-ry-rry, PT_E189, 24);
										if (rii >= 0)
										{
											parts[rii].ctype = rtmp ^ 2;
											parts[rii].tmp = parts[ri>>8].tmp;
											parts[rii].tmp2 = parts[ri>>8].tmp2;
											parts[rii].tmp3 = parts[r>>8].tmp; // fixed overflow?
											if (rii > i)
												parts[rii].flags |= FLAG_SKIPMOVE; // set wait flag
										}
									}
									goto break1b;
								}
							}
						break;
					}
				}
			break1b:
			break;
		// case 11: reserved for E189's life = 24.
		case 12:
			{
				rndstore = rand();
				rx = (rndstore&1)*2-1;
				ry = (rndstore&2)-1;
				if (parts[i].tmp2 == 1)
				{
					for (rii = 1; rii <= 2; rii++)
					{
						if (BOUNDS_CHECK)
						{
							rtmp = parts[i].tmp;
							rrx = pmap[y][x+rx*rii];
							rry = pmap[y+ry*rii][x];
							if ((rry & 0xFF) == PT_NSCN && (rtmp & 1))
								conductTo (sim, rry, x, y+ry*rii, parts);
							if ((rrx & 0xFF) == PT_NSCN && (rtmp & 2))
								conductTo (sim, rrx, x+rx*rii, y, parts);
						}
					}
				}
				for (rr = rii = 0; rii < 4; rii++)
				{
					if (BOUNDS_CHECK)
					{
						r = osc_r1[rii];
						rx = pmap[y][x+r];
						ry = pmap[y+r][x];
						if ((rx & 0xFF) == PT_SPRK && parts[rx>>8].life == 3 && parts[rx>>8].ctype == PT_PSCN) rr |= 1;
						if ((ry & 0xFF) == PT_SPRK && parts[ry>>8].life == 3 && parts[ry>>8].ctype == PT_PSCN) rr |= 2;
					}
				}
				if (rr && !((rctype & 1) && parts[i].tmp2))
				{
					parts[i].tmp = rr; parts[i].tmp2 = 2;
				}
			}
			break;
		case 13:
			if (parts[i].tmp2 == 1)
			{
				for (rx = -2; rx <= 2; rx++)
					for (ry = -2; ry <= 2; ry++)
						if (BOUNDS_CHECK && (rx || ry))
						{
							r = pmap[y+ry][x+rx];
							if ((r & 0xFF) == PT_NSCN)
								sim->create_part(r>>8,x+rx,y+ry,PT_PSCN);
							if ((r & 0xFF) == PT_PSCN)
							{
								conductTo (sim, r, x+rx, y+ry, parts);
							}
						}
			}
			for (rx = -2; rx <= 2; rx++)
				for (ry = -2; ry <= 2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						rr = ((r>>8) > i) ? (parts[r>>8].tmp) : (parts[r>>8].tmp2);
						if ((r & 0xFF) == PT_E189 && parts[r>>8].life == 19 && rr == 9)
						{
							parts[i].tmp2 = 2;
							goto break1a;
						}
					}
			break;
		case 14:
		case 15:
			if (parts[i].tmp2 == 1)
			{
				rdif = (parts[i].tmp == PT_PSCN) ? 100.0f : -100.0f;
				for (rx = -2; rx <= 2; rx++)
					for (ry = -2; ry <= 2; ry++)
						if (BOUNDS_CHECK && (rx || ry))
						{
							r = pmap[y+ry][x+rx];
							if (rctype == 14 ? (r & 0xFF) == PT_WIFI : ((r & 0xFF) == PT_E189 && parts[r>>8].life == 33) )
							{
								parts[r>>8].temp = restrict_flt(parts[r>>8].temp + rdif, MIN_TEMP, MAX_TEMP);
							}
						}
				parts[i].tmp = 0; // PT_NONE ( or clear .tmp )
			}
			goto continue1a;
		case 16:
			if (parts[i].tmp2 == 1)
			{
				for (rx = -2; rx <= 2; rx++)
					for (ry = -2; ry <= 2; ry++)
						if (BOUNDS_CHECK && (rx || ry))
						{
							r = pmap[y+ry][x+rx];
							if ((r & 0xFF) == PT_PSNS || (r & 0xFF) == PT_TSNS || (r & 0xFF) == PT_DTEC)
							{
								parts[r>>8].tmp3 ^= 1;
							}
						}
			}
			goto continue1a;
		case 17:
			if (parts[i].tmp2 == 1)
			{
				for (rx = -1; rx < 2; rx++)
					for (ry = -1; ry < 2; ry++)
						if (BOUNDS_CHECK && (rx || ry))
						{
							nx = x + rx; ny = y + ry;
							r = pmap[ny][nx];
							if (!r)
								continue;
							if ((r & 0xFF) == PT_FILT)
							{
								rrx = parts[r>>8].ctype;
								rry = parts[i].tmp;
								switch (rry) // rry = sender type
								{
								// rrx = wavelengths
								case PT_PSCN: case PT_NSCN:
									rrx += (rry == PT_PSCN) ? 1 : -1;
									break;
								case PT_INST:
									rrx <<= 1;
									rrx |= (rrx >> 29) & 1; // for 29-bit FILT data
									break;
								case PT_INWR:
									// for 29-bit FILT data
									rrx &= 0x1FFFFFFF;
									rrx = (rrx >> 1) | (rrx << 28);
									break;
								}
								rrx &= 0x1FFFFFFF;
								rrx |= 0x20000000;
								while (BOUNDS_CHECK && (
									(r & 0xFF) == PT_FILT
								)) // check another FILT
								{
									parts[r>>8].ctype = rrx;
									r = pmap[ny += ry][nx += rx];
								}
								break;
							}
							else if ((r & 0xFF) == PT_CRAY)
							{
								for (;;)
								{
									r = pmap[ny += ry][nx += rx];
									if (!r)
									{
										sim->create_part(-1, nx, ny, PT_INWR);
										break;
									}
									else if ((r&0xFF) == PT_INWR)
										sim->kill_part(r>>8);
									else
										break;
								}
								break;
							}
						}
			}
			goto continue1a;
		case 18:
			if (parts[i].tmp2 == 1)
			{
				for (rx = -1; rx < 2; rx++)
					for (ry = -1; ry < 2; ry++)
						if (BOUNDS_CHECK && (rx || ry))
						{
							nx = x + rx; ny = y + ry;
							r = pmap[ny][nx];
							if (!r)
								continue;
							if ((r & 0xFF) == PT_FILT)
							{
								rrx = parts[r>>8].ctype;
								rry = parts[i].tmp;
								switch (rry) // rry = sender type
								{
								// rrx = wavelengths
								case PT_PSCN:
									rrx &= ~0x1;
								case PT_NSCN:
									rrx |= 0x1;
									break;
								case PT_INWR:
									rrx ^= 0x1;
									break;
								case PT_PTCT:
									rrx <<= 1;
									break;
								case PT_NTCT:
									// for 29-bit FILT data
									rrx = (rrx >> 1) & 0x0FFFFFFF;
									break;
								case PT_INST:
									rry = (rrx ^ (rrx>>28)) & 0x00000001; // swap 28
									rrx ^= (rry| (rry<<28));
									rry = (rrx ^ (rrx>>18)) & 0x000003FE; // swap 18
									rrx ^= (rry| (rry<<18));
									rry = (rrx ^ (rrx>> 6)) & 0x00381C0E; // swap 6
									rrx ^= (rry| (rry<< 6));
									rry = (rrx ^ (rrx>> 2)) & 0x02492492; // swap 2
									parts[r>>8].ctype = rry ^ (rry | (rry << 2));
									goto continue1b;
								}
								rrx &= 0x1FFFFFFF;
								rrx |= 0x20000000;
							continue1b:
								while (BOUNDS_CHECK && (
									(r & 0xFF) == PT_FILT
								)) // check another FILT
								{
									parts[r>>8].ctype = rrx;
									r = pmap[ny += ry][nx += rx];
								}
							}
						}
			}
			goto continue1a;
		continue1a:
			for (rx = -2; rx <= 2; rx++)
				for (ry = -2; ry <= 2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if (!r) continue;
						pavg = sim->parts_avg(i,r>>8,PT_INSL);
						if ((pavg != PT_INSL && pavg != PT_INDI) && (r & 0xFF) == PT_SPRK && parts[r>>8].life == 3)
						{
							parts[i].tmp = parts[r>>8].ctype;
							parts[i].tmp2 = 2;
							goto break1a;
						}
					}
			break;
		case 19: // universal conducts?
			if (parts[i].tmp2 == 1)
			{
				for (rx = -1; rx < 2; rx++)
					for (ry = -1; ry < 2; ry++)
						if (BOUNDS_CHECK && (!rx || !ry))
						{
							r = pmap[y+ry][x+rx];
							if (!r)
								continue;
							pavg = sim->parts_avg(i,r>>8,PT_INSL);
							if (pavg != PT_INSL && pavg != PT_INDI)
							{
								if ((r & 0xFF) == PT_INST)
								{
									sim->FloodINST(x+rx,y+ry,PT_SPRK,PT_INST);
								}
								else if (sim->elements[r].Properties & PROP_CONDUCTS)
								{
									conductTo (sim, r, x+rx, y+ry, parts);
								}
							}
						}
			}
			goto continue1c;
		continue1c:
			for (rx = -2; rx <= 2; rx++)
				for (ry = -2; ry <= 2; ry++)
					if (BOUNDS_CHECK && (!rx || !ry))
					{
						r = pmap[y+ry][x+rx];
						if (!r) continue;
						pavg = sim->parts_avg(i,r>>8,PT_INSL);
						if ((pavg != PT_INSL && pavg != PT_INDI) && (r & 0xFF) == PT_SPRK && parts[r>>8].life == 3)
						{
							parts[i].tmp2 = 2;
							goto break1a;
						}
					}
			break;
		}
		break;
			
	case 19:
		parts[i].tmp2 = parts[i].tmp;
		if (parts[i].tmp)
			--parts[i].tmp;
		for (rx = -2; rx <= 2; rx++)
			for (ry = -2; ry <= 2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r & 0xFF) == PT_SPRK && parts[r>>8].ctype == PT_PSCN && parts[r>>8].life == 3)
					{
						parts[i].tmp = 9;
						goto break3;
					}
				}
		break3:
		break;
	case 20: // particle emitter
		for (rx = -2; rx <= 2; rx++)
			for (ry = -2; ry <= 2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					rctype = parts[i].ctype;
					if ((r & 0xFF) == PT_SPRK && parts[r>>8].life == 3)
					{
						if (!(rctype & 0xFF))
							goto break3;
						if ((rctype & 0xFF) != PT_LIGH || !(rand() & 7))
						{
							rx = rand()%3-1;
							ry = rand()%3-1;
							int np = sim->create_part(-1, x+rx, y+ry, rctype & 0xFF, rctype >> 8);
							if (np >= 0) { parts[np].vx = rx; parts[np].vy = ry; }
						}
						goto break3;
					}
				}
		break;
	case 21:
	/* MERC/DEUT/YEST expander, or SPNG "water releaser",
	 *   or TRON detector.
	 * note: exclude E185 "replicating powder"
	 */
		rndstore = rand(), trade = 5;
		for (rx = -1; rx < 2; rx++)
			for (ry = -1; ry < 2; ry++)
			{
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r & 0xFF) == PT_TRON && !(rx && ry)) // (!(rx && ry)) equivalent to (!rx || !ry)
					{
						rr = pmap[y-ry][x-rx];
						rt = rr & 0xFF;
						rr >>= 8;
						if (parts[rr].life == 30)
						{
							if ((parts[rr].tmp >> 20) == 3)
							{
								parts[rr].ctype &= ~0x1F;
								parts[rr].ctype |= (parts[r>>8].tmp >> 11) & 0x1F;
							}
							else
								parts[rr].ctype = (parts[r>>8].tmp >> 7) & 0x1FF;
						}
					}
					else
					{
						if (!(rndstore&7))
						{
							switch (r & 0xFF)
							{
							case PT_MERC:
								parts[r>>8].tmp += parts[i].tmp;
								break;
							case PT_DEUT:
								parts[r>>8].life += parts[i].tmp;
								break;
							case PT_YEST:
								rtmp = parts[i].tmp;
								if (rtmp > 0)
									parts[r>>8].temp = 303.0f + (rtmp > 28 ? 28 : (float)rtmp * 0.5f);
								else if (-rtmp > (rand()&31))
									sim->kill_part(r>>8);
								break;
							case PT_SPNG:
								if (parts[r>>8].life > 0)
								{
									rr = sim->create_part(-1, x-rx, y-ry, PT_WATR);
									if (rr >= 0)
										parts[r>>8].life --;
								}
								break;
							case PT_VIBR:
								if (parts[r>>8].tmp > 0)
								{
									rr = pmap[y-ry][x-rx];
									rt = rr & 0xFF;
									if (rt == PT_WATR || rt == PT_DSTW || rt == PT_SLTW || rt == PT_CBNW)
									{
										rr >>= 8;
										if(!(rand()%3))
											sim->part_change_type(rr, x-rx, y-ry, PT_O2);
										else
											sim->part_change_type(rr, x-rx, y-ry, PT_H2);
										if (rt == PT_CBNW)
										{
											rrx = rand() % 5 - 2;
											rry = rand() % 5 - 2;
											sim->create_part(-1, x+rrx, y+rry, PT_CO2);
										}
										parts[r>>8].tmp --;
									}
								}
								break;
							}
						}
						if (!--trade)
						{
							trade = 5;
							rndstore = rand();
						}
						else
							rndstore >>= 3;
					}
				}
			}
		break;
	case 24: // moving duplicator particle
		if (parts[i].flags & FLAG_SKIPMOVE)
			parts[i].flags &= ~FLAG_SKIPMOVE; // clear wait flag
		else if (BOUNDS_CHECK)
		{
			/* definition:
			 *   tmp = length, tmp2 = total distance
			 * first step: like DRAY action
			 */
			rctype = parts[i].ctype;
			rr   = parts[i].tmp2;
			rtmp = parts[i].tmp;
			rtmp = rtmp > rr ? rr : (rtmp <= 0 ? rr : rtmp);
			rx = tron_rx[(rctype>>2) & 3], ry = tron_ry[(rctype>>2) & 3];
			int x_src = x + rx, y_src = y + ry, rx_dest = rx * rr, ry_dest = ry * rr;
			// int x_copyTo, y_copyTo;

			rr = pmap[y_src][x_src]; // override "rr" variable
			while (sim->InBounds(x_src, y_src) && rtmp--)
			{
				r = pmap[y_src][x_src];
				if (r) // if particle exist
				{
					rt = r & 0xFF;
					nx = x_src + rx_dest;
					ny = y_src + ry_dest;
					if (!sim->InBounds(nx, ny))
						break;
					rii = sim->create_part(-1, nx, ny, (rt == PT_SPRK) ? PT_METL : rt); // spark hack
					if (rii >= 0)
					{
						if (rt == PT_SPRK)
							sim->part_change_type(rii, nx, ny, PT_SPRK); // restore type for spark hack
						parts[rii] = parts[r>>8]; // duplicating all properties?
						parts[rii].x = nx; // restore X coordinates
						parts[rii].y = ny; // restore Y coordinates
					}
				}
				x_src += rx, y_src += ry;
			}
			
			rx_dest = x + tron_rx[rctype & 3], ry_dest = y + tron_ry[rctype & 3]; // override 2 variables (variable renaming?)
			if (parts[i].tmp3)
			{
				if (!(--parts[i].tmp3))
				{
					sim->kill_part(i); break;
				}
			}
			else if ((rr&0xFF) == PT_E189 && parts[rii = rr>>8].life == 16 && parts[rii].ctype == 11)
			{
				sim->kill_part(i); break;
			}
			if (!sim->InBounds(rx_dest, ry_dest) || pmap[ry_dest][rx_dest]) // if out of boundary
				sim->kill_part(i);
			else
			{
				sim->pmap[y][x] = 0; // what stacked particle?
				sim->pmap[ry_dest][rx_dest] = (i << 8) | PT_E189; // actual is particle's index shift left by 8 plus particle's type
				parts[i].x = rx_dest;
				parts[i].y = ry_dest;
			}
		}
		break;
	case 26: // button
		if (rtmp)
		{
			if (rtmp == 8)
			{
				for (rx = -1; rx <= 1; rx++)
					for (ry = -1; ry <= 1; ry++)
					{
						r = pmap[y+ry][x+rx];
						rt = r & 0xFF;
						if ((sim->elements[rt].Properties & (PROP_CONDUCTS|PROP_INSULATED)) == PROP_CONDUCTS)
						{
							conductTo (sim, r, x+rx, y+ry, parts);
						}
					}
			}
			parts[i].tmp --;
		}
		break;
	case 28: // ARAY/BRAY reflector
		for (rx = -1; rx < 2; rx++)
			for (ry = -1; ry < 2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r & 0xFF) == PT_SPRK && parts[r>>8].life == 3)
					{
						parts[i].tmp ^= 1;
						goto break3;
					}
				}
		break;
	case 29: // TRON emitter
		for (rr = 0; rr < 4; rr++)
			if (BOUNDS_CHECK)
			{
				rx = tron_rx[rr];
				ry = tron_ry[rr];
				r = pmap[y-ry][x-rx];
				if ((r & 0xFF) == PT_SPRK && parts[r>>8].life == 3)
				{
					r = sim->create_part (-1, x+rx, y+ry, PT_TRON);
					if (r >= 0) {
						rrx = parts[r].tmp;
						rrx &= ~0x1006A; // clear direction data and custom flags
						rrx |= rr << 5; // set direction data
						rrx |= ((rtmp & 1) << 1) | ((rtmp & 2) << 2) | ((rtmp & 4) << 14); // set custom flags
						if (r > i) rrx |= 0x04;
						parts[r].tmp = rrx;
					}
				}
			}
		break;
	case 30: // TRON filter
		if (rtmp & 0x04)
			rtmp &= ~0x04;
		else if (rtmp & 0x01)
		{
			rt = rtmp >> 20;
			rrx = parts[i].ctype;
			if (rt == 4) // TRON splitter
			{
				for (rr = 0; rr < 4; rr++)
				{
					r = pmap[y + tron_ry[rr]][x + tron_rx[rr]];
					if ((r & 0xFF) == PT_E189 && parts[r >> 8].life == 3)
					{
						ri = r >> 8;
						parts[ri].tmp &= 0xE0000;
						parts[ri].tmp |= (rtmp & 0x1FF9F) | (rr << 5);
						if (ri > i)
							sim->parts[ri].tmp |= 0x04;
						parts[ri].tmp2 = parts[i].tmp2;
					}
				}
				parts[i].tmp = rtmp & 0x7E0000;
				break;
			}
			rr = (rtmp >> 5) & ((rtmp >> 19 & 1) - 1);
			direction = (rr + (rtmp >> 17)) & 0x3;
			r = pmap[y + tron_ry[direction]][x + tron_rx[direction]];
			rii = parts[r >> 8].life;
			if ((r & 0xFF) == PT_E189 && rii == 3)
			{
				ri = r >> 8;
				parts[ri].tmp &= 0xE0000;
				rctype = (rtmp >> 7) & 0x1FF;
				switch (rt & 7)
				{
					case 0: rctype  = rrx; break; // set colour
					case 1: rctype += rrx; break; // hue shift (add)
					case 2: rctype -= rrx; break; // hue shift (subtract)
					case 3: // if color is / isn't ... then pass through
						if ((((rctype >> 4) & 0x1F) == rrx) == ((rrx >> 5) & 1)) // rightmost 5 bits xnor 6th bit
							rtmp = 0;
					break;
				}
				parts[ri].tmp |= (rtmp & 0x1009F) | (((rctype % 368 + 368) % 368) << 7) | (direction << 5); // colour modulo 368, rather than 360
				if (ri > i)
					sim->parts[ri].tmp |= 0x04;
				parts[ri].tmp2 = parts[i].tmp2;
			}
			rtmp &= 0x7E0000;
		}
		parts[i].tmp = rtmp;
		break;
	case 31: // TRON delay
		if (rtmp & 0x04)
			rtmp &= ~0x04;
		else if (parts[i].tmp3)
			parts[i].tmp3--;
		else if (rtmp & 0x01)
		{
			rr = (rtmp >> 5) & ((rtmp >> 19 & 1) - 1);
			direction = (rr + (rtmp >> 17)) & 0x3;
			r = pmap[y + tron_ry[direction]][x + tron_rx[direction]];
			rii = parts[r >> 8].life;
			if ((r & 0xFF) == PT_E189 && (rii & ~0x1) == 2)
			{
				ri = r >> 8;
				parts[ri].tmp |= (rtmp & 0x1FF9F) | (direction << 5);
				if (ri > i)
					sim->parts[ri].tmp |= 0x04;
				parts[ri].tmp2 = parts[i].tmp2;
			}
			rtmp &= 0xE0000;
		}
		parts[i].tmp = rtmp;
		break;
	case 33: // Second Wi-Fi
		rr = (1 << (parts[i].ctype & 0x1F));
		rii = (parts[i].ctype >> 4) & 0xE;
		parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
		if (parts[i].tmp>=CHANNELS) parts[i].tmp = CHANNELS-1;
		else if (parts[i].tmp<0) parts[i].tmp = 0;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					// wireless[][0] - whether channel is active on this frame
					// wireless[][1] - whether channel should be active on next frame
					if (sim->wireless2[parts[i].tmp][rii] & rr)
					{
						if (((r&0xFF)==PT_NSCN||(r&0xFF)==PT_PSCN||(r&0xFF)==PT_INWR) && sim->wireless2[parts[i].tmp][rii])
						{
							conductTo (sim, r, x+rx, y+ry, parts);
						}
					}
					if ((r&0xFF)==PT_SPRK && parts[r>>8].ctype!=PT_NSCN && parts[r>>8].life>=3)
					{
						sim->wireless2[parts[i].tmp][rii+1] |= rr;
						sim->ISWIRE2 = 2;
					}
				}
		break;
	case 34: // Sub-frame filter incrementer
		for (rx = -1; rx < 2; rx++)
			for (ry = -1; ry < 2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					nx = x + rx; ny = y + ry;
					r = pmap[ny][nx];
					if ((r & 0xFF) == PT_FILT)
					{
						rr = parts[r>>8].ctype + parts[i].ctype;
						rr &= 0x1FFFFFFF;
						rr |= 0x20000000;
						while (BOUNDS_CHECK && (
							(r & 0xFF) == PT_FILT
						)) // check another FILT
						{
							parts[r>>8].ctype = rr;
							r = pmap[ny += ry][nx += rx];
						}
					}
				}
		break;
	}
	
	if(ttan>=2) {
		sim->air->bmap_blockair[y/CELL][x/CELL] = 1;
		sim->air->bmap_blockairh[y/CELL][x/CELL] = 0x8;
	}
		
	return 0;
}

E189_Update::~E189_Update() {}
