#include <element.h>

int update_SOAP(UPDATE_FUNC_ARGS) {
	int r, rx, ry, wc;

	wc = -1;

	//0x01 - bubble on/off
	//0x02 - first mate yes/no
	//0x04 - second mate yes/no
	
	if (pv[y/CELL][x/CELL]>0.5f)
		parts[i].ctype |= 0x01;

	if (parts[i].ctype&0x01 == 0x01)
	{
		parts[i].vy -= 0.1f;

		parts[i].vy *= 0.5f;
		parts[i].vx *= 0.5f;

		if(parts[i].ctype&0x02 != 0x02)
		{
			for (rx=-2; rx<3; rx++)
				for (ry=-2; ry<3; ry++)
					if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((r>>8)>=NPART || !r)
							continue;

						if ((parts[r>>8].type == PT_SOAP) && (parts[r>>8].ctype&0x01==0x01) 
								&& (parts[r>>8].ctype&0x04!=0x04))
						{
							if (parts[r>>8].ctype&0x02 == 0x02)
							{
								parts[i].tmp = r>>8;
								parts[r>>8].tmp2 = i;

								parts[i].ctype |= 0x02;
								parts[r>>8].ctype |= 0x04;
							}
							else
							{
								if (parts[i].ctype&0x02!=0x02)
								{
									parts[i].tmp = r>>8;
									parts[r>>8].tmp2 = i;

									parts[i].ctype |= 0x02;
									parts[r>>8].ctype |= 0x04;
								}
							}
						}

					}
		}
		else
		{
			float d, dx, dy;

			dx = parts[i].x - parts[parts[i].tmp].x;
			dy = parts[i].y - parts[parts[i].tmp].y;

			d = 9/(pow(dx, 2)+pow(dy, 2)+9)-0.5;

			parts[parts[i].tmp].x -= dx*d;
			parts[parts[i].tmp].y -= dy*d;

			parts[i].x += dx*d;
			parts[i].y += dy*d;

			if ((parts[parts[i].tmp].ctype&0x02 == 0x02) && (parts[parts[i].tmp].ctype&0x01 == 0x01))
			{
				float d, dx, dy;
				int ii;

				ii = parts[parts[i].tmp].tmp;

				dx = parts[ii].x - parts[parts[i].tmp].x;
				dy = parts[ii].y - parts[parts[i].tmp].y;

				d = 36/(pow(dx, 2)+pow(dy, 2)+36)-0.5;

				parts[parts[i].tmp].x -= dx*d*0.5f;
				parts[parts[i].tmp].y -= dy*d*0.5f;

				parts[ii].x += dx*d*0.5f;
				parts[ii].y += dy*d*0.5f;
			}
		}

	}
	else
	{
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>8)>=NPART || !r)
						continue;

				}
	}

	return 0;
}
