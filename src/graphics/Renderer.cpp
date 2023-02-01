#include "Renderer.h"
#include "Misc.h"
#include "common/tpt-rand.h"
#include "common/tpt-compat.h"
#include "simulation/Simulation.h"
#include "simulation/ElementGraphics.h"
#include "simulation/ElementClasses.h"
#include "simulation/Air.h"
#include "simulation/gravity/Gravity.h"
#include <cmath>

constexpr auto VIDXRES = WINDOWW;
// constexpr auto VIDYRES = WINDOWH; // not actually used anywhere

VideoBuffer * Renderer::WallIcon(int wallID, int width, int height)
{
	static std::vector<wall_type> Renderer_wtypes = LoadWalls();
	int i, j;
	int wt = wallID;
	if (wt<0 || wt>=(int)Renderer_wtypes.size())
		return 0;
	wall_type *wtypes = Renderer_wtypes.data();
	pixel pc = wtypes[wt].colour;
	pixel gc = wtypes[wt].eglow;
	VideoBuffer * newTexture = new VideoBuffer(width, height);
	if (wtypes[wt].drawstyle==1)
	{
		for (j=0; j<height; j+=2)
			for (i=(j>>1)&1; i<width; i+=2)
				newTexture->SetPixel(i, j, PIXR(pc), PIXG(pc), PIXB(pc), 255);
	}
	else if (wtypes[wt].drawstyle==2)
	{
		for (j=0; j<height; j+=2)
			for (i=0; i<width; i+=2)
				newTexture->SetPixel(i, j, PIXR(pc), PIXG(pc), PIXB(pc), 255);
	}
	else if (wtypes[wt].drawstyle==3)
	{
		for (j=0; j<height; j++)
			for (i=0; i<width; i++)
				newTexture->SetPixel(i, j, PIXR(pc), PIXG(pc), PIXB(pc), 255);
	}
	else if (wtypes[wt].drawstyle==4)
	{
		for (j=0; j<height; j++)
			for (i=0; i<width; i++)
				if(i%CELL == j%CELL)
					newTexture->SetPixel(i, j, PIXR(pc), PIXG(pc), PIXB(pc), 255);
				else if  (i%CELL == (j%CELL)+1 || (i%CELL == 0 && j%CELL == CELL-1))
					newTexture->SetPixel(i, j, PIXR(gc), PIXG(gc), PIXB(gc), 255);
				else
					newTexture->SetPixel(i, j, 0x20, 0x20, 0x20, 255);
	}

	// special rendering for some walls
	if (wt==WL_EWALL)
	{
		for (j=0; j<height; j++)
		{
			for (i=0; i<(width/4)+j; i++)
			{
				if (!(i&j&1))
					newTexture->SetPixel(i, j, PIXR(pc), PIXG(pc), PIXB(pc), 255);
			}
			for (; i<width; i++)
			{
				if (i&j&1)
					newTexture->SetPixel(i, j, PIXR(pc), PIXG(pc), PIXB(pc), 255);
			}
		}
	}
	else if (wt==WL_WALLELEC)
	{
		for (j=0; j<height; j++)
			for (i=0; i<width; i++)
			{
				if (!(j%2) && !(i%2))
					newTexture->SetPixel(i, j, PIXR(pc), PIXG(pc), PIXB(pc), 255);
				else
					newTexture->SetPixel(i, j, 0x80, 0x80, 0x80, 255);
			}
	}
	else if (wt==WL_EHOLE || wt==WL_STASIS)
	{
		for (j=0; j<height; j++)
		{
			for (i=0; i<(width/4)+j; i++)
			{
				if (i&j&1)
					newTexture->SetPixel(i, j, PIXR(pc), PIXG(pc), PIXB(pc), 255);
			}
			for (; i<width; i++)
			{
				if (!(i&j&1))
					newTexture->SetPixel(i, j, PIXR(pc), PIXG(pc), PIXB(pc), 255);
			}
		}
	}
	else if (wt == WL_ERASE)
	{
		for (j=0; j<height; j+=2)
		{
			for (i=1+(1&(j>>1)); i<width/2; i+=2)
			{
				newTexture->SetPixel(i, j, PIXR(pc), PIXG(pc), PIXB(pc), 255);
			}
		}
		for (j=0; j<height; j++)
		{
			for (i=width/2; i<width; i++)
			{
				newTexture->SetPixel(i, j, PIXR(pc), PIXG(pc), PIXB(pc), 255);
			}
		}
		for (j=3; j<(width-4)/2; j++)
		{
			newTexture->SetPixel(j+6, j, 0xFF, 0, 0, 255);
			newTexture->SetPixel(j+7, j, 0xFF, 0, 0, 255);
			newTexture->SetPixel(-j+19, j, 0xFF, 0, 0, 255);
			newTexture->SetPixel(-j+20, j, 0xFF, 0, 0, 255);
		}
	}
	else if (wt == WL_ERASEALL)
	{
		for (int j = 0; j < height; j++)
		{
			int r = 100, g = 150, b = 50;
			int rd = 1, gd = -1, bd = -1;
			for (int i = 0; i < width; i++)
			{
				r += 15*rd;
				g += 15*gd;
				b += 15*bd;
				if (r > 200) rd = -1;
				if (g > 200) gd = -1;
				if (b > 200) bd = -1;
				if (r < 15) rd = 1;
				if (g < 15) gd = 1;
				if (b < 15) bd = 1;
				int rc = std::min(150, std::max(0, r));
				int gc = std::min(200, std::max(0, g));
				int bc = std::min(200, std::max(0, b));
				newTexture->SetPixel(i, j, rc, gc, bc, 255);
			}
		}
		for (int j = 3; j < (width-4)/2; j++)
		{
			newTexture->SetPixel(j+0, j, 0xFF, 0, 0, 255);
			newTexture->SetPixel(j+1, j, 0xFF, 0, 0, 255);
			newTexture->SetPixel(-j+13, j, 0xFF, 0, 0, 255);
			newTexture->SetPixel(-j+14, j, 0xFF, 0, 0, 255);

			newTexture->SetPixel(j+11, j, 0xFF, 0, 0, 255);
			newTexture->SetPixel(j+12, j, 0xFF, 0, 0, 255);
			newTexture->SetPixel(-j+24, j, 0xFF, 0, 0, 255);
			newTexture->SetPixel(-j+25, j, 0xFF, 0, 0, 255);
		}
	}
	else if(wt == WL_STREAM)
	{
		for (j=0; j<height; j++)
		{
			for (i=0; i<width; i++)
			{
				pc =  i==0||i==width-1||j==0||j==height-1 ? PIXPACK(0xA0A0A0) : PIXPACK(0x000000);
				newTexture->SetPixel(i, j, PIXR(pc), PIXG(pc), PIXB(pc), 255);
			}
		}
		newTexture->AddCharacter(4, 2, 0xE00D, 255, 255, 255, 255);
		for (i=width/3; i<width; i++)
		{
			newTexture->SetPixel(i, 7+(int)(3.9f*cos(i*0.3f)), 255, 255, 255, 255);
		}
	}
	return newTexture;
}

