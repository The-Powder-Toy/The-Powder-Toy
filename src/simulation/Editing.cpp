#include "Simulation.h"
#include "Sample.h"
#include "SimTool.h"
#include "Snapshot.h"
#include "Air.h"
#include "gravity/Gravity.h"
#include "common/tpt-rand.h"
#include "common/tpt-compat.h"
#include "client/GameSave.h"
#include "ElementClasses.h"
#include "graphics/Renderer.h"
#include "gui/game/Brush.h"
#include <iostream>
#include <cmath>

std::unique_ptr<Snapshot> Simulation::CreateSnapshot() const
{
	auto snap = std::make_unique<Snapshot>();
	snap->AirPressure    .insert   (snap->AirPressure    .begin(), &pv  [0][0]      , &pv  [0][0] + NCELL);
	snap->AirVelocityX   .insert   (snap->AirVelocityX   .begin(), &vx  [0][0]      , &vx  [0][0] + NCELL);
	snap->AirVelocityY   .insert   (snap->AirVelocityY   .begin(), &vy  [0][0]      , &vy  [0][0] + NCELL);
	snap->AmbientHeat    .insert   (snap->AmbientHeat    .begin(), &hv  [0][0]      , &hv  [0][0] + NCELL);
	snap->BlockMap       .insert   (snap->BlockMap       .begin(), &bmap[0][0]      , &bmap[0][0] + NCELL);
	snap->ElecMap        .insert   (snap->ElecMap        .begin(), &emap[0][0]      , &emap[0][0] + NCELL);
	snap->BlockAir       .insert   (snap->BlockAir       .begin(), &air->bmap_blockair[0][0] , &air->bmap_blockair[0][0]  + NCELL);
	snap->BlockAirH      .insert   (snap->BlockAirH      .begin(), &air->bmap_blockairh[0][0], &air->bmap_blockairh[0][0] + NCELL);
	snap->FanVelocityX   .insert   (snap->FanVelocityX   .begin(), &fvx [0][0]      , &fvx [0][0] + NCELL);
	snap->FanVelocityY   .insert   (snap->FanVelocityY   .begin(), &fvy [0][0]      , &fvy [0][0] + NCELL);
	snap->GravVelocityX  .insert   (snap->GravVelocityX  .begin(), &gravx  [0]      , &gravx  [0] + NCELL);
	snap->GravVelocityY  .insert   (snap->GravVelocityY  .begin(), &gravy  [0]      , &gravy  [0] + NCELL);
	snap->GravValue      .insert   (snap->GravValue      .begin(), &gravp  [0]      , &gravp  [0] + NCELL);
	snap->GravMap        .insert   (snap->GravMap        .begin(), &gravmap[0]      , &gravmap[0] + NCELL);
	snap->Particles      .insert   (snap->Particles      .begin(), &parts  [0]      , &parts  [0] + parts_lastActiveIndex + 1);
	snap->PortalParticles.insert   (snap->PortalParticles.begin(), &portalp[0][0][0], &portalp[0][0][0] + CHANNELS * 8 * 80);
	snap->WirelessData   .insert   (snap->WirelessData   .begin(), &wireless[0][0]  , &wireless[0][0] + CHANNELS * 2);
	snap->stickmen       .insert   (snap->stickmen       .begin(), &fighters[0]     , &fighters[0] + MAX_FIGHTERS);
	snap->stickmen       .push_back(player2);
	snap->stickmen       .push_back(player);
	snap->signs = signs;
	snap->FrameCount = frameCount;
	snap->RngState = rng.state();
	return snap;
}

