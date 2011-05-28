#include <element.h>

void detach(int i)
{
	if ((parts[i].ctype&2) == 2)
	{
		if ((parts[parts[i].tmp].ctype&4) == 4)
			parts[parts[i].tmp].ctype ^= 4;
	}

	if ((parts[i].ctype&4) == 4)
	{
		if ((parts[parts[i].tmp2].ctype&2) == 2)
			parts[parts[i].tmp2].ctype ^= 2;
	}

	parts[i].ctype = 0;
}

int update_SOAP(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, target, ii, buf;
	float d, dx, dy, ax, ay;

	//0x01 - bubble on/off
	//0x02 - first mate yes/no
	//0x04 - "back" mate yes/no

	if ((parts[i].ctype&1) == 1)
	{
		if (parts[i].life<=0)
		{
			if ((parts[i].ctype&6) != 6 && parts[i].ctype>1)
			{

				target = i;

				while((parts[target].ctype&6) != 6 && parts[target].ctype>1)
				{
					if ((parts[target].ctype&2) == 2)
					{
						target = parts[target].tmp;
						detach(target);
					}

					if ((parts[target].ctype&4) == 4)
					{
						target = parts[target].tmp2;
						detach(target);
					}
				}
			}

			if ((parts[i].ctype&6) != 6)
				parts[i].ctype = 0;

			if ((parts[i].ctype&6) == 6 && (parts[parts[i].tmp].ctype&6) == 6 && parts[parts[i].tmp].tmp == i)
				detach(i);
		}

		parts[i].vy -= 0.1f;

		parts[i].vy *= 0.5f;
		parts[i].vx *= 0.5f;

		if((parts[i].ctype&2) != 2)
		{
			for (rx=-2; rx<3; rx++)
				for (ry=-2; ry<3; ry++)
					if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((r>>PS)>=NPART || !r)
							continue;

						if ((parts[r>>PS].type == PT_SOAP) && ((parts[r>>PS].ctype&1) == 1)
								&& ((parts[r>>PS].ctype&4) != 4))
						{
							if ((parts[r>>PS].ctype&2) == 2)
							{
								parts[i].tmp = r>>PS;
								parts[r>>PS].tmp2 = i;

								parts[i].ctype |= 2;
								parts[r>>PS].ctype |= 4;
							}
							else
							{
								if ((parts[i].ctype&2) != 2)
								{
									parts[i].tmp = r>>PS;
									parts[r>>PS].tmp2 = i;

									parts[i].ctype |= 2;
									parts[r>>PS].ctype |= 4;
								}
							}
						}
					}
		}
		else
		{
			if (parts[i].life<=0)
				for (rx=-2; rx<3; rx++)
					for (ry=-2; ry<3; ry++)
						if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
						{
							r = pmap[y+ry][x+rx];
							if ((r>>PS)>=NPART || !r)
								continue;

							if (((r&TYPE) != PT_SOAP && (r&TYPE) != PT_GLAS)
									|| (parts[r>>PS].ctype == 0 && (r&TYPE) == PT_SOAP
									&& (abs(parts[r>>PS].vx)<2 || abs(parts[r>>PS].vy)<2)))
							{
								detach(i);
								continue;
							}

							if ((r&TYPE) == PT_SOAP && parts[r>>PS].ctype == 1)
							{

								buf = parts[i].tmp;

								parts[i].tmp = r>>PS;
								parts[buf].tmp2 = r>>PS;
								parts[r>>PS].tmp2 = i;
								parts[r>>PS].tmp = buf;
								parts[r>>PS].ctype = 7;
							}

							if ((r&TYPE) == PT_SOAP && parts[r>>PS].ctype == 7 && parts[i].tmp != r>>PS && parts[i].tmp2 != r>>PS)
							{

								parts[parts[i].tmp].tmp2 = parts[r>>PS].tmp2;
								parts[parts[r>>PS].tmp2].tmp = parts[i].tmp;
								parts[r>>PS].tmp2 = i;
								parts[i].tmp = r>>PS;
							}
						}
		}

		if((parts[i].ctype&2) == 2)
		{


			dx = parts[i].x - parts[parts[i].tmp].x;
			dy = parts[i].y - parts[parts[i].tmp].y;

			d = 9/(pow(dx, 2)+pow(dy, 2)+9)-0.5;

			parts[parts[i].tmp].vx -= dx*d;
			parts[parts[i].tmp].vy -= dy*d;

			parts[i].vx += dx*d;
			parts[i].vy += dy*d;

			if (((parts[parts[i].tmp].ctype&2) == 2) && ((parts[parts[i].tmp].ctype&1) == 1)
					&& ((parts[parts[parts[i].tmp].tmp].ctype&2) == 2) && ((parts[parts[parts[i].tmp].tmp].ctype&1) == 1))
			{

				ii = parts[parts[parts[i].tmp].tmp].tmp;

				dx = parts[ii].x - parts[parts[i].tmp].x;
				dy = parts[ii].y - parts[parts[i].tmp].y;

				d = 81/(pow(dx, 2)+pow(dy, 2)+81)-0.5;

				parts[parts[i].tmp].vx -= dx*d*0.5f;
				parts[parts[i].tmp].vy -= dy*d*0.5f;

				parts[ii].vx += dx*d*0.5f;
				parts[ii].vy += dy*d*0.5f;
			}
		}
	}
	else
	{
		if (pv[y/CELL][x/CELL]>0.5f || pv[y/CELL][x/CELL]<(-0.5f))
		{
			parts[i].ctype = 1;
			parts[i].life = 10;
		}

		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>PS)>=NPART || !r)
						continue;

					if ((r&TYPE) == PT_OIL)
					{
						parts[i].vy -= 0.1f;

						parts[i].vy *= 0.5f;
						parts[i].vx *= 0.5f;

						ax = (parts[i].vx + parts[r>>PS].vx)/2;
						ay = (parts[i].vy + parts[r>>PS].vy)/2;

						parts[i].vx = ax;
						parts[i].vy = ay;
						parts[r>>PS].vx = ax;
						parts[r>>PS].vy = ay;
					}
				}
	}

	return 0;
}