void Renderer::DrawSigns()
{
	int x, y, w, h;
	std::vector<sign> signs = sim->signs;
	for (auto &currentSign : signs)
	{
		if (currentSign.text.length())
		{
			String text = currentSign.getDisplayText(sim, x, y, w, h);
			clearrect(x, y, w+1, h);
			drawrect(x, y, w+1, h, 192, 192, 192, 255);
			drawtext(x+3, y+4, text, 255, 255, 255, 255);

			if (currentSign.ju != sign::None)
			{
				int x = currentSign.x;
				int y = currentSign.y;
				int dx = 1 - currentSign.ju;
				int dy = (currentSign.y > 18) ? -1 : 1;
				for (int j = 0; j < 4; j++)
				{
					blendpixel(x, y, 192, 192, 192, 255);
					x += dx;
					y += dy;
				}
			}
		}
	}
}

void Renderer::render_parts()
{
	int deca, decr, decg, decb, cola, colr, colg, colb, firea, firer, fireg, fireb, pixel_mode, q, i, t, nx, ny, x, y;
	int orbd[4] = {0, 0, 0, 0}, orbl[4] = {0, 0, 0, 0};
	float gradv, flicker;
	Particle * parts;
	Element *elements;
	if(!sim)
		return;
	parts = sim->parts;
	elements = sim->elements.data();
	if (gridSize)//draws the grid
	{
		for (ny=0; ny<YRES; ny++)
			for (nx=0; nx<XRES; nx++)
			{
				if (ny%(4*gridSize) == 0)
					blendpixel(nx, ny, 100, 100, 100, 80);
				if (nx%(4*gridSize) == 0 && ny%(4*gridSize) != 0)
					blendpixel(nx, ny, 100, 100, 100, 80);
			}
	}
	foundElements = 0;
	for(i = 0; i<=sim->parts_lastActiveIndex; i++) {
		if (sim->parts[i].type && sim->parts[i].type >= 0 && sim->parts[i].type < PT_NUM) {
			t = sim->parts[i].type;

			nx = (int)(sim->parts[i].x+0.5f);
			ny = (int)(sim->parts[i].y+0.5f);

			if(nx >= XRES || nx < 0 || ny >= YRES || ny < 0)
				continue;
			if(TYP(sim->photons[ny][nx]) && !(sim->elements[t].Properties & TYPE_ENERGY) && t!=PT_STKM && t!=PT_STKM2 && t!=PT_FIGH)
				continue;

			//Defaults
			pixel_mode = 0 | PMODE_FLAT;
			cola = 255;
			colr = PIXR(elements[t].Colour);
			colg = PIXG(elements[t].Colour);
			colb = PIXB(elements[t].Colour);
			firer = fireg = fireb = firea = 0;

			deca = (sim->parts[i].dcolour>>24)&0xFF;
			decr = (sim->parts[i].dcolour>>16)&0xFF;
			decg = (sim->parts[i].dcolour>>8)&0xFF;
			decb = (sim->parts[i].dcolour)&0xFF;

			if(decorations_enable && blackDecorations)
			{
				if(deca < 250 || decr > 5 || decg > 5 || decb > 5)
					deca = 0;
				else
				{
					deca = 255;
					decr = decg = decb = 0;
				}
			}

			{
				if (graphicscache[t].isready)
				{
					pixel_mode = graphicscache[t].pixel_mode;
					cola = graphicscache[t].cola;
					colr = graphicscache[t].colr;
					colg = graphicscache[t].colg;
					colb = graphicscache[t].colb;
					firea = graphicscache[t].firea;
					firer = graphicscache[t].firer;
					fireg = graphicscache[t].fireg;
					fireb = graphicscache[t].fireb;
				}
				else if(!(colour_mode & COLOUR_BASC))
				{
					if (!elements[t].Graphics || (*(elements[t].Graphics))(this, &(sim->parts[i]), nx, ny, &pixel_mode, &cola, &colr, &colg, &colb, &firea, &firer, &fireg, &fireb)) //That's a lot of args, a struct might be better
					{
						graphicscache[t].isready = 1;
						graphicscache[t].pixel_mode = pixel_mode;
						graphicscache[t].cola = cola;
						graphicscache[t].colr = colr;
						graphicscache[t].colg = colg;
						graphicscache[t].colb = colb;
						graphicscache[t].firea = firea;
						graphicscache[t].firer = firer;
						graphicscache[t].fireg = fireg;
						graphicscache[t].fireb = fireb;
					}
				}
				if((elements[t].Properties & PROP_HOT_GLOW) && sim->parts[i].temp>(elements[t].HighTemperature-800.0f))
				{
					gradv = 3.1415/(2*elements[t].HighTemperature-(elements[t].HighTemperature-800.0f));
					auto caddress = int((sim->parts[i].temp>elements[t].HighTemperature)?elements[t].HighTemperature-(elements[t].HighTemperature-800.0f):sim->parts[i].temp-(elements[t].HighTemperature-800.0f));
					colr += int(sin(gradv*caddress) * 226);
					colg += int(sin(gradv*caddress*4.55 +TPT_PI_DBL) * 34);
					colb += int(sin(gradv*caddress*2.22 +TPT_PI_DBL) * 64);
				}

				if((pixel_mode & FIRE_ADD) && !(render_mode & FIRE_ADD))
					pixel_mode |= PMODE_GLOW;
				if((pixel_mode & FIRE_BLEND) && !(render_mode & FIRE_BLEND))
					pixel_mode |= PMODE_BLUR;
				if((pixel_mode & PMODE_BLUR) && !(render_mode & PMODE_BLUR))
					pixel_mode |= PMODE_FLAT;
				if((pixel_mode & PMODE_GLOW) && !(render_mode & PMODE_GLOW))
					pixel_mode |= PMODE_BLEND;
				if (render_mode & PMODE_BLOB)
					pixel_mode |= PMODE_BLOB;

				pixel_mode &= render_mode;

				//Alter colour based on display mode
				if(colour_mode & COLOUR_HEAT)
				{
					constexpr float min_temp = MIN_TEMP;
					constexpr float max_temp = MAX_TEMP;
					firea = 255;
					auto color = heatTableAt(int((sim->parts[i].temp - min_temp) / (max_temp - min_temp) * 1024));
					firer = colr = PIXR(color);
					fireg = colg = PIXG(color);
					fireb = colb = PIXB(color);
					cola = 255;
					if(pixel_mode & (FIREMODE | PMODE_GLOW))
						pixel_mode = (pixel_mode & ~(FIREMODE|PMODE_GLOW)) | PMODE_BLUR;
					else if ((pixel_mode & (PMODE_BLEND | PMODE_ADD)) == (PMODE_BLEND | PMODE_ADD))
						pixel_mode = (pixel_mode & ~(PMODE_BLEND|PMODE_ADD)) | PMODE_FLAT;
					else if (!pixel_mode)
						pixel_mode |= PMODE_FLAT;
				}
				else if(colour_mode & COLOUR_LIFE)
				{
					gradv = 0.4f;
					if (!(sim->parts[i].life<5))
						q = int(sqrt((float)sim->parts[i].life));
					else
						q = sim->parts[i].life;
					colr = colg = colb = int(sin(gradv*q) * 100 + 128);
					cola = 255;
					if(pixel_mode & (FIREMODE | PMODE_GLOW))
						pixel_mode = (pixel_mode & ~(FIREMODE|PMODE_GLOW)) | PMODE_BLUR;
					else if ((pixel_mode & (PMODE_BLEND | PMODE_ADD)) == (PMODE_BLEND | PMODE_ADD))
						pixel_mode = (pixel_mode & ~(PMODE_BLEND|PMODE_ADD)) | PMODE_FLAT;
					else if (!pixel_mode)
						pixel_mode |= PMODE_FLAT;
				}
				else if(colour_mode & COLOUR_BASC)
				{
					colr = PIXR(elements[t].Colour);
					colg = PIXG(elements[t].Colour);
					colb = PIXB(elements[t].Colour);
					pixel_mode = PMODE_FLAT;
				}

				//Apply decoration colour
				if(!(colour_mode & ~COLOUR_GRAD) && decorations_enable && deca)
				{
					deca++;
					if(!(pixel_mode & NO_DECO))
					{
						colr = (deca*decr + (256-deca)*colr) >> 8;
						colg = (deca*decg + (256-deca)*colg) >> 8;
						colb = (deca*decb + (256-deca)*colb) >> 8;
					}

					if(pixel_mode & DECO_FIRE)
					{
						firer = (deca*decr + (256-deca)*firer) >> 8;
						fireg = (deca*decg + (256-deca)*fireg) >> 8;
						fireb = (deca*decb + (256-deca)*fireb) >> 8;
					}
				}

				if (colour_mode & COLOUR_GRAD)
				{
					auto frequency = 0.05f;
					auto q = int(sim->parts[i].temp-40);
					colr = int(sin(frequency*q) * 16 + colr);
					colg = int(sin(frequency*q) * 16 + colg);
					colb = int(sin(frequency*q) * 16 + colb);
					if(pixel_mode & (FIREMODE | PMODE_GLOW)) pixel_mode = (pixel_mode & ~(FIREMODE|PMODE_GLOW)) | PMODE_BLUR;
				}

				//All colours are now set, check ranges
				if(colr>255) colr = 255;
				else if(colr<0) colr = 0;
				if(colg>255) colg = 255;
				else if(colg<0) colg = 0;
				if(colb>255) colb = 255;
				else if(colb<0) colb = 0;
				if(cola>255) cola = 255;
				else if(cola<0) cola = 0;

				if(firer>255) firer = 255;
				else if(firer<0) firer = 0;
				if(fireg>255) fireg = 255;
				else if(fireg<0) fireg = 0;
				if(fireb>255) fireb = 255;
				else if(fireb<0) fireb = 0;
				if(firea>255) firea = 255;
				else if(firea<0) firea = 0;

				if (findingElement)
				{
					if (TYP(findingElement) == parts[i].type &&
							(parts[i].type != PT_LIFE || (ID(findingElement) == parts[i].ctype)))
					{
						colr = firer = 255;
						colg = fireg = colb = fireb = 0;
						foundElements++;
					}
					else
					{
						colr /= 10;
						colg /= 10;
						colb /= 10;
						firer /= 5;
						fireg /= 5;
						fireb /= 5;
					}
				}

				//Pixel rendering
				if (pixel_mode & EFFECT_LINES)
				{
					if (t==PT_SOAP)
					{
						if ((parts[i].ctype&3) == 3 && parts[i].tmp >= 0 && parts[i].tmp < NPART)
							draw_line(nx, ny, (int)(parts[parts[i].tmp].x+0.5f), (int)(parts[parts[i].tmp].y+0.5f), colr, colg, colb, cola);
					}
				}
				if(pixel_mode & PSPEC_STICKMAN)
				{
					int legr, legg, legb;
					playerst *cplayer;
					if(t==PT_STKM)
						cplayer = &sim->player;
					else if(t==PT_STKM2)
						cplayer = &sim->player2;
					else if (t==PT_FIGH && sim->parts[i].tmp >= 0 && sim->parts[i].tmp < MAX_FIGHTERS)
						cplayer = &sim->fighters[(unsigned char)sim->parts[i].tmp];
					else
						continue;

					if (mousePos.X>(nx-3) && mousePos.X<(nx+3) && mousePos.Y<(ny+3) && mousePos.Y>(ny-3)) //If mouse is in the head
					{
						String hp = String::Build(Format::Width(sim->parts[i].life, 3));
						drawtext(mousePos.X-8-2*(sim->parts[i].life<100)-2*(sim->parts[i].life<10), mousePos.Y-12, hp, 255, 255, 255, 255);
					}

					if (findingElement == t)
					{
						colr = 255;
						colg = colb = 0;
					}
					else if (colour_mode != COLOUR_HEAT)
					{
						if (cplayer->fan)
						{
							colr = PIXR(0x8080FF);
							colg = PIXG(0x8080FF);
							colb = PIXB(0x8080FF);
						}
						else if (cplayer->elem < PT_NUM && cplayer->elem > 0)
						{
							colr = PIXR(elements[cplayer->elem].Colour);
							colg = PIXG(elements[cplayer->elem].Colour);
							colb = PIXB(elements[cplayer->elem].Colour);
						}
						else
						{
							colr = 0x80;
							colg = 0x80;
							colb = 0xFF;
						}
					}

					if (findingElement && findingElement == t)
					{
						legr = 255;
						legg = legb = 0;
					}
					else if (colour_mode==COLOUR_HEAT)
					{
						legr = colr;
						legg = colg;
						legb = colb;
					}
					else if (t==PT_STKM2)
					{
						legr = 100;
						legg = 100;
						legb = 255;
					}
					else
					{
						legr = 255;
						legg = 255;
						legb = 255;
					}

					if (findingElement && findingElement != t)
					{
						colr /= 10;
						colg /= 10;
						colb /= 10;
						legr /= 10;
						legg /= 10;
						legb /= 10;
					}

					//head
					if(t==PT_FIGH)
					{
						draw_line(nx, ny+2, nx+2, ny, colr, colg, colb, 255);
						draw_line(nx+2, ny, nx, ny-2, colr, colg, colb, 255);
						draw_line(nx, ny-2, nx-2, ny, colr, colg, colb, 255);
						draw_line(nx-2, ny, nx, ny+2, colr, colg, colb, 255);
					}
					else
					{
						draw_line(nx-2, ny+2, nx+2, ny+2, colr, colg, colb, 255);
						draw_line(nx-2, ny-2, nx+2, ny-2, colr, colg, colb, 255);
						draw_line(nx-2, ny-2, nx-2, ny+2, colr, colg, colb, 255);
						draw_line(nx+2, ny-2, nx+2, ny+2, colr, colg, colb, 255);
					}
					//legs
					draw_line(nx, ny+3, int(cplayer->legs[0]), int(cplayer->legs[1]), legr, legg, legb, 255);
					draw_line(int(cplayer->legs[0]), int(cplayer->legs[1]), int(cplayer->legs[4]), int(cplayer->legs[5]), legr, legg, legb, 255);
					draw_line(nx, ny+3, int(cplayer->legs[8]), int(cplayer->legs[9]), legr, legg, legb, 255);
					draw_line(int(cplayer->legs[8]), int(cplayer->legs[9]), int(cplayer->legs[12]), int(cplayer->legs[13]), legr, legg, legb, 255);
					if (cplayer->rocketBoots)
					{
						for (int leg=0; leg<2; leg++)
						{
							int nx = int(cplayer->legs[leg*8+4]), ny = int(cplayer->legs[leg*8+5]);
							int colr = 255, colg = 0, colb = 255;
							if (((int)(cplayer->comm)&0x04) == 0x04 || (((int)(cplayer->comm)&0x01) == 0x01 && leg==0) || (((int)(cplayer->comm)&0x02) == 0x02 && leg==1))
								blendpixel(nx, ny, 0, 255, 0, 255);
							else
								blendpixel(nx, ny, 255, 0, 0, 255);
							blendpixel(nx+1, ny, colr, colg, colb, 223);
							blendpixel(nx-1, ny, colr, colg, colb, 223);
							blendpixel(nx, ny+1, colr, colg, colb, 223);
							blendpixel(nx, ny-1, colr, colg, colb, 223);

							blendpixel(nx+1, ny-1, colr, colg, colb, 112);
							blendpixel(nx-1, ny-1, colr, colg, colb, 112);
							blendpixel(nx+1, ny+1, colr, colg, colb, 112);
							blendpixel(nx-1, ny+1, colr, colg, colb, 112);
						}
					}
				}
				if(pixel_mode & PMODE_FLAT)
				{
					vid[ny*(VIDXRES)+nx] = PIXRGB(colr,colg,colb);
				}
				if(pixel_mode & PMODE_BLEND)
				{
					blendpixel(nx, ny, colr, colg, colb, cola);
				}
				if(pixel_mode & PMODE_ADD)
				{
					addpixel(nx, ny, colr, colg, colb, cola);
				}
				if(pixel_mode & PMODE_BLOB)
				{
					vid[ny*(VIDXRES)+nx] = PIXRGB(colr,colg,colb);

					blendpixel(nx+1, ny, colr, colg, colb, 223);
					blendpixel(nx-1, ny, colr, colg, colb, 223);
					blendpixel(nx, ny+1, colr, colg, colb, 223);
					blendpixel(nx, ny-1, colr, colg, colb, 223);

					blendpixel(nx+1, ny-1, colr, colg, colb, 112);
					blendpixel(nx-1, ny-1, colr, colg, colb, 112);
					blendpixel(nx+1, ny+1, colr, colg, colb, 112);
					blendpixel(nx-1, ny+1, colr, colg, colb, 112);
				}
				if(pixel_mode & PMODE_GLOW)
				{
					int cola1 = (5*cola)/255;
					addpixel(nx, ny, colr, colg, colb, (192*cola)/255);
					addpixel(nx+1, ny, colr, colg, colb, (96*cola)/255);
					addpixel(nx-1, ny, colr, colg, colb, (96*cola)/255);
					addpixel(nx, ny+1, colr, colg, colb, (96*cola)/255);
					addpixel(nx, ny-1, colr, colg, colb, (96*cola)/255);

					for (x = 1; x < 6; x++) {
						addpixel(nx, ny-x, colr, colg, colb, cola1);
						addpixel(nx, ny+x, colr, colg, colb, cola1);
						addpixel(nx-x, ny, colr, colg, colb, cola1);
						addpixel(nx+x, ny, colr, colg, colb, cola1);
						for (y = 1; y < 6; y++) {
							if(x + y > 7)
								continue;
							addpixel(nx+x, ny-y, colr, colg, colb, cola1);
							addpixel(nx-x, ny+y, colr, colg, colb, cola1);
							addpixel(nx+x, ny+y, colr, colg, colb, cola1);
							addpixel(nx-x, ny-y, colr, colg, colb, cola1);
						}
					}
				}
				if(pixel_mode & PMODE_BLUR)
				{
					for (x=-3; x<4; x++)
					{
						for (y=-3; y<4; y++)
						{
							if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
								blendpixel(x+nx, y+ny, colr, colg, colb, 30);
							if (abs(x)+abs(y) <=3 && abs(x)+abs(y))
								blendpixel(x+nx, y+ny, colr, colg, colb, 20);
							if (abs(x)+abs(y) == 2)
								blendpixel(x+nx, y+ny, colr, colg, colb, 10);
						}
					}
				}
				if(pixel_mode & PMODE_SPARK)
				{
					flicker = float(random_gen()%20);
					gradv = 4*sim->parts[i].life + flicker;
					for (x = 0; gradv>0.5; x++) {
						addpixel(nx+x, ny, colr, colg, colb, int(gradv));
						addpixel(nx-x, ny, colr, colg, colb, int(gradv));

						addpixel(nx, ny+x, colr, colg, colb, int(gradv));
						addpixel(nx, ny-x, colr, colg, colb, int(gradv));
						gradv = gradv/1.5f;
					}
				}
				if(pixel_mode & PMODE_FLARE)
				{
					flicker = float(random_gen()%20);
					gradv = flicker + fabs(parts[i].vx)*17 + fabs(sim->parts[i].vy)*17;
					blendpixel(nx, ny, colr, colg, colb, int((gradv*4)>255?255:(gradv*4)) );
					blendpixel(nx+1, ny, colr, colg, colb,int( (gradv*2)>255?255:(gradv*2)) );
					blendpixel(nx-1, ny, colr, colg, colb, int((gradv*2)>255?255:(gradv*2)) );
					blendpixel(nx, ny+1, colr, colg, colb, int((gradv*2)>255?255:(gradv*2)) );
					blendpixel(nx, ny-1, colr, colg, colb, int((gradv*2)>255?255:(gradv*2)) );
					if (gradv>255) gradv=255;
					blendpixel(nx+1, ny-1, colr, colg, colb, int(gradv));
					blendpixel(nx-1, ny-1, colr, colg, colb, int(gradv));
					blendpixel(nx+1, ny+1, colr, colg, colb, int(gradv));
					blendpixel(nx-1, ny+1, colr, colg, colb, int(gradv));
					for (x = 1; gradv>0.5; x++) {
						addpixel(nx+x, ny, colr, colg, colb, int(gradv));
						addpixel(nx-x, ny, colr, colg, colb, int(gradv));
						addpixel(nx, ny+x, colr, colg, colb, int(gradv));
						addpixel(nx, ny-x, colr, colg, colb, int(gradv));
						gradv = gradv/1.2f;
					}
				}
				if(pixel_mode & PMODE_LFLARE)
				{
					flicker = float(random_gen()%20);
					gradv = flicker + fabs(parts[i].vx)*17 + fabs(parts[i].vy)*17;
					blendpixel(nx, ny, colr, colg, colb, int((gradv*4)>255?255:(gradv*4)) );
					blendpixel(nx+1, ny, colr, colg, colb, int((gradv*2)>255?255:(gradv*2)) );
					blendpixel(nx-1, ny, colr, colg, colb, int((gradv*2)>255?255:(gradv*2)) );
					blendpixel(nx, ny+1, colr, colg, colb, int((gradv*2)>255?255:(gradv*2)) );
					blendpixel(nx, ny-1, colr, colg, colb, int((gradv*2)>255?255:(gradv*2)) );
					if (gradv>255) gradv=255;
					blendpixel(nx+1, ny-1, colr, colg, colb, int(gradv));
					blendpixel(nx-1, ny-1, colr, colg, colb, int(gradv));
					blendpixel(nx+1, ny+1, colr, colg, colb, int(gradv));
					blendpixel(nx-1, ny+1, colr, colg, colb, int(gradv));
					for (x = 1; gradv>0.5; x++) {
						addpixel(nx+x, ny, colr, colg, colb, int(gradv));
						addpixel(nx-x, ny, colr, colg, colb, int(gradv));
						addpixel(nx, ny+x, colr, colg, colb, int(gradv));
						addpixel(nx, ny-x, colr, colg, colb, int(gradv));
						gradv = gradv/1.01f;
					}
				}
				if (pixel_mode & EFFECT_GRAVIN)
				{
					int nxo = 0;
					int nyo = 0;
					int r;
					float drad = 0.0f;
					float ddist = 0.0f;
					sim->orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
					for (r = 0; r < 4; r++) {
						ddist = ((float)orbd[r])/16.0f;
						drad = (TPT_PI_FLT * ((float)orbl[r]) / 180.0f)*1.41f;
						nxo = (int)(ddist*cos(drad));
						nyo = (int)(ddist*sin(drad));
						if (ny+nyo>0 && ny+nyo<YRES && nx+nxo>0 && nx+nxo<XRES && TYP(sim->pmap[ny+nyo][nx+nxo]) != PT_PRTI)
							addpixel(nx+nxo, ny+nyo, colr, colg, colb, 255-orbd[r]);
					}
				}
				if (pixel_mode & EFFECT_GRAVOUT)
				{
					int nxo = 0;
					int nyo = 0;
					int r;
					float drad = 0.0f;
					float ddist = 0.0f;
					sim->orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
					for (r = 0; r < 4; r++) {
						ddist = ((float)orbd[r])/16.0f;
						drad = (TPT_PI_FLT * ((float)orbl[r]) / 180.0f)*1.41f;
						nxo = (int)(ddist*cos(drad));
						nyo = (int)(ddist*sin(drad));
						if (ny+nyo>0 && ny+nyo<YRES && nx+nxo>0 && nx+nxo<XRES && TYP(sim->pmap[ny+nyo][nx+nxo]) != PT_PRTO)
							addpixel(nx+nxo, ny+nyo, colr, colg, colb, 255-orbd[r]);
					}
				}
				if (pixel_mode & EFFECT_DBGLINES && !(display_mode&DISPLAY_PERS))
				{
					// draw lines connecting wifi/portal channels
					if (mousePos.X == nx && mousePos.Y == ny && i == ID(sim->pmap[ny][nx]) && debugLines)
					{
						int type = parts[i].type, tmp = (int)((parts[i].temp-73.15f)/100+1), othertmp;
						if (type == PT_PRTI)
							type = PT_PRTO;
						else if (type == PT_PRTO)
							type = PT_PRTI;
						for (int z = 0; z <= sim->parts_lastActiveIndex; z++)
						{
							if (parts[z].type == type)
							{
								othertmp = (int)((parts[z].temp-73.15f)/100+1);
								if (tmp == othertmp)
									xor_line(nx,ny,(int)(parts[z].x+0.5f),(int)(parts[z].y+0.5f));
							}
						}
					}
				}
				//Fire effects
				if(firea && (pixel_mode & FIRE_BLEND))
				{
					firea /= 2;
					fire_r[ny/CELL][nx/CELL] = (firea*firer + (255-firea)*fire_r[ny/CELL][nx/CELL]) >> 8;
					fire_g[ny/CELL][nx/CELL] = (firea*fireg + (255-firea)*fire_g[ny/CELL][nx/CELL]) >> 8;
					fire_b[ny/CELL][nx/CELL] = (firea*fireb + (255-firea)*fire_b[ny/CELL][nx/CELL]) >> 8;
				}
				if(firea && (pixel_mode & FIRE_ADD))
				{
					firea /= 8;
					firer = ((firea*firer) >> 8) + fire_r[ny/CELL][nx/CELL];
					fireg = ((firea*fireg) >> 8) + fire_g[ny/CELL][nx/CELL];
					fireb = ((firea*fireb) >> 8) + fire_b[ny/CELL][nx/CELL];

					if(firer>255)
						firer = 255;
					if(fireg>255)
						fireg = 255;
					if(fireb>255)
						fireb = 255;

					fire_r[ny/CELL][nx/CELL] = firer;
					fire_g[ny/CELL][nx/CELL] = fireg;
					fire_b[ny/CELL][nx/CELL] = fireb;
				}
				if(firea && (pixel_mode & FIRE_SPARK))
				{
					firea /= 4;
					fire_r[ny/CELL][nx/CELL] = (firea*firer + (255-firea)*fire_r[ny/CELL][nx/CELL]) >> 8;
					fire_g[ny/CELL][nx/CELL] = (firea*fireg + (255-firea)*fire_g[ny/CELL][nx/CELL]) >> 8;
					fire_b[ny/CELL][nx/CELL] = (firea*fireb + (255-firea)*fire_b[ny/CELL][nx/CELL]) >> 8;
				}
			}
		}
	}
}