void Simulation::Restore(const Snapshot &snap)
{
	std::fill(elementCount, elementCount + PT_NUM, 0);
	elementRecount = true;
	force_stacking_check = true;
	for (auto &part : parts)
	{
		part.type = 0;
	}
	std::copy(snap.AirPressure    .begin(), snap.AirPressure    .end(), &pv[0][0]        );
	std::copy(snap.AirVelocityX   .begin(), snap.AirVelocityX   .end(), &vx[0][0]        );
	std::copy(snap.AirVelocityY   .begin(), snap.AirVelocityY   .end(), &vy[0][0]        );
	std::copy(snap.AmbientHeat    .begin(), snap.AmbientHeat    .end(), &hv[0][0]        );
	std::copy(snap.BlockMap       .begin(), snap.BlockMap       .end(), &bmap[0][0]      );
	std::copy(snap.ElecMap        .begin(), snap.ElecMap        .end(), &emap[0][0]      );
	std::copy(snap.BlockAir       .begin(), snap.BlockAir       .end(), &air->bmap_blockair[0][0] );
	std::copy(snap.BlockAirH      .begin(), snap.BlockAirH      .end(), &air->bmap_blockairh[0][0]);
	std::copy(snap.FanVelocityX   .begin(), snap.FanVelocityX   .end(), &fvx[0][0]       );
	std::copy(snap.FanVelocityY   .begin(), snap.FanVelocityY   .end(), &fvy[0][0]       );
	if (grav->IsEnabled())
	{
		grav->Clear();
		std::copy(snap.GravVelocityX.begin(), snap.GravVelocityX.end(), &gravx  [0]      );
		std::copy(snap.GravVelocityY.begin(), snap.GravVelocityY.end(), &gravy  [0]      );
		std::copy(snap.GravValue    .begin(), snap.GravValue    .end(), &gravp  [0]      );
		std::copy(snap.GravMap      .begin(), snap.GravMap      .end(), &gravmap[0]      );
	}
	std::copy(snap.Particles      .begin(), snap.Particles      .end(), &parts[0]        );
	std::copy(snap.PortalParticles.begin(), snap.PortalParticles.end(), &portalp[0][0][0]);
	std::copy(snap.WirelessData   .begin(), snap.WirelessData   .end(), &wireless[0][0]  );
	std::copy(snap.stickmen       .begin(), snap.stickmen.end() - 2   , &fighters[0]     );
	player  = snap.stickmen[snap.stickmen.size() - 1];
	player2 = snap.stickmen[snap.stickmen.size() - 2];
	signs = snap.signs;
	frameCount = snap.FrameCount;
	rng.state(snap.RngState);
	parts_lastActiveIndex = NPART - 1;
	RecalcFreeParticles(false);
	gravWallChanged = true;
}

void Simulation::clear_area(int area_x, int area_y, int area_w, int area_h)
{
	auto intersection = RES.OriginRect() & RectSized(Vec2{ area_x, area_y }, Vec2{ area_w, area_h });
	area_x = intersection.TopLeft.X;
	area_y = intersection.TopLeft.Y;
	area_w = intersection.Size().X;
	area_h = intersection.Size().Y;
	float fx = area_x-.5f, fy = area_y-.5f;
	for (int i = 0; i <= parts_lastActiveIndex; i++)
	{
		if (parts[i].type)
			if (parts[i].x >= fx && parts[i].x <= fx+area_w+1 && parts[i].y >= fy && parts[i].y <= fy+area_h+1)
				kill_part(i);
	}
	int cx1 = area_x/CELL, cy1 = area_y/CELL, cx2 = (area_x+area_w)/CELL, cy2 = (area_y+area_h)/CELL;
	for (int y = cy1; y <= cy2; y++)
	{
		for (int x = cx1; x <= cx2; x++)
		{
			if (bmap[y][x] == WL_GRAV)
				gravWallChanged = true;
			bmap[y][x] = 0;
			emap[y][x] = 0;
		}
	}
	for( int i = signs.size()-1; i >= 0; i--)
	{
		if (signs[i].text.length() && signs[i].x >= area_x && signs[i].y >= area_y && signs[i].x <= area_x+area_w && signs[i].y <= area_y+area_h)
		{
			signs.erase(signs.begin()+i);
		}
	}
}

SimulationSample Simulation::GetSample(int x, int y)
{
	SimulationSample sample;
	sample.PositionX = x;
	sample.PositionY = y;
	if (x >= 0 && x < XRES && y >= 0 && y < YRES)
	{
		if (photons[y][x])
		{
			sample.particle = parts[ID(photons[y][x])];
			sample.ParticleID = ID(photons[y][x]);
		}
		else if (pmap[y][x])
		{
			sample.particle = parts[ID(pmap[y][x])];
			sample.ParticleID = ID(pmap[y][x]);
		}
		if (bmap[y/CELL][x/CELL])
		{
			sample.WallType = bmap[y/CELL][x/CELL];
		}
		sample.AirPressure = pv[y/CELL][x/CELL];
		sample.AirTemperature = hv[y/CELL][x/CELL];
		sample.AirVelocityX = vx[y/CELL][x/CELL];
		sample.AirVelocityY = vy[y/CELL][x/CELL];

		if(grav->IsEnabled())
		{
			sample.Gravity = gravp[(y/CELL)*XCELLS+(x/CELL)];
			sample.GravityVelocityX = gravx[(y/CELL)*XCELLS+(x/CELL)];
			sample.GravityVelocityY = gravy[(y/CELL)*XCELLS+(x/CELL)];
		}
	}
	else
		sample.isMouseInSim = false;

	sample.NumParts = NUM_PARTS;
	return sample;
}

void Simulation::SetDecoSpace(int newDecoSpace)
{
	if (newDecoSpace < 0 || newDecoSpace >= NUM_DECOSPACES)
	{
		newDecoSpace = DECOSPACE_SRGB;
	}
	deco_space = newDecoSpace;
}

