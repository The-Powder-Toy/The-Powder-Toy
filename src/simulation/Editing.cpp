#include "Simulation.h"
#include "client/Client.h"
#include "ElementClasses.h"
#include "graphics/Renderer.h"
#include "gui/game/Brush.h"
#include <iostream>
#include <cmath>

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
						case 0: // sRGB
							pa = (pa <= 0.04045f) ? (pa / 12.92f) : pow((pa + 0.055f) / 1.055f, 2.4f);
							pr = (pr <= 0.04045f) ? (pr / 12.92f) : pow((pr + 0.055f) / 1.055f, 2.4f);
							pg = (pg <= 0.04045f) ? (pg / 12.92f) : pow((pg + 0.055f) / 1.055f, 2.4f);
							pb = (pb <= 0.04045f) ? (pb / 12.92f) : pow((pb + 0.055f) / 1.055f, 2.4f);
							break;

						case 1: // linear
							break;

						case 2: // Gamma = 2.2
							pa = pow(pa, 2.2f);
							pr = pow(pr, 2.2f);
							pg = pow(pg, 2.2f);
							pb = pow(pb, 2.2f);
							break;

						case 3: // Gamma = 1.8
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
			case 0: // sRGB
				ta = (ta <= 0.0031308f) ? (ta * 12.92f) : (1.055f * pow(ta, 1.f / 2.4f) - 0.055f);
				tr = (tr <= 0.0031308f) ? (tr * 12.92f) : (1.055f * pow(tr, 1.f / 2.4f) - 0.055f);
				tg = (tg <= 0.0031308f) ? (tg * 12.92f) : (1.055f * pow(tg, 1.f / 2.4f) - 0.055f);
				tb = (tb <= 0.0031308f) ? (tb * 12.92f) : (1.055f * pow(tb, 1.f / 2.4f) - 0.055f);
				break;

			case 1: // linear
				break;

			case 2: // Gamma = 2.2
				ta = pow(ta, 1.f / 2.2f);
				tr = pow(tr, 1.f / 2.2f);
				tg = pow(tg, 1.f / 2.2f);
				tb = pow(tb, 1.f / 2.2f);
				break;

			case 3: // Gamma = 1.8
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

void Simulation::ApplyDecorationPoint(int positionX, int positionY, int colR, int colG, int colB, int colA, int mode, Brush * cBrush)
{
	if(cBrush)
	{
		int radiusX = cBrush->GetRadius().X, radiusY = cBrush->GetRadius().Y, sizeX = cBrush->GetSize().X, sizeY = cBrush->GetSize().Y;
		unsigned char *bitmap = cBrush->GetBitmap();

		for(int y = 0; y < sizeY; y++)
		{
			for(int x = 0; x < sizeX; x++)
			{
				if(bitmap[(y*sizeX)+x] && (positionX+(x-radiusX) >= 0 && positionY+(y-radiusY) >= 0 && positionX+(x-radiusX) < XRES && positionY+(y-radiusY) < YRES))
				{
					ApplyDecoration(positionX+(x-radiusX), positionY+(y-radiusY), colR, colG, colB, colA, mode);
				}
			}
		}
	}
}

void Simulation::ApplyDecorationLine(int x1, int y1, int x2, int y2, int colR, int colG, int colB, int colA, int mode, Brush * cBrush)
{
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	int x, y, dx, dy, sy, rx = 0, ry = 0;
	float e = 0.0f, de;

	if(cBrush)
	{
		rx = cBrush->GetRadius().X;
		ry = cBrush->GetRadius().Y;
	}

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
	pixel pix = ren->vid[x+y*WINDOWW];
	int r = PIXR(pix);
	int g = PIXG(pix);
	int b = PIXB(pix);
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

int Simulation::ToolBrush(int positionX, int positionY, int tool, Brush * cBrush, float strength)
{
	if(cBrush)
	{
		int radiusX = cBrush->GetRadius().X, radiusY = cBrush->GetRadius().Y, sizeX = cBrush->GetSize().X, sizeY = cBrush->GetSize().Y;
		unsigned char *bitmap = cBrush->GetBitmap();
		for(int y = 0; y < sizeY; y++)
			for(int x = 0; x < sizeX; x++)
				if(bitmap[(y*sizeX)+x] && (positionX+(x-radiusX) >= 0 && positionY+(y-radiusY) >= 0 && positionX+(x-radiusX) < XRES && positionY+(y-radiusY) < YRES))
					Tool(positionX + (x - radiusX), positionY + (y - radiusY), tool, positionX, positionY, strength);
	}
	return 0;
}

void Simulation::ToolLine(int x1, int y1, int x2, int y2, int tool, Brush * cBrush, float strength)
{
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	int x, y, dx, dy, sy, rx = cBrush->GetRadius().X, ry = cBrush->GetRadius().Y;
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

int Simulation::CreateParts(int positionX, int positionY, int c, Brush * cBrush, int flags)
{
	if (flags == -1)
		flags = replaceModeFlags;
	if (cBrush)
	{
		int radiusX = cBrush->GetRadius().X, radiusY = cBrush->GetRadius().Y, sizeX = cBrush->GetSize().X, sizeY = cBrush->GetSize().Y;
		unsigned char *bitmap = cBrush->GetBitmap();

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

		for (int y = sizeY-1; y >=0; y--)
		{
			for (int x = 0; x < sizeX; x++)
			{
				if (bitmap[(y*sizeX)+x] && (positionX+(x-radiusX) >= 0 && positionY+(y-radiusY) >= 0 && positionX+(x-radiusX) < XRES && positionY+(y-radiusY) < YRES))
				{
					CreatePartFlags(positionX+(x-radiusX), positionY+(y-radiusY), c, flags);
				}
			}
		}
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

void Simulation::CreateLine(int x1, int y1, int x2, int y2, int c, Brush * cBrush, int flags)
{
	int x, y, dx, dy, sy, rx = cBrush->GetRadius().X, ry = cBrush->GetRadius().Y;
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