void Renderer::draw_other() // EMP effect
{
	int i, j;
	int emp_decor = sim->emp_decor;
	if (emp_decor>40) emp_decor = 40;
	if (emp_decor<0) emp_decor = 0;
	if (!(render_mode & EFFECT)) // not in nothing mode
		return;
	if (emp_decor>0)
	{
		int r=int(emp_decor*2.5), g=int(100+emp_decor*1.5), b=255;
		int a=int((1.0*emp_decor/110)*255);
		if (r>255) r=255;
		if (g>255) g=255;
		if (b>255) g=255;
		if (a>255) a=255;
		for (j=0; j<YRES; j++)
			for (i=0; i<XRES; i++)
			{
				blendpixel(i, j, r, g, b, a);
			}
	}
}

void Renderer::draw_grav_zones()
{
	if(!gravityZonesEnabled)
		return;

	int x, y, i, j;
	for (y=0; y<YCELLS; y++)
	{
		for (x=0; x<XCELLS; x++)
		{
			if(sim->grav->gravmask[y*XCELLS+x])
			{
				for (j=0; j<CELL; j++)//draws the colors
					for (i=0; i<CELL; i++)
						if(i == j)
							blendpixel(x*CELL+i, y*CELL+j, 255, 200, 0, 120);
						else
							blendpixel(x*CELL+i, y*CELL+j, 32, 32, 32, 120);
			}
		}
	}
}