int Simulation::Tool(int x, int y, int tool, int brushX, int brushY, float strength)
{
	Particle * cpart = NULL;
	int r;
	if ((r = pmap[y][x]))
		cpart = &(parts[ID(r)]);
	else if ((r = photons[y][x]))
		cpart = &(parts[ID(r)]);
	auto &sd = SimulationData::CRef();
	return sd.tools[tool].Perform(this, cpart, x, y, brushX, brushY, strength);
}

int Simulation::CreateWalls(int x, int y, int rx, int ry, int wall, Brush const *cBrush)
{
	if(cBrush)
	{
		rx = cBrush->GetRadius().X;
		ry = cBrush->GetRadius().Y;
	}

	ry = ry/CELL;
	rx = rx/CELL;
	x = x/CELL;
	y = y/CELL;
	x -= rx;
	y -= ry;
	for (int wallX = x; wallX <= x+rx+rx; wallX++)
	{
		for (int wallY = y; wallY <= y+ry+ry; wallY++)
		{
			if (wallX >= 0 && wallX < XCELLS && wallY >= 0 && wallY < YCELLS)
			{
				if (wall == WL_FAN)
				{
					fvx[wallY][wallX] = 0.0f;
					fvy[wallY][wallX] = 0.0f;
				}
				else if (wall == WL_STREAM)
				{
					wallX = x + rx;
					wallY = y + ry;
					//streamlines can't be drawn next to each other
					for (int tempY = wallY-1; tempY < wallY+2; tempY++)
						for (int tempX = wallX-1; tempX < wallX+2; tempX++)
						{
							if (tempX >= 0 && tempX < XCELLS && tempY >= 0 && tempY < YCELLS && bmap[tempY][tempX] == WL_STREAM)
								return 1;
						}
				}
				if (wall == WL_GRAV || bmap[wallY][wallX] == WL_GRAV)
					gravWallChanged = true;

				if (wall == WL_ERASEALL)
				{
					for (int i = 0; i < CELL; i++)
						for (int j = 0; j < CELL; j++)
						{
							delete_part(wallX*CELL+i, wallY*CELL+j);
						}
					for (int i = signs.size()-1; i >= 0; i--)
						if (signs[i].x >= wallX*CELL && signs[i].y >= wallY*CELL && signs[i].x <= (wallX+1)*CELL && signs[i].y <= (wallY+1)*CELL)
							signs.erase(signs.begin()+i);
					bmap[wallY][wallX] = 0;
				}
				else
					bmap[wallY][wallX] = wall;
			}
		}
	}
	return 1;
}

void Simulation::CreateWallLine(int x1, int y1, int x2, int y2, int rx, int ry, int wall, Brush const *cBrush)
{
	int x, y, dx, dy, sy;
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	float e = 0.0f, de;
	if (reverseXY)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
	{
		y = x1;
		x1 = x2;
		x2 = y;
		y = y1;
		y1 = y2;
		y2 = y;
	}
	dx = x2 - x1;
	dy = abs(y2 - y1);
	de = dx ? dy/(float)dx : 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			CreateWalls(y, x, rx, ry, wall, cBrush);
		else
			CreateWalls(x, y, rx, ry, wall, cBrush);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if ((y1<y2) ? (y<=y2) : (y>=y2))
			{
				if (reverseXY)
					CreateWalls(y, x, rx, ry, wall, cBrush);
				else
					CreateWalls(x, y, rx, ry, wall, cBrush);
			}
			e -= 1.0f;
		}
	}
}

void Simulation::CreateWallBox(int x1, int y1, int x2, int y2, int wall)
{
	int i, j;
	if (x1>x2)
	{
		i = x2;
		x2 = x1;
		x1 = i;
	}
	if (y1>y2)
	{
		j = y2;
		y2 = y1;
		y1 = j;
	}
	for (j=y1; j<=y2; j++)
		for (i=x1; i<=x2; i++)
			CreateWalls(i, j, 0, 0, wall, NULL);
}

int Simulation::FloodWalls(int x, int y, int wall, int bm)
{
	int x1, x2, dy = CELL;
	if (bm==-1)
	{
		if (wall==WL_ERASE || wall==WL_ERASEALL)
		{
			bm = bmap[y/CELL][x/CELL];
			if (!bm)
				return 0;
		}
		else
			bm = 0;
	}

	if (bmap[y/CELL][x/CELL]!=bm)
		return 1;

	// go left as far as possible
	x1 = x2 = x;
	while (x1>=CELL)
	{
		if (bmap[y/CELL][(x1-1)/CELL]!=bm)
		{
			break;
		}
		x1--;
	}
	while (x2<XRES-CELL)
	{
		if (bmap[y/CELL][(x2+1)/CELL]!=bm)
		{
			break;
		}
		x2++;
	}

	// fill span
	for (x=x1; x<=x2; x++)
	{
		if (!CreateWalls(x, y, 0, 0, wall, NULL))
			return 0;
	}
	// fill children
	if (y>=CELL)
		for (x=x1; x<=x2; x++)
			if (bmap[(y-dy)/CELL][x/CELL]==bm)
				if (!FloodWalls(x, y-dy, wall, bm))
					return 0;
	if (y<YRES-CELL)
		for (x=x1; x<=x2; x++)
			if (bmap[(y+dy)/CELL][x/CELL]==bm)
				if (!FloodWalls(x, y+dy, wall, bm))
					return 0;
	return 1;
}

