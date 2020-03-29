#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_ESNG()
{
	Identifier = "DEFAULT_PT_ESNG";
	Name = "ESNG";
	Colour = PIXPACK(0xC4F2EB);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;
  
  Advection = 0.0f;
	AirDrag = 0.00f;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -0.99f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

  HeatConduct = 255;
  DefaultProperties.temp = 0.0f;
  
  Description = "Energetic Singularity. Absorbs heat and releases it as energy on touch.";
  
	Properties = TYPE_ENERGY;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, cry, crx, spawncount, nb, rx2, ry2, r2;
  float angle, v;
  
  if (sim->parts[i].temp > 100 && !(sim->aheat_enable))
    {
      sim->parts[i].temp -= 100;
      sim->parts[i].life += 1;
    }
  
  for (rx=-3; rx<4; rx++) 
		for (ry=-3; ry<4; ry++)
      {
        r = pmap[y+ry][x+rx];
        if (!r) continue;
        if (sim->parts[ID(r)].life > (sim->parts[i].life+1)) {
          sim->parts[ID(r)].life -= 1;
          sim->parts[i].life += 1;
        }
        if (sim->parts[ID(r)].temp > 100)
          {
            sim->parts[ID(r)].temp -= 100;
            sim->parts[i].life += 1;
          }
      }
  
  int rmin = (sim->parts[i].vx || sim->parts[i].vy) ? -8 : -1;
  int rmax = (sim->parts[i].vx || sim->parts[i].vy) ? 9 : 2;
  
	for (rx=rmin; rx<rmax; rx++) 
		for (ry=rmin; ry<rmax; ry++)
      if (!(rx == 0 && ry == 0)) {
        r = pmap[y+ry][x+rx];
				if (!r)
					continue;
        for (rx2=-8; rx2<9; rx2++) 
          for (ry2=-8; ry2<9; ry2++)
            {
              r2 = pmap[y+ry][x+rx];
              if (!r2) continue;
              if (sim->parts[ID(r2)].type == sim->parts[i].type) {
                sim->parts[i].life += sim->parts[ID(r2)].life;
                sim->kill_part(ID(r2));
              }
            }
        int life = sim->parts[i].life;
        sim->kill_part(i);
        for (rx=-1; rx<2; rx++) {
          crx = (x/CELL)+rx;
          for (ry=-1; ry<2; ry++) {
            cry = (y/CELL)+ry;
            if (cry >= 0 && crx >= 0 && crx < (XRES/CELL) && cry < (YRES/CELL)) {
              sim->pv[cry][crx] += (float)parts[i].life;
            }
          }
        }
        spawncount = std::abs(life);
        spawncount = ((double)(spawncount / 10))*M_PI;
        for (int j = 0;j < spawncount; j++)
        {
          switch (RNG::Ref().gen() % 5)
          {
            case 0:
              nb = sim->create_part(-3, x, y, PT_PHOT);
              break;
            case 1:
              nb = sim->create_part(-3, x, y, PT_NEUT);
              break;
            case 2:
              nb = sim->create_part(-3, x, y, PT_ELEC);
              break;
            case 3:
              nb = sim->create_part(-3, x, y, PT_GRVT);
              break;
            case 4:
              nb = sim->create_part(-3, x, y, PT_PROT);
              break;
          }
          if (nb!=-1) {
            parts[nb].life = RNG::Ref().between(0, 299);
            parts[nb].temp = MAX_TEMP/2;
            angle = RNG::Ref().uniform01()*2.0f*M_PI;
            v = RNG::Ref().uniform01()*5.0f;
            parts[nb].vx = v*cosf(angle);
            parts[nb].vy = v*sinf(angle);
          }
          else if (sim->pfree==-1)
            break;
        }
        break;
      }
  
	return 0;
}

static int max(int a, int b) {
  if (a > b) {
    return a;
  }
  return b;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 255;
	*firer = 196;
	*fireg = 242;
	*fireb = 235;

	*pixel_mode |= FIRE_ADD;
  
	return 0;
}