void Renderer::draw_grav()
{
	int x, y, i, ca;
	float nx, ny, dist;

	if(!gravityFieldEnabled)
		return;

	for (y=0; y<YCELLS; y++)
	{
		for (x=0; x<XCELLS; x++)
		{
			ca = y*XCELLS+x;
			if(fabsf(sim->gravx[ca]) <= 0.001f && fabsf(sim->gravy[ca]) <= 0.001f)
				continue;
			nx = float(x*CELL);
			ny = float(y*CELL);
			dist = fabsf(sim->gravy[ca])+fabsf(sim->gravx[ca]);
			for(i = 0; i < 4; i++)
			{
				nx -= sim->gravx[ca]*0.5f;
				ny -= sim->gravy[ca]*0.5f;
				addpixel((int)(nx+0.5f), (int)(ny+0.5f), 255, 255, 255, (int)(dist*20.0f));
			}
		}
	}
}

void Renderer::draw_air()
{
	if(!sim->aheat_enable && (display_mode & DISPLAY_AIRH))
		return;
	if(!(display_mode & DISPLAY_AIR))
		return;
	int x, y, i, j;
	float (*pv)[XCELLS] = sim->air->pv;
	float (*hv)[XCELLS] = sim->air->hv;
	float (*vx)[XCELLS] = sim->air->vx;
	float (*vy)[XCELLS] = sim->air->vy;
	pixel c = 0;
	for (y=0; y<YCELLS; y++)
		for (x=0; x<XCELLS; x++)
		{
			if (display_mode & DISPLAY_AIRP)
			{
				if (pv[y][x] > 0.0f)
					c  = PIXRGB(clamp_flt(pv[y][x], 0.0f, 8.0f), 0, 0);//positive pressure is red!
				else
					c  = PIXRGB(0, 0, clamp_flt(-pv[y][x], 0.0f, 8.0f));//negative pressure is blue!
			}
			else if (display_mode & DISPLAY_AIRV)
			{
				c  = PIXRGB(clamp_flt(fabsf(vx[y][x]), 0.0f, 8.0f),//vx adds red
					clamp_flt(pv[y][x], 0.0f, 8.0f),//pressure adds green
					clamp_flt(fabsf(vy[y][x]), 0.0f, 8.0f));//vy adds blue
			}
			else if (display_mode & DISPLAY_AIRH)
			{
				c = HeatToColour(hv[y][x]);
				//c  = PIXRGB(clamp_flt(fabsf(vx[y][x]), 0.0f, 8.0f),//vx adds red
				//	clamp_flt(hv[y][x], 0.0f, 1600.0f),//heat adds green
				//	clamp_flt(fabsf(vy[y][x]), 0.0f, 8.0f));//vy adds blue
			}
			else if (display_mode & DISPLAY_AIRC)
			{
				int r;
				int g;
				int b;
				// velocity adds grey
				r = clamp_flt(fabsf(vx[y][x]), 0.0f, 24.0f) + clamp_flt(fabsf(vy[y][x]), 0.0f, 20.0f);
				g = clamp_flt(fabsf(vx[y][x]), 0.0f, 20.0f) + clamp_flt(fabsf(vy[y][x]), 0.0f, 24.0f);
				b = clamp_flt(fabsf(vx[y][x]), 0.0f, 24.0f) + clamp_flt(fabsf(vy[y][x]), 0.0f, 20.0f);
				if (pv[y][x] > 0.0f)
				{
					r += clamp_flt(pv[y][x], 0.0f, 16.0f);//pressure adds red!
					if (r>255)
						r=255;
					if (g>255)
						g=255;
					if (b>255)
						b=255;
					c  = PIXRGB(r, g, b);
				}
				else
				{
					b += clamp_flt(-pv[y][x], 0.0f, 16.0f);//pressure adds blue!
					if (r>255)
						r=255;
					if (g>255)
						g=255;
					if (b>255)
						b=255;
					c  = PIXRGB(r, g, b);
				}
			}
			if (findingElement)
				c = PIXRGB(PIXR(c)/10,PIXG(c)/10,PIXB(c)/10);
			for (j=0; j<CELL; j++)//draws the colors
				for (i=0; i<CELL; i++)
					vid[(x*CELL+i) + (y*CELL+j)*(VIDXRES)] = c;
		}
}