int Simulation::CreatePartFlags(int x, int y, int c, int flags)
{
	if (x < 0 || y < 0 || x >= XRES || y >= YRES)
	{
		return 0;
	}

	if (flags & REPLACE_MODE)
	{
		// if replace whatever and there's something to replace
		// or replace X and there's a non-energy particle on top with type X
		// or replace X and there's an energy particle on top with type X
		if ((!replaceModeSelected && (photons[y][x] || pmap[y][x])) ||
			(!photons[y][x] && pmap[y][x] && TYP(pmap[y][x]) == replaceModeSelected) ||
			(photons[y][x] && TYP(photons[y][x]) == replaceModeSelected))
		{
			if (c)
				create_part(photons[y][x] ? ID(photons[y][x]) : ID(pmap[y][x]), x, y, TYP(c), ID(c));
			else
				delete_part(x, y);
		}
		return 0;
	}
	else if (!c)
	{
		delete_part(x, y);
		return 0;
	}
	else if (flags & SPECIFIC_DELETE)
	{
		// if delete whatever and there's something to delete
		// or delete X and there's a non-energy particle on top with type X
		// or delete X and there's an energy particle on top with type X
		if ((!replaceModeSelected && (photons[y][x] || pmap[y][x])) ||
			(!photons[y][x] && pmap[y][x] && TYP(pmap[y][x]) == replaceModeSelected) ||
			(photons[y][x] && TYP(photons[y][x]) == replaceModeSelected))
		{
			delete_part(x, y);
		}
		return 0;
	}
	else
	{
		return (create_part(-2, x, y, TYP(c), ID(c)) == -1);
	}

	// I'm sure at least one compiler exists that would complain if this wasn't here
	return 0;
}

void Simulation::ApplyDecoration(int x, int y, int colR_, int colG_, int colB_, int colA_, int mode)
{
	int rp;
	float tr, tg, tb, ta, colR = float(colR_), colG = float(colG_), colB = float(colB_), colA = float(colA_);
	float strength = 0.01f;
	rp = pmap[y][x];
	if (!rp)
		rp = photons[y][x];
	if (!rp)
		return;

	ta = float((parts[ID(rp)].dcolour>>24)&0xFF);
	tr = float((parts[ID(rp)].dcolour>>16)&0xFF);
	tg = float((parts[ID(rp)].dcolour>>8)&0xFF);
	tb = float((parts[ID(rp)].dcolour)&0xFF);

	ta /= 255.0f; tr /= 255.0f; tg /= 255.0f; tb /= 255.0f;
	colR /= 255.0f; colG /= 255.0f; colB /= 255.0f; colA /= 255.0f;

	if (mode == DECO_DRAW)
	{
		ta = colA;
		tr = colR;
		tg = colG;
		tb = colB;
	}
	else if (mode == DECO_CLEAR)
	{
		ta = tr = tg = tb = 0.0f;
	}
	else if (mode == DECO_ADD)
	{
		//ta += (colA*strength)*colA;
		tr += (colR*strength)*colA;
		tg += (colG*strength)*colA;
		tb += (colB*strength)*colA;
	}
	else if (mode == DECO_SUBTRACT)
	{
		//ta -= (colA*strength)*colA;
		tr -= (colR*strength)*colA;
		tg -= (colG*strength)*colA;
		tb -= (colB*strength)*colA;
	}
	else if (mode == DECO_MULTIPLY)
	{
		tr *= 1.0f+(colR*strength)*colA;
		tg *= 1.0f+(colG*strength)*colA;
		tb *= 1.0f+(colB*strength)*colA;
	}
	else if (mode == DECO_DIVIDE)
	{
		tr /= 1.0f+(colR*strength)*colA;
		tg /= 1.0f+(colG*strength)*colA;
		tb /= 1.0f+(colB*strength)*colA;
	}
	else if (mode == DECO_SMUDGE)
	{
		if (x >= CELL && x < XRES-CELL && y >= CELL && y < YRES-CELL)
		{
			float tas = 0.0f, trs = 0.0f, tgs = 0.0f, tbs = 0.0f;

			int rx, ry;
			float num = 0;
			for (rx=-2; rx<3; rx++)
				for (ry=-2; ry<3; ry++)
				{
					if (abs(rx)+abs(ry) > 2 && TYP(pmap[y+ry][x+rx]) && parts[ID(pmap[y+ry][x+rx])].dcolour)
					{
						Particle part = parts[ID(pmap[y+ry][x+rx])];
						num += 1.0f;
						float pa = ((float)((part.dcolour>>24)&0xFF)) / 255.f;
						float pr = ((float)((part.dcolour>>16)&0xFF)) / 255.f;
						float pg = ((float)((part.dcolour>> 8)&0xFF)) / 255.f;
						float pb = ((float)((part.dcolour    )&0xFF)) / 255.f;
						switch (deco_space)
						{
						case DECOSPACE_SRGB:
							pa = (pa <= 0.04045f) ? (pa / 12.92f) : pow((pa + 0.055f) / 1.055f, 2.4f);
							pr = (pr <= 0.04045f) ? (pr / 12.92f) : pow((pr + 0.055f) / 1.055f, 2.4f);
							pg = (pg <= 0.04045f) ? (pg / 12.92f) : pow((pg + 0.055f) / 1.055f, 2.4f);
							pb = (pb <= 0.04045f) ? (pb / 12.92f) : pow((pb + 0.055f) / 1.055f, 2.4f);
							break;

						case DECOSPACE_LINEAR:
							break;

						case DECOSPACE_GAMMA22:
							pa = pow(pa, 2.2f);
							pr = pow(pr, 2.2f);
							pg = pow(pg, 2.2f);
							pb = pow(pb, 2.2f);
							break;

						case DECOSPACE_GAMMA18:
							pa = pow(pa, 1.8f);
							pr = pow(pr, 1.8f);
							pg = pow(pg, 1.8f);
							pb = pow(pb, 1.8f);
							break;
						}
						tas += pa;
						trs += pr;
						tgs += pg;
						tbs += pb;
					}
				}
			if (num == 0)
				return;
			ta = tas / num;
			tr = trs / num;
			tg = tgs / num;
			tb = tbs / num;
			switch (deco_space)
			{
			case DECOSPACE_SRGB:
				ta = (ta <= 0.0031308f) ? (ta * 12.92f) : (1.055f * pow(ta, 1.f / 2.4f) - 0.055f);
				tr = (tr <= 0.0031308f) ? (tr * 12.92f) : (1.055f * pow(tr, 1.f / 2.4f) - 0.055f);
				tg = (tg <= 0.0031308f) ? (tg * 12.92f) : (1.055f * pow(tg, 1.f / 2.4f) - 0.055f);
				tb = (tb <= 0.0031308f) ? (tb * 12.92f) : (1.055f * pow(tb, 1.f / 2.4f) - 0.055f);
				break;

			case DECOSPACE_LINEAR:
				break;

			case DECOSPACE_GAMMA22:
				ta = pow(ta, 1.f / 2.2f);
				tr = pow(tr, 1.f / 2.2f);
				tg = pow(tg, 1.f / 2.2f);
				tb = pow(tb, 1.f / 2.2f);
				break;

			case DECOSPACE_GAMMA18:
				ta = pow(ta, 1.f / 1.8f);
				tr = pow(tr, 1.f / 1.8f);
				tg = pow(tg, 1.f / 1.8f);
				tb = pow(tb, 1.f / 1.8f);
				break;
			}
			if (!parts[ID(rp)].dcolour)
				ta -= 3/255.0f;
		}
	}

	ta *= 255.0f; tr *= 255.0f; tg *= 255.0f; tb *= 255.0f;
	ta += .5f; tr += .5f; tg += .5f; tb += .5f;

	colA_ = int(ta);
	colR_ = int(tr);
	colG_ = int(tg);
	colB_ = int(tb);

	if(colA_ > 255)
		colA_ = 255;
	else if(colA_ < 0)
		colA_ = 0;
	if(colR_ > 255)
		colR_ = 255;
	else if(colR_ < 0)
		colR_ = 0;
	if(colG_ > 255)
		colG_ = 255;
	else if(colG_ < 0)
		colG_ = 0;
	if(colB_ > 255)
		colB_ = 255;
	else if(colB_ < 0)
		colB_ = 0;
	parts[ID(rp)].dcolour = ((colA_<<24)|(colR_<<16)|(colG_<<8)|colB_);
}

void Simulation::ApplyDecorationPoint(int positionX, int positionY, int colR, int colG, int colB, int colA, int mode, Brush const &cBrush)
{
	for (ui::Point off : cBrush)
	{
		ui::Point coords = ui::Point(positionX, positionY) + off;
		if (coords.X >= 0 && coords.Y >= 0 && coords.X < XRES && coords.Y < YRES)
			ApplyDecoration(coords.X, coords.Y, colR, colG, colB, colA, mode);
	}
}