void Renderer::DrawWalls()
{
	for (int y = 0; y < YCELLS; y++)
		for (int x =0; x < XCELLS; x++)
			if (sim->bmap[y][x])
			{
				unsigned char wt = sim->bmap[y][x];
				if (wt >= UI_WALLCOUNT)
					continue;
				unsigned char powered = sim->emap[y][x];
				pixel pc = PIXPACK(sim->wtypes[wt].colour);
				pixel gc = PIXPACK(sim->wtypes[wt].eglow);

				if (findingElement)
				{
					pc = PIXRGB(PIXR(pc)/10,PIXG(pc)/10,PIXB(pc)/10);
					gc = PIXRGB(PIXR(gc)/10,PIXG(gc)/10,PIXB(gc)/10);
				}

				switch (sim->wtypes[wt].drawstyle)
				{
				case 0:
					if (wt == WL_EWALL || wt == WL_STASIS)
					{
						bool reverse = wt == WL_STASIS;
						if ((powered > 0) ^ reverse)
						{
							for (int j = 0; j < CELL; j++)
								for (int i =0; i < CELL; i++)
									if (i&j&1)
										vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = pc;
						}
						else
						{
							for (int j = 0; j < CELL; j++)
								for (int i = 0; i < CELL; i++)
									if (!(i&j&1))
										vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = pc;
						}
					}
					else if (wt == WL_WALLELEC)
					{
						for (int j = 0; j < CELL; j++)
							for (int i = 0; i < CELL; i++)
							{
								if (!((y*CELL+j)%2) && !((x*CELL+i)%2))
									vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = pc;
								else
									vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = PIXPACK(0x808080);
							}
					}
					else if (wt == WL_EHOLE)
					{
						if (powered)
						{
							for (int j = 0; j < CELL; j++)
								for (int i = 0; i < CELL; i++)
									vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = PIXPACK(0x242424);
							for (int j = 0; j < CELL; j += 2)
								for (int i = 0; i < CELL; i += 2)
									vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = PIXPACK(0x000000);
						}
						else
						{
							for (int j = 0; j < CELL; j += 2)
								for (int i =0; i < CELL; i += 2)
									vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = PIXPACK(0x242424);
						}
					}
					else if (wt == WL_STREAM)
					{
						float xf = x*CELL + CELL*0.5f;
						float yf = y*CELL + CELL*0.5f;
						int oldX = (int)(xf+0.5f), oldY = (int)(yf+0.5f);
						int newX, newY;
						float xVel = sim->vx[y][x]*0.125f, yVel = sim->vy[y][x]*0.125f;
						// there is no velocity here, draw a streamline and continue
						if (!xVel && !yVel)
						{
							drawtext(x*CELL, y*CELL-2, 0xE00D, 255, 255, 255, 128);
							addpixel(oldX, oldY, 255, 255, 255, 255);
							continue;
						}
						bool changed = false;
						for (int t = 0; t < 1024; t++)
						{
							newX = (int)(xf+0.5f);
							newY = (int)(yf+0.5f);
							if (newX != oldX || newY != oldY)
							{
								changed = true;
								oldX = newX;
								oldY = newY;
							}
							if (changed && (newX<0 || newX>=XRES || newY<0 || newY>=YRES))
								break;
							addpixel(newX, newY, 255, 255, 255, 64);
							// cache velocity and other checks so we aren't running them constantly
							if (changed)
							{
								int wallX = newX/CELL;
								int wallY = newY/CELL;
								xVel = sim->vx[wallY][wallX]*0.125f;
								yVel = sim->vy[wallY][wallX]*0.125f;
								if (wallX != x && wallY != y && sim->bmap[wallY][wallX] == WL_STREAM)
									break;
							}
							xf += xVel;
							yf += yVel;
						}
						drawtext(x*CELL, y*CELL-2, 0xE00D, 255, 255, 255, 128);
					}
					break;
				case 1:
					for (int j = 0; j < CELL; j += 2)
						for (int i = (j>>1)&1; i < CELL; i += 2)
							vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = pc;
					break;
				case 2:
					for (int j = 0; j < CELL; j += 2)
						for (int i = 0; i < CELL; i += 2)
							vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = pc;
					break;
				case 3:
					for (int j = 0; j < CELL; j++)
						for (int i = 0; i < CELL; i++)
							vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = pc;
					break;
				case 4:
					for (int j = 0; j < CELL; j++)
						for (int i = 0; i < CELL; i++)
							if (i == j)
								vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = pc;
							else if (i == j+1 || (i == 0 && j == CELL-1))
								vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = gc;
							else
								vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = PIXPACK(0x202020);
					break;
				}

				// when in blob view, draw some blobs...
				if (render_mode & PMODE_BLOB)
				{
					switch (sim->wtypes[wt].drawstyle)
					{
					case 0:
						if (wt == WL_EWALL || wt == WL_STASIS)
						{
							bool reverse = wt == WL_STASIS;
							if ((powered>0) ^ reverse)
							{
								for (int j = 0; j < CELL; j++)
									for (int i =0; i < CELL; i++)
										if (i&j&1)
											drawblob((x*CELL+i), (y*CELL+j), PIXR(pc), PIXG(pc), PIXB(pc));
							}
							else
							{
								for (int j = 0; j < CELL; j++)
									for (int i = 0; i < CELL; i++)
										if (!(i&j&1))
											drawblob((x*CELL+i), (y*CELL+j), PIXR(pc), PIXG(pc), PIXB(pc));
							}
						}
						else if (wt == WL_WALLELEC)
						{
							for (int j = 0; j < CELL; j++)
								for (int i =0; i < CELL; i++)
								{
									if (!((y*CELL+j)%2) && !((x*CELL+i)%2))
										drawblob((x*CELL+i), (y*CELL+j), PIXR(pc), PIXG(pc), PIXB(pc));
									else
										drawblob((x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
								}
						}
						else if (wt == WL_EHOLE)
						{
							if (powered)
							{
								for (int j = 0; j < CELL; j++)
									for (int i = 0; i < CELL; i++)
										drawblob((x*CELL+i), (y*CELL+j), 0x24, 0x24, 0x24);
								for (int j = 0; j < CELL; j += 2)
									for (int i = 0; i < CELL; i += 2)
										// looks bad if drawing black blobs
										vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = PIXPACK(0x000000);
							}
							else
							{
								for (int j = 0; j < CELL; j += 2)
									for (int i = 0; i < CELL; i += 2)
										drawblob((x*CELL+i), (y*CELL+j), 0x24, 0x24, 0x24);
							}
						}
						break;
					case 1:
						for (int j = 0; j < CELL; j += 2)
							for (int i = (j>>1)&1; i < CELL; i += 2)
								drawblob((x*CELL+i), (y*CELL+j), PIXR(pc), PIXG(pc), PIXB(pc));
						break;
					case 2:
						for (int j = 0; j < CELL; j += 2)
							for (int i = 0; i < CELL; i+=2)
								drawblob((x*CELL+i), (y*CELL+j), PIXR(pc), PIXG(pc), PIXB(pc));
						break;
					case 3:
						for (int j = 0; j < CELL; j++)
							for (int i = 0; i < CELL; i++)
								drawblob((x*CELL+i), (y*CELL+j), PIXR(pc), PIXG(pc), PIXB(pc));
						break;
					case 4:
						for (int j = 0; j < CELL; j++)
							for (int i = 0; i < CELL; i++)
								if (i == j)
									drawblob((x*CELL+i), (y*CELL+j), PIXR(pc), PIXG(pc), PIXB(pc));
								else if (i == j+1 || (i == 0 && j == CELL-1))
									vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = gc;
								else
									// looks bad if drawing black blobs
									vid[(y*CELL+j)*(VIDXRES)+(x*CELL+i)] = PIXPACK(0x202020);
						break;
					}
				}

				if (sim->wtypes[wt].eglow && powered)
				{
					// glow if electrified
					pixel glow = sim->wtypes[wt].eglow;
					int alpha = 255;
					int cr = (alpha*PIXR(glow) + (255-alpha)*fire_r[y/CELL][x/CELL]) >> 8;
					int cg = (alpha*PIXG(glow) + (255-alpha)*fire_g[y/CELL][x/CELL]) >> 8;
					int cb = (alpha*PIXB(glow) + (255-alpha)*fire_b[y/CELL][x/CELL]) >> 8;

					if (cr > 255)
						cr = 255;
					if (cg > 255)
						cg = 255;
					if (cb > 255)
						cb = 255;
					fire_r[y][x] = cr;
					fire_g[y][x] = cg;
					fire_b[y][x] = cb;
				}
			}
}

void Renderer::render_fire()
{
	if(!(render_mode & FIREMODE))
		return;
	int i,j,x,y,r,g,b,a;
	for (j=0; j<YCELLS; j++)
		for (i=0; i<XCELLS; i++)
		{
			r = fire_r[j][i];
			g = fire_g[j][i];
			b = fire_b[j][i];
			if (r || g || b)
				for (y=-CELL; y<2*CELL; y++)
					for (x=-CELL; x<2*CELL; x++)
					{
						a = fire_alpha[y+CELL][x+CELL];
						if (findingElement)
							a /= 2;
						addpixel(i*CELL+x, j*CELL+y, r, g, b, a);
					}
			r *= 8;
			g *= 8;
			b *= 8;
			for (y=-1; y<2; y++)
				for (x=-1; x<2; x++)
					if ((x || y) && i+x>=0 && j+y>=0 && i+x<XCELLS && j+y<YCELLS)
					{
						r += fire_r[j+y][i+x];
						g += fire_g[j+y][i+x];
						b += fire_b[j+y][i+x];
					}
			r /= 16;
			g /= 16;
			b /= 16;
			fire_r[j][i] = r>4 ? r-4 : 0;
			fire_g[j][i] = g>4 ? g-4 : 0;
			fire_b[j][i] = b>4 ? b-4 : 0;
		}
}

int HeatToColour(float temp)
{
	constexpr float min_temp = MIN_TEMP;
	constexpr float max_temp = MAX_TEMP;
	auto color = Renderer::heatTableAt(int((temp - min_temp) / (max_temp - min_temp) * 1024));
	return PIXRGB((int)(PIXR(color)*0.7f), (int)(PIXG(color)*0.7f), (int)(PIXB(color)*0.7f));
}