void Simulation::ApplyDecorationLine(int x1, int y1, int x2, int y2, int colR, int colG, int colB, int colA, int mode, Brush const &cBrush)
{
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	int x, y, dx, dy, sy, rx = 0, ry = 0;
	float e = 0.0f, de;

	rx = cBrush.GetRadius().X;
	ry = cBrush.GetRadius().Y;

	if (reverseXY)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
	{
		y = x1;
		x1 = x2;
		x2 = y;
		y = y1;
		y1 = y2;
		y2 = y;
	}
	dx = x2 - x1;
	dy = abs(y2 - y1);
	de = dx ? dy/(float)dx : 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			ApplyDecorationPoint(y, x, colR, colG, colB, colA, mode, cBrush);
		else
			ApplyDecorationPoint(x, y, colR, colG, colB, colA, mode, cBrush);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(rx+ry))
			{
				if (reverseXY)
					ApplyDecorationPoint(y, x, colR, colG, colB, colA, mode, cBrush);
				else
					ApplyDecorationPoint(x, y, colR, colG, colB, colA, mode, cBrush);
			}
			e -= 1.0f;
		}
	}
}

void Simulation::ApplyDecorationBox(int x1, int y1, int x2, int y2, int colR, int colG, int colB, int colA, int mode)
{
	int i, j;

	if (x1>x2)
	{
		i = x2;
		x2 = x1;
		x1 = i;
	}
	if (y1>y2)
	{
		j = y2;
		y2 = y1;
		y1 = j;
	}
	for (j=y1; j<=y2; j++)
		for (i=x1; i<=x2; i++)
			ApplyDecoration(i, j, colR, colG, colB, colA, mode);
}

bool Simulation::ColorCompare(Renderer *ren, int x, int y, int replaceR, int replaceG, int replaceB)
{
	auto pix = RGB<uint8_t>::Unpack(ren->GetPixel({ x, y }));
	int r = pix.Red;
	int g = pix.Green;
	int b = pix.Blue;
	int diff = std::abs(replaceR-r) + std::abs(replaceG-g) + std::abs(replaceB-b);
	return diff < 15;
}

void Simulation::ApplyDecorationFill(Renderer *ren, int x, int y, int colR, int colG, int colB, int colA, int replaceR, int replaceG, int replaceB)
{
	int x1, x2;
	char *bitmap = (char*)malloc(XRES*YRES); //Bitmap for checking
	if (!bitmap)
		return;
	memset(bitmap, 0, XRES*YRES);

	if (!ColorCompare(ren, x, y, replaceR, replaceG, replaceB)) {
		free(bitmap);
		return;
	}

	try
	{
		CoordStack& cs = getCoordStackSingleton();
		cs.clear();
		
		cs.push(x, y);
		do
		{
			cs.pop(x, y);
			x1 = x2 = x;
			// go left as far as possible
			while (x1>0)
			{
				if (bitmap[(x1-1)+y*XRES] || !ColorCompare(ren, x1-1, y, replaceR, replaceG, replaceB))
				{
					break;
				}
				x1--;
			}
			// go right as far as possible
			while (x2<XRES-1)
			{
				if (bitmap[(x1+1)+y*XRES] || !ColorCompare(ren, x2+1, y, replaceR, replaceG, replaceB))
				{
					break;
				}
				x2++;
			}
			// fill span
			for (x=x1; x<=x2; x++)
			{
				ApplyDecoration(x, y, colR, colG, colB, colA, DECO_DRAW);
				bitmap[x+y*XRES] = 1;
			}

			if (y >= 1)
				for (x=x1; x<=x2; x++)
					if (!bitmap[x+(y-1)*XRES] && ColorCompare(ren, x, y-1, replaceR, replaceG, replaceB))
						cs.push(x, y-1);

			if (y < YRES-1)
				for (x=x1; x<=x2; x++)
					if (!bitmap[x+(y+1)*XRES] && ColorCompare(ren, x, y+1, replaceR, replaceG, replaceB))
						cs.push(x, y+1);
		} while (cs.getSize() > 0);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		free(bitmap);
		return;
	}
	free(bitmap);
}

int Simulation::ToolBrush(int positionX, int positionY, int tool, Brush const &cBrush, float strength)
{
	for (ui::Point off : cBrush)
	{
		ui::Point coords = ui::Point(positionX, positionY) + off;
		if (coords.X >= 0 && coords.Y >= 0 && coords.X < XRES && coords.Y < YRES)
			Tool(coords.X, coords.Y, tool, positionX, positionY, strength);
	}
	return 0;
}

void Simulation::ToolLine(int x1, int y1, int x2, int y2, int tool, Brush const &cBrush, float strength)
{
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	int x, y, dx, dy, sy, rx = cBrush.GetRadius().X, ry = cBrush.GetRadius().Y;
	float e = 0.0f, de;
	if (reverseXY)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
	{
		y = x1;
		x1 = x2;
		x2 = y;
		y = y1;
		y1 = y2;
		y2 = y;
	}
	dx = x2 - x1;
	dy = abs(y2 - y1);
	de = dx ? dy/(float)dx : 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			ToolBrush(y, x, tool, cBrush, strength);
		else
			ToolBrush(x, y, tool, cBrush, strength);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(rx+ry) && ((y1<y2) ? (y<=y2) : (y>=y2)))
			{
				if (reverseXY)
					ToolBrush(y, x, tool, cBrush, strength);
				else
					ToolBrush(x, y, tool, cBrush, strength);
			}
			e -= 1.0f;
		}
	}
}

void Simulation::ToolBox(int x1, int y1, int x2, int y2, int tool, float strength)
{
	int brushX, brushY;
	brushX = ((x1 + x2) / 2);
	brushY = ((y1 + y2) / 2);
	int i, j;
	if (x1>x2)
	{
		i = x2;
		x2 = x1;
		x1 = i;
	}
	if (y1>y2)
	{
		j = y2;
		y2 = y1;
		y1 = j;
	}
	for (j=y1; j<=y2; j++)
		for (i=x1; i<=x2; i++)
			Tool(i, j, tool, brushX, brushY, strength);
}

int Simulation::CreateParts(int positionX, int positionY, int c, Brush const &cBrush, int flags)
{
	if (flags == -1)
		flags = replaceModeFlags;
	int radiusX = cBrush.GetRadius().X, radiusY = cBrush.GetRadius().Y;

	// special case for LIGH
	if (c == PT_LIGH)
	{
		if (currentTick < lightningRecreate)
			return 1;
		int newlife = radiusX + radiusY;
		if (newlife > 55)
			newlife = 55;
		c = PMAP(newlife, c);
		lightningRecreate = currentTick + std::max(newlife / 4, 1);
		return CreatePartFlags(positionX, positionY, c, flags);
	}
	else if (c == PT_TESC)
	{
		int newtmp = (radiusX*4+radiusY*4+7);
		if (newtmp > 300)
			newtmp = 300;
		c = PMAP(newtmp, c);
	}

	for (ui::Point off : cBrush)
	{
		ui::Point coords = ui::Point(positionX, positionY) + off;
		if (coords.X >= 0 && coords.Y >= 0 && coords.X < XRES && coords.Y < YRES)
			CreatePartFlags(coords.X, coords.Y, c, flags);
	}
	return 0;
}

int Simulation::CreateParts(int x, int y, int rx, int ry, int c, int flags)
{
	bool created = false;

	if (flags == -1)
		flags = replaceModeFlags;

	// special case for LIGH
	if (c == PT_LIGH)
	{
		if (currentTick < lightningRecreate)
			return 1;
		int newlife = rx + ry;
		if (newlife > 55)
			newlife = 55;
		c = PMAP(newlife, c);
		lightningRecreate = currentTick + std::max(newlife / 4, 1);
		rx = ry = 0;
	}
	else if (c == PT_TESC)
	{
		int newtmp = (rx*4+ry*4+7);
		if (newtmp > 300)
			newtmp = 300;
		c = PMAP(newtmp, c);
	}

	for (int j = -ry; j <= ry; j++)
		for (int i = -rx; i <= rx; i++)
			if (CreatePartFlags(x+i, y+j, c, flags))
				created = true;
	return !created;
}

void Simulation::CreateLine(int x1, int y1, int x2, int y2, int c, Brush const &cBrush, int flags)
{
	int x, y, dx, dy, sy, rx = cBrush.GetRadius().X, ry = cBrush.GetRadius().Y;
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	float e = 0.0f, de;
	if (reverseXY)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
	{
		y = x1;
		x1 = x2;
		x2 = y;
		y = y1;
		y1 = y2;
		y2 = y;
	}
	dx = x2 - x1;
	dy = abs(y2 - y1);
	de = dx ? dy/(float)dx : 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			CreateParts(y, x, c, cBrush, flags);
		else
			CreateParts(x, y, c, cBrush, flags);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(rx+ry) && ((y1<y2) ? (y<=y2) : (y>=y2)))
			{
				if (reverseXY)
					CreateParts(y, x, c, cBrush, flags);
				else
					CreateParts(x, y, c, cBrush, flags);
			}
			e -= 1.0f;
		}
	}
}

void Simulation::CreateBox(int x1, int y1, int x2, int y2, int c, int flags)
{
	int i, j;
	if (x1>x2)
	{
		i = x2;
		x2 = x1;
		x1 = i;
	}
	if (y1>y2)
	{
		j = y2;
		y2 = y1;
		y1 = j;
	}
	for (j=y2; j>=y1; j--)
		for (i=x1; i<=x2; i++)
			CreateParts(i, j, 0, 0, c, flags);
}

int Simulation::FloodParts(int x, int y, int fullc, int cm, int flags)
{
	int c = TYP(fullc);
	int x1, x2, dy = (c<PT_NUM)?1:CELL;
	int coord_stack_limit = XRES*YRES;
	unsigned short (*coord_stack)[2];
	int coord_stack_size = 0;
	int created_something = 0;

	// Bitmap for checking where we've already looked
	auto bitmapPtr = std::unique_ptr<char[]>(new char[XRES * YRES]);
	char *bitmap = bitmapPtr.get();
	std::fill(&bitmap[0], &bitmap[0] + XRES * YRES, 0);

	if (cm==-1)
	{
		//if initial flood point is out of bounds, do nothing
		if (c != 0 && (x < CELL || x >= XRES-CELL || y < CELL || y >= YRES-CELL || c == PT_SPRK))
			return 1;
		else if (x < 0 || x >= XRES || y < 0 || y >= YRES)
			return 1;
		
		if (c == 0)
		{
			cm = TYP(pmap[y][x]);
			if (!cm)
			{
				cm = TYP(photons[y][x]);
				if (!cm)
				{
					if (bmap[y/CELL][x/CELL])
						return FloodWalls(x, y, WL_ERASE, -1);
					else
						return -1;
				}
			}
		}
		else
			cm = 0;
	}
	
	if (c != 0 && IsWallBlocking(x, y, c))
		return 1;

	if (!FloodFillPmapCheck(x, y, cm))
		return 1;

	coord_stack = (short unsigned int (*)[2])malloc(sizeof(unsigned short)*2*coord_stack_limit);
	coord_stack[coord_stack_size][0] = x;
	coord_stack[coord_stack_size][1] = y;
	coord_stack_size++;

	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	do
	{
		coord_stack_size--;
		x = coord_stack[coord_stack_size][0];
		y = coord_stack[coord_stack_size][1];
		x1 = x2 = x;
		// go left as far as possible
		while (c?x1>CELL:x1>0)
		{
			if (bitmap[(y * XRES) + x1 - 1] || !FloodFillPmapCheck(x1-1, y, cm) || (c != 0 && IsWallBlocking(x1-1, y, c)))
			{
				break;
			}
			x1--;
		}
		// go right as far as possible
		while (c?x2<XRES-CELL-1:x2<XRES-1)
		{
			if (bitmap[(y * XRES) + x2 + 1] || !FloodFillPmapCheck(x2+1, y, cm) || (c != 0 && IsWallBlocking(x2+1, y, c)))
			{
				break;
			}
			x2++;
		}
		// fill span
		for (x=x1; x<=x2; x++)
		{
			if (!fullc)
			{
				if (elements[cm].Properties&TYPE_ENERGY)
				{
					if (photons[y][x])
					{
						kill_part(ID(photons[y][x]));
						created_something = 1;
					}
				}
				else if (pmap[y][x])
				{
					kill_part(ID(pmap[y][x]));
					created_something = 1;
				}
			}
			else if (CreateParts(x, y, 0, 0, fullc, flags))
				created_something = 1;
			bitmap[(y * XRES) + x] = 1;
		}

		if (c?y>=CELL+dy:y>=dy)
			for (x=x1; x<=x2; x++)
				if (!bitmap[((y - dy) * XRES) + x] && FloodFillPmapCheck(x, y-dy, cm) && (c == 0 || !IsWallBlocking(x, y-dy, c)))
				{
					coord_stack[coord_stack_size][0] = x;
					coord_stack[coord_stack_size][1] = y-dy;
					coord_stack_size++;
					if (coord_stack_size>=coord_stack_limit)
					{
						free(coord_stack);
						return -1;
					}
				}

		if (c?y<YRES-CELL-dy:y<YRES-dy)
			for (x=x1; x<=x2; x++)
				if (!bitmap[((y + dy) * XRES) + x] && FloodFillPmapCheck(x, y+dy, cm) && (c == 0 || !IsWallBlocking(x, y+dy, c)))
				{
					coord_stack[coord_stack_size][0] = x;
					coord_stack[coord_stack_size][1] = y+dy;
					coord_stack_size++;
					if (coord_stack_size>=coord_stack_limit)
					{
						free(coord_stack);
						return -1;
					}
				}
	} while (coord_stack_size>0);
	free(coord_stack);
	return created_something;
}
