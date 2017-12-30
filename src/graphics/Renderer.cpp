#include <cmath>
#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include "Config.h"
#include "Misc.h"
#include "Renderer.h"
#include "Graphics.h"
#include "common/tpt-minmax.h"
#include "gui/game/RenderPreset.h"
#include "simulation/Elements.h"
#include "simulation/ElementGraphics.h"
#include "simulation/Air.h"
#ifdef LUACONSOLE
#include "lua/LuaScriptInterface.h"
#include "lua/LuaScriptHelper.h"
#endif
extern "C"
{
#include "hmap.h"
#ifdef OGLR
#include "Shaders.h"
#endif
}

#ifndef OGLI
#define VIDXRES WINDOWW
#define VIDYRES WINDOWH
#else
#define VIDXRES XRES
#define VIDYRES YRES
#endif


void Renderer::RenderBegin()
{
#ifdef OGLI
#ifdef OGLR
	draw_air();
	draw_grav();
	DrawWalls();
	render_parts();
	render_fire();
	draw_other();
	draw_grav_zones();
	DrawSigns();

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
	glTranslated(0, MENUSIZE, 0);
#else
	if(display_mode & DISPLAY_PERS)
	{
		std::copy(persistentVid, persistentVid+(VIDXRES*YRES), vid);
	}
	pixel * oldVid;
	if(display_mode & DISPLAY_WARP)
	{
		oldVid = vid;
		vid = warpVid;
		std::fill(warpVid, warpVid+(VIDXRES*VIDYRES), 0);
	}

	draw_air();
	draw_grav();
	DrawWalls();
	render_parts();
	if(display_mode & DISPLAY_PERS)
	{
		int i,r,g,b;
		for (i = 0; i < VIDXRES*YRES; i++)
		{
			r = PIXR(vid[i]);
			g = PIXG(vid[i]);
			b = PIXB(vid[i]);
			if (r>0)
				r--;
			if (g>0)
				g--;
			if (b>0)
				b--;
			persistentVid[i] = PIXRGB(r,g,b);
		}
	}

	render_fire();
	draw_other();
	draw_grav_zones();
	DrawSigns();
	if(display_mode & DISPLAY_WARP)
	{
		vid = oldVid;
	}
#endif
#else
	if(display_mode & DISPLAY_PERS)
	{
		std::copy(persistentVid, persistentVid+(VIDXRES*YRES), vid);
	}
	pixel * oldVid = NULL;
	if(display_mode & DISPLAY_WARP)
	{
		oldVid = vid;
		vid = warpVid;
		std::fill(warpVid, warpVid+(VIDXRES*VIDYRES), 0);
	}

	draw_air();
	draw_grav();
	DrawWalls();
	render_parts();
	if(display_mode & DISPLAY_PERS)
	{
		int i,r,g,b;
		for (i = 0; i < VIDXRES*YRES; i++)
		{
			r = PIXR(vid[i]);
			g = PIXG(vid[i]);
			b = PIXB(vid[i]);
			if (r>0)
				r--;
			if (g>0)
				g--;
			if (b>0)
				b--;
			persistentVid[i] = PIXRGB(r,g,b);
		}
	}

	render_fire();
	draw_other();
	draw_grav_zones();
	DrawSigns();

	if(display_mode & DISPLAY_WARP)
	{
		vid = oldVid;
	}

	FinaliseParts();
#endif
}

void Renderer::RenderEnd()
{
#ifdef OGLI
#ifdef OGLR
	glTranslated(0, -MENUSIZE, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevFbo);
	FinaliseParts();
	RenderZoom();
#else
	RenderZoom();
	FinaliseParts();
#endif
#else
	RenderZoom();
#endif
}

void Renderer::SetSample(int x, int y)
{
	sampleColor = GetPixel(x, y);
}

void Renderer::clearScreen(float alpha)
{
#ifdef OGLR
	GLint prevFbo;
	if(alpha > 0.999f)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevFbo);
	}
	else
	{
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
		glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(XRES, 0);
		glVertex2f(XRES, YRES);
		glVertex2f(0, YRES);
		glEnd();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevFbo);
		glBlendEquation(GL_FUNC_ADD);
	}
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
#ifdef OGLI
#ifndef OGLR
	std::fill(vid, vid+(VIDXRES*VIDYRES), 0);
#endif
#else
	g->Clear();
#endif
}
#ifdef OGLR
void Renderer::checkShader(GLuint shader, const char * shname)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		char errorBuf[ GL_INFO_LOG_LENGTH];
		int errLen;
		glGetShaderInfoLog(shader, GL_INFO_LOG_LENGTH, &errLen, errorBuf);
		fprintf(stderr, "Failed to compile %s shader:\n%s\n", shname, errorBuf);
		exit(1);
	}
}
void Renderer::checkProgram(GLuint program, const char * progname)
{
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		char errorBuf[ GL_INFO_LOG_LENGTH];
		int errLen;
		glGetShaderInfoLog(program, GL_INFO_LOG_LENGTH, &errLen, errorBuf);
		fprintf(stderr, "Failed to link %s program:\n%s\n", progname, errorBuf);
		exit(1);
	}
}
void Renderer::loadShaders()
{
	GLuint vertexShader, fragmentShader;

	//Particle texture
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource( vertexShader, 1, &fireVertex, NULL);
	glShaderSource( fragmentShader, 1, &fireFragment, NULL);

	glCompileShader( vertexShader );
	checkShader(vertexShader, "FV");
	glCompileShader( fragmentShader );
	checkShader(fragmentShader, "FF");

	fireProg = glCreateProgram();
	glAttachShader( fireProg, vertexShader );
	glAttachShader( fireProg, fragmentShader );
	glLinkProgram( fireProg );
	checkProgram(fireProg, "F");

	//Lensing
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource( vertexShader, 1, &lensVertex, NULL);
	glShaderSource( fragmentShader, 1, &lensFragment, NULL);

	glCompileShader( vertexShader );
	checkShader(vertexShader, "LV");
	glCompileShader( fragmentShader );
	checkShader(fragmentShader, "LF");

	lensProg = glCreateProgram();
	glAttachShader( lensProg, vertexShader );
	glAttachShader( lensProg, fragmentShader );
	glLinkProgram( lensProg );
	checkProgram(lensProg, "L");

	//Air Velocity
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource( vertexShader, 1, &airVVertex, NULL);
	glShaderSource( fragmentShader, 1, &airVFragment, NULL);

	glCompileShader( vertexShader );
	checkShader(vertexShader, "AVX");
	glCompileShader( fragmentShader );
	checkShader(fragmentShader, "AVF");

	airProg_Velocity = glCreateProgram();
	glAttachShader( airProg_Velocity, vertexShader );
	glAttachShader( airProg_Velocity, fragmentShader );
	glLinkProgram( airProg_Velocity );
	checkProgram(airProg_Velocity, "AV");

	//Air Pressure
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource( vertexShader, 1, &airPVertex, NULL);
	glShaderSource( fragmentShader, 1, &airPFragment, NULL);

	glCompileShader( vertexShader );
	checkShader(vertexShader, "APV");
	glCompileShader( fragmentShader );
	checkShader(fragmentShader, "APF");

	airProg_Pressure = glCreateProgram();
	glAttachShader( airProg_Pressure, vertexShader );
	glAttachShader( airProg_Pressure, fragmentShader );
	glLinkProgram( airProg_Pressure );
	checkProgram(airProg_Pressure, "AP");

	//Air cracker
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource( vertexShader, 1, &airCVertex, NULL);
	glShaderSource( fragmentShader, 1, &airCFragment, NULL);

	glCompileShader( vertexShader );
	checkShader(vertexShader, "ACV");
	glCompileShader( fragmentShader );
	checkShader(fragmentShader, "ACF");

	airProg_Cracker = glCreateProgram();
	glAttachShader( airProg_Cracker, vertexShader );
	glAttachShader( airProg_Cracker, fragmentShader );
	glLinkProgram( airProg_Cracker );
	checkProgram(airProg_Cracker, "AC");
}
#endif

void Renderer::FinaliseParts()
{
#ifdef OGLR
	glEnable( GL_TEXTURE_2D );
	if(display_mode & DISPLAY_WARP)
	{
		float xres = XRES, yres = YRES;
		glUseProgram(lensProg);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, partsFboTex);
		glUniform1i(glGetUniformLocation(lensProg, "pTex"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, partsTFX);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, XRES/CELL, YRES/CELL, GL_RED, GL_FLOAT, sim->gravx);
		glUniform1i(glGetUniformLocation(lensProg, "tfX"), 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, partsTFY);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, XRES/CELL, YRES/CELL, GL_GREEN, GL_FLOAT, sim->gravy);
		glUniform1i(glGetUniformLocation(lensProg, "tfY"), 2);
		glActiveTexture(GL_TEXTURE0);
		glUniform1fv(glGetUniformLocation(lensProg, "xres"), 1, &xres);
		glUniform1fv(glGetUniformLocation(lensProg, "yres"), 1, &yres);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, partsFboTex);
		glBlendFunc(GL_ONE, GL_ONE);
	}

	int sdl_scale = 1;
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2d(1, 0);
	//glVertex3f(XRES*sdl_scale, WINDOWH*sdl_scale, 1.0);
	glVertex3f(XRES*sdl_scale, YRES*sdl_scale, 1.0);
	glTexCoord2d(0, 0);
	//glVertex3f(0, WINDOWH*sdl_scale, 1.0);
	glVertex3f(0, YRES*sdl_scale, 1.0);
	glTexCoord2d(0, 1);
	//glVertex3f(0, MENUSIZE*sdl_scale, 1.0);
	glVertex3f(0, 0, 1.0);
	glTexCoord2d(1, 1);
	//glVertex3f(XRES*sdl_scale, MENUSIZE*sdl_scale, 1.0);
	glVertex3f(XRES*sdl_scale, 0, 1.0);
	glEnd();

	if(display_mode & DISPLAY_WARP)
	{
		glUseProgram(0);

	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable( GL_TEXTURE_2D );
#endif

#if defined(OGLI) && !defined(OGLR)
	if(display_mode & DISPLAY_WARP)
	{
		render_gravlensing(warpVid);
	}
	g->draw_image(vid, 0, 0, VIDXRES, VIDYRES, 255);
#endif

#if !defined(OGLR) && !defined(OGLI)
	if(display_mode & DISPLAY_WARP)
	{
		render_gravlensing(warpVid);
	}
#endif
}

void Renderer::RenderZoom()
{
	if(!zoomEnabled)
		return;
	#if defined(OGLR)
		int sdl_scale = 1;
		int origBlendSrc, origBlendDst;
		float zcx1, zcx0, zcy1, zcy0, yfactor, xfactor, i; //X-Factor is shit, btw
		xfactor = 1.0f/(float)XRES;
		yfactor = 1.0f/(float)YRES;
		yfactor*=-1.0f;

		zcx1 = (zoomScopePosition.X)*xfactor;
		zcx0 = (zoomScopePosition.X+zoomScopeSize)*xfactor;
		zcy1 = (zoomScopePosition.Y-1)*yfactor;
		zcy0 = ((zoomScopePosition.Y-1+zoomScopeSize))*yfactor;

		glGetIntegerv(GL_BLEND_SRC, &origBlendSrc);
		glGetIntegerv(GL_BLEND_DST, &origBlendDst);
		glBlendFunc(GL_ONE, GL_ZERO);

		glEnable( GL_TEXTURE_2D );
		//glReadBuffer(GL_AUX0);
		glBindTexture(GL_TEXTURE_2D, partsFboTex);

		//Draw zoomed texture
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glTexCoord2d(zcx1, zcy1);
		glVertex2i(zoomWindowPosition.X, zoomWindowPosition.Y);
		glTexCoord2d(zcx0, zcy1);
		glVertex2i(zoomWindowPosition.X+(zoomScopeSize*ZFACTOR), zoomWindowPosition.Y);
		glTexCoord2d(zcx0, zcy0);
		glVertex2i(zoomWindowPosition.X+(zoomScopeSize*ZFACTOR), zoomWindowPosition.Y+(zoomScopeSize*ZFACTOR));
		glTexCoord2d(zcx1, zcy0);
		glVertex2i(zoomWindowPosition.X, zoomWindowPosition.Y+(zoomScopeSize*ZFACTOR));
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable( GL_TEXTURE_2D );

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//Lines to make the pixels stand out
		glLineWidth(sdl_scale);
		//glEnable(GL_LINE_SMOOTH);
		glBegin(GL_LINES);
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		for(i = 0; i < zoomScopeSize; i++)
		{
			//Across
			glVertex2i(zoomWindowPosition.X, zoomWindowPosition.Y+(i*ZFACTOR));
			glVertex2i(zoomWindowPosition.X+(zoomScopeSize*ZFACTOR), zoomWindowPosition.Y+(i*ZFACTOR));

			//Down
			glVertex2i(zoomWindowPosition.X+(i*ZFACTOR), zoomWindowPosition.Y);
			glVertex2i(zoomWindowPosition.X+(i*ZFACTOR), zoomWindowPosition.Y+(zoomScopeSize*ZFACTOR));
		}
		glEnd();

		//Draw zoom window border
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBegin(GL_LINE_LOOP);
		glVertex2i(zoomWindowPosition.X, zoomWindowPosition.Y);
		glVertex2i(zoomWindowPosition.X+(zoomScopeSize*ZFACTOR), zoomWindowPosition.Y);
		glVertex2i(zoomWindowPosition.X+(zoomScopeSize*ZFACTOR), zoomWindowPosition.Y+(zoomScopeSize*ZFACTOR));
		glVertex2i(zoomWindowPosition.X, zoomWindowPosition.Y+(zoomScopeSize*ZFACTOR));
		glEnd();
		//glDisable(GL_LINE_SMOOTH);

		if(zoomEnabled)
		{
			glEnable(GL_COLOR_LOGIC_OP);
			//glEnable(GL_LINE_SMOOTH);
			glLogicOp(GL_XOR);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glBegin(GL_LINE_LOOP);
			glVertex2i(zoomScopePosition.X, zoomScopePosition.Y);
			glVertex2i(zoomScopePosition.X+zoomScopeSize, zoomScopePosition.Y);
			glVertex2i(zoomScopePosition.X+zoomScopeSize, zoomScopePosition.Y+zoomScopeSize);
			glVertex2i(zoomScopePosition.X, zoomScopePosition.Y+zoomScopeSize);
			/*glVertex3i((zoomScopePosition.X-1)*sdl_scale, (WINDOWH-(zoomScopePosition.Y-1))*sdl_scale, 0);
			glVertex3i((zoomScopePosition.X-1)*sdl_scale, (WINDOWH-(zoomScopePosition.Y+zoomScopeSize))*sdl_scale, 0);
			glVertex3i((zoomScopePosition.X+zoomScopeSize)*sdl_scale, (WINDOWH-(zoomScopePosition.Y+zoomScopeSize))*sdl_scale, 0);
			glVertex3i((zoomScopePosition.X+zoomScopeSize)*sdl_scale, (WINDOWH-(zoomScopePosition.Y-1))*sdl_scale, 0);
			glVertex3i((zoomScopePosition.X-1)*sdl_scale, (WINDOWH-(zoomScopePosition.Y-1))*sdl_scale, 0);*/
			glEnd();
			glDisable(GL_COLOR_LOGIC_OP);
		}
		glLineWidth(1);
		glBlendFunc(origBlendSrc, origBlendDst);
	#else
		int x, y, i, j;
		pixel pix;
		pixel * img = vid;
		clearrect(zoomWindowPosition.X-1, zoomWindowPosition.Y-1, zoomScopeSize*ZFACTOR+1, zoomScopeSize*ZFACTOR+1);
		drawrect(zoomWindowPosition.X-2, zoomWindowPosition.Y-2, zoomScopeSize*ZFACTOR+3, zoomScopeSize*ZFACTOR+3, 192, 192, 192, 255);
		drawrect(zoomWindowPosition.X-1, zoomWindowPosition.Y-1, zoomScopeSize*ZFACTOR+1, zoomScopeSize*ZFACTOR+1, 0, 0, 0, 255);
		for (j=0; j<zoomScopeSize; j++)
			for (i=0; i<zoomScopeSize; i++)
			{
				pix = img[(j+zoomScopePosition.Y)*(VIDXRES)+(i+zoomScopePosition.X)];
				for (y=0; y<ZFACTOR-1; y++)
					for (x=0; x<ZFACTOR-1; x++)
						img[(j*ZFACTOR+y+zoomWindowPosition.Y)*(VIDXRES)+(i*ZFACTOR+x+zoomWindowPosition.X)] = pix;
			}
		if (zoomEnabled)
		{
			for (j=-1; j<=zoomScopeSize; j++)
			{
				xor_pixel(zoomScopePosition.X+j, zoomScopePosition.Y-1);
				xor_pixel(zoomScopePosition.X+j, zoomScopePosition.Y+zoomScopeSize);
			}
			for (j=0; j<zoomScopeSize; j++)
			{
				xor_pixel(zoomScopePosition.X-1, zoomScopePosition.Y+j);
				xor_pixel(zoomScopePosition.X+zoomScopeSize, zoomScopePosition.Y+j);
			}
		}
	#endif
}

int Renderer_wtypesCount;
wall_type * Renderer_wtypes = LoadWalls(Renderer_wtypesCount);


VideoBuffer * Renderer::WallIcon(int wallID, int width, int height)
{
	int i, j;
	int wt = wallID;
	if (wt<0 || wt>=Renderer_wtypesCount)
		return 0;
	wall_type *wtypes = Renderer_wtypes;
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
	else if (wt==WL_EHOLE)
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
		newTexture->SetCharacter(4, 2, 0x8D, 255, 255, 255, 255);
		for (i=width/3; i<width; i++)
		{
			newTexture->SetPixel(i, 7+(int)(3.9f*cos(i*0.3f)), 255, 255, 255, 255);
		}
	}
	return newTexture;
}

void Renderer::DrawBlob(int x, int y, unsigned char cr, unsigned char cg, unsigned char cb)
{
	blendpixel(x+1, y, cr, cg, cb, 112);
	blendpixel(x-1, y, cr, cg, cb, 112);
	blendpixel(x, y+1, cr, cg, cb, 112);
	blendpixel(x, y-1, cr, cg, cb, 112);

	blendpixel(x+1, y-1, cr, cg, cb, 64);
	blendpixel(x-1, y-1, cr, cg, cb, 64);
	blendpixel(x+1, y+1, cr, cg, cb, 64);
	blendpixel(x-1, y+1, cr, cg, cb, 64);
}

void Renderer::DrawWalls()
{
#ifdef OGLR
	// terrible OpenGL "support"
	GLint prevFbo;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
	glTranslated(0, MENUSIZE, 0);

	for (int y = 0; y < YRES/CELL; y++)
		for (int x = 0; x < XRES/CELL; x++)
			if (sim->bmap[y][x])
			{
				unsigned char wt = sim->bmap[y][x];
				if (wt >= UI_WALLCOUNT)
					continue;
				pixel pc = sim->wtypes[wt].colour;
				pixel gc = sim->wtypes[wt].eglow;

				int cr = PIXR(pc);
				int cg = PIXG(pc);
				int cb = PIXB(pc);

				fillrect(x*CELL, y*CELL, CELL, CELL, cr, cg, cb, 255);
			}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevFbo);
	glTranslated(0, -MENUSIZE, 0);
#else
	for (int y = 0; y < YRES/CELL; y++)
		for (int x =0; x < XRES/CELL; x++)
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
					if (wt == WL_EWALL)
					{
						if (powered)
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
							drawtext(x*CELL, y*CELL-2, "\x8D", 255, 255, 255, 128);
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
						drawtext(x*CELL, y*CELL-2, "\x8D", 255, 255, 255, 128);
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
						if (wt == WL_EWALL)
						{
							if (powered)
							{
								for (int j = 0; j < CELL; j++)
									for (int i =0; i < CELL; i++)
										if (i&j&1)
											drawblob((x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
							}
							else
							{
								for (int j = 0; j < CELL; j++)
									for (int i = 0; i < CELL; i++)
										if (!(i&j&1))
											drawblob((x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
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
#endif
}

void Renderer::DrawSigns()
{
	int x, y, w, h;
	std::vector<sign> signs = sim->signs;
#ifdef OGLR
	GLint prevFbo;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
	glTranslated(0, MENUSIZE, 0);
#endif
	for (size_t i = 0; i < signs.size(); i++)
		if (signs[i].text.length())
		{
			char type = 0;
			std::string text = signs[i].getText(sim);
			sign::splitsign(signs[i].text.c_str(), &type);
			signs[i].pos(text, x, y, w, h);
			clearrect(x, y, w+1, h);
			drawrect(x, y, w+1, h, 192, 192, 192, 255);
			if (!type)
				drawtext(x+3, y+3, text, 255, 255, 255, 255);
			else if(type == 'b')
				drawtext(x+3, y+3, text, 211, 211, 40, 255);
			else
				drawtext(x+3, y+3, text, 0, 191, 255, 255);
				
			if (signs[i].ju != sign::None)
			{
				int x = signs[i].x;
				int y = signs[i].y;
				int dx = 1 - signs[i].ju;
				int dy = (signs[i].y > 18) ? -1 : 1;
#ifdef OGLR
				glBegin(GL_LINES);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glVertex2i(x, y);
				glVertex2i(x+(dx*4), y+(dy*4));
				glEnd();
#else
				for (int j = 0; j < 4; j++)
				{
					blendpixel(x, y, 192, 192, 192, 255);
					x += dx;
					y += dy;
				}
#endif
			}
		}
#ifdef OGLR
	glTranslated(0, -MENUSIZE, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevFbo);
#endif
}

void Renderer::render_gravlensing(pixel * source)
{
#ifndef OGLR
	int nx, ny, rx, ry, gx, gy, bx, by, co;
	int r, g, b;
	pixel t;
	pixel *src = source;
	pixel *dst = vid;
	if (!dst)
		return;
	for(nx = 0; nx < XRES; nx++)
	{
		for(ny = 0; ny < YRES; ny++)
		{
			co = (ny/CELL)*(XRES/CELL)+(nx/CELL);
			rx = (int)(nx-sim->gravx[co]*0.75f+0.5f);
			ry = (int)(ny-sim->gravy[co]*0.75f+0.5f);
			gx = (int)(nx-sim->gravx[co]*0.875f+0.5f);
			gy = (int)(ny-sim->gravy[co]*0.875f+0.5f);
			bx = (int)(nx-sim->gravx[co]+0.5f);
			by = (int)(ny-sim->gravy[co]+0.5f);
			if(rx >= 0 && rx < XRES && ry >= 0 && ry < YRES && gx >= 0 && gx < XRES && gy >= 0 && gy < YRES && bx >= 0 && bx < XRES && by >= 0 && by < YRES)
			{
				t = dst[ny*(VIDXRES)+nx];
				r = PIXR(src[ry*(VIDXRES)+rx]) + PIXR(t);
				g = PIXG(src[gy*(VIDXRES)+gx]) + PIXG(t);
				b = PIXB(src[by*(VIDXRES)+bx]) + PIXB(t);
				if (r>255)
					r = 255;
				if (g>255)
					g = 255;
				if (b>255)
					b = 255;
				dst[ny*(VIDXRES)+nx] = PIXRGB(r,g,b);
			}
		}
	}
#endif
}

void Renderer::render_fire()
{
#ifndef OGLR
	if(!(render_mode & FIREMODE))
		return;
	int i,j,x,y,r,g,b,a;
	for (j=0; j<YRES/CELL; j++)
		for (i=0; i<XRES/CELL; i++)
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
					if ((x || y) && i+x>=0 && j+y>=0 && i+x<XRES/CELL && j+y<YRES/CELL)
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
#endif
}

float temp[CELL*3][CELL*3];
float fire_alphaf[CELL*3][CELL*3];
float glow_alphaf[11][11];
float blur_alphaf[7][7];
void Renderer::prepare_alpha(int size, float intensity)
{
	//TODO: implement size
	int x,y,i,j;
	float multiplier = 255.0f*intensity;

	memset(temp, 0, sizeof(temp));
	for (x=0; x<CELL; x++)
		for (y=0; y<CELL; y++)
			for (i=-CELL; i<CELL; i++)
				for (j=-CELL; j<CELL; j++)
					temp[y+CELL+j][x+CELL+i] += expf(-0.1f*(i*i+j*j));
	for (x=0; x<CELL*3; x++)
		for (y=0; y<CELL*3; y++)
			fire_alpha[y][x] = (int)(multiplier*temp[y][x]/(CELL*CELL));

#ifdef OGLR
	memset(fire_alphaf, 0, sizeof(fire_alphaf));
	for (x=0; x<CELL*3; x++)
		for (y=0; y<CELL*3; y++)
		{
			fire_alphaf[y][x] = intensity*temp[y][x]/((float)(CELL*CELL));
		}
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, fireAlpha);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, CELL*3, CELL*3, GL_ALPHA, GL_FLOAT, fire_alphaf);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	memset(glow_alphaf, 0, sizeof(glow_alphaf));

	int c = 5;

	glow_alphaf[c][c-1] = 0.4f;
	glow_alphaf[c][c+1] = 0.4f;
	glow_alphaf[c-1][c] = 0.4f;
	glow_alphaf[c+1][c] = 0.4f;
	for (x = 1; x < 6; x++) {
		glow_alphaf[c][c-x] += 0.02f;
		glow_alphaf[c][c+x] += 0.02f;
		glow_alphaf[c-x][c] += 0.02f;
		glow_alphaf[c+x][c] += 0.02f;
		for (y = 1; y < 6; y++) {
			if(x + y > 7)
				continue;
			glow_alphaf[c+x][c-y] += 0.02f;
			glow_alphaf[c-x][c+y] += 0.02f;
			glow_alphaf[c+x][c+y] += 0.02f;
			glow_alphaf[c-x][c-y] += 0.02f;
		}
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, glowAlpha);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 11, 11, GL_ALPHA, GL_FLOAT, glow_alphaf);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	c = 3;

	for (x=-3; x<4; x++)
	{
		for (y=-3; y<4; y++)
		{
			if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
				blur_alphaf[c+x][c-y] = 0.11f;
			if (abs(x)+abs(y) <=3 && abs(x)+abs(y))
				blur_alphaf[c+x][c-y] = 0.08f;
			if (abs(x)+abs(y) == 2)
				blur_alphaf[c+x][c-y] = 0.04f;
		}
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, blurAlpha);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 7, 7, GL_ALPHA, GL_FLOAT, blur_alphaf);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
#endif
}

void Renderer::render_parts()
{
	int deca, decr, decg, decb, cola, colr, colg, colb, firea, firer, fireg, fireb, pixel_mode, q, i, t, nx, ny, x, y, caddress;
	int orbd[4] = {0, 0, 0, 0}, orbl[4] = {0, 0, 0, 0};
	float gradv, flicker;
	Particle * parts;
	Element *elements;
	if(!sim)
		return;
	parts = sim->parts;
	elements = sim->elements;
#ifdef OGLR
	float fnx, fny;
	int cfireV = 0, cfireC = 0, cfire = 0;
	int csmokeV = 0, csmokeC = 0, csmoke = 0;
	int cblobV = 0, cblobC = 0, cblob = 0;
	int cblurV = 0, cblurC = 0, cblur = 0;
	int cglowV = 0, cglowC = 0, cglow = 0;
	int cflatV = 0, cflatC = 0, cflat = 0;
	int caddV = 0, caddC = 0, cadd = 0;
	int clineV = 0, clineC = 0, cline = 0;
	GLint origBlendSrc, origBlendDst, prevFbo;

	glGetIntegerv(GL_BLEND_SRC, &origBlendSrc);
	glGetIntegerv(GL_BLEND_DST, &origBlendDst);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
	//Render to the particle FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
	glTranslated(0, MENUSIZE, 0);
#else
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
#endif
	for(i = 0; i<=sim->parts_lastActiveIndex; i++) {
		if (sim->parts[i].type && sim->parts[i].type >= 0 && sim->parts[i].type < PT_NUM) {
			t = sim->parts[i].type;

			nx = (int)(sim->parts[i].x+0.5f);
			ny = (int)(sim->parts[i].y+0.5f);
#ifdef OGLR
			fnx = sim->parts[i].x;
			fny = sim->parts[i].y;
#endif

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
					if (elements[t].Graphics)
					{
#if !defined(RENDERER) && defined(LUACONSOLE)
						if (lua_gr_func[t])
						{
							if (luacon_graphicsReplacement(this, &(sim->parts[i]), nx, ny, &pixel_mode, &cola, &colr, &colg, &colb, &firea, &firer, &fireg, &fireb, i))
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
						else if ((*(elements[t].Graphics))(this, &(sim->parts[i]), nx, ny, &pixel_mode, &cola, &colr, &colg, &colb, &firea, &firer, &fireg, &fireb)) //That's a lot of args, a struct might be better
#else
						if ((*(elements[t].Graphics))(this, &(sim->parts[i]), nx, ny, &pixel_mode, &cola, &colr, &colg, &colb, &firea, &firer, &fireg, &fireb)) //That's a lot of args, a struct might be better
#endif
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
					else
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
					caddress = (sim->parts[i].temp>elements[t].HighTemperature)?elements[t].HighTemperature-(elements[t].HighTemperature-800.0f):sim->parts[i].temp-(elements[t].HighTemperature-800.0f);
					colr += sin(gradv*caddress) * 226;;
					colg += sin(gradv*caddress*4.55 +3.14) * 34;
					colb += sin(gradv*caddress*2.22 +3.14) * 64;
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
					caddress = restrict_flt((int)( restrict_flt((float)(sim->parts[i].temp+(-MIN_TEMP)), 0.0f, MAX_TEMP+(-MIN_TEMP)) / ((MAX_TEMP+(-MIN_TEMP))/1024) ) *3, 0.0f, (1024.0f*3)-3);
					firea = 255;
					firer = colr = color_data[caddress];
					fireg = colg = color_data[caddress+1];
					fireb = colb = color_data[caddress+2];
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
						q = sqrt((float)sim->parts[i].life);
					else
						q = sim->parts[i].life;
					colr = colg = colb = sin(gradv*q) * 100 + 128;
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

				if (findingElement)
				{
					if (findingElement == parts[i].type)
					{
						colr = firer = 255;
						colg = fireg = colb = fireb = 0;
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

				if (colour_mode & COLOUR_GRAD)
				{
					float frequency = 0.05;
					int q = sim->parts[i].temp-40;
					colr = sin(frequency*q) * 16 + colr;
					colg = sin(frequency*q) * 16 + colg;
					colb = sin(frequency*q) * 16 + colb;
					if(pixel_mode & (FIREMODE | PMODE_GLOW)) pixel_mode = (pixel_mode & ~(FIREMODE|PMODE_GLOW)) | PMODE_BLUR;
				}

	#ifndef OGLR
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
	#endif

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
						char buff[12];  //Buffer for HP
						sprintf(buff, "%3d", sim->parts[i].life);  //Show HP
						drawtext(mousePos.X-8-2*(sim->parts[i].life<100)-2*(sim->parts[i].life<10), mousePos.Y-12, buff, 255, 255, 255, 255);
					}

					if (findingElement == t)
					{
						colr = 255;
						colg = colb = 0;
					}
					else if (colour_mode != COLOUR_HEAT)
					{
						if (cplayer->elem<PT_NUM && cplayer->elem > 0)
						{
							if (cplayer->elem == SPC_AIR)
							{
								colr = PIXR(0x8080FF);
								colg = PIXG(0x8080FF);
								colb = PIXB(0x8080FF);
							}
							else
							{
								colr = PIXR(elements[cplayer->elem].Colour);
								colg = PIXG(elements[cplayer->elem].Colour);
								colb = PIXB(elements[cplayer->elem].Colour);
							}
						}
						else
						{
							colr = 0x80;
							colg = 0x80;
							colb = 0xFF;
						}
					}

#ifdef OGLR
					glColor4f(((float)colr)/255.0f, ((float)colg)/255.0f, ((float)colb)/255.0f, 1.0f);
					glBegin(GL_LINE_STRIP);
					if(t==PT_FIGH)
					{
						glVertex2f(fnx, fny+2);
						glVertex2f(fnx+2, fny);
						glVertex2f(fnx, fny-2);
						glVertex2f(fnx-2, fny);
						glVertex2f(fnx, fny+2);
					}
					else
					{
						glVertex2f(fnx-2, fny-2);
						glVertex2f(fnx+2, fny-2);
						glVertex2f(fnx+2, fny+2);
						glVertex2f(fnx-2, fny+2);
						glVertex2f(fnx-2, fny-2);
					}
					glEnd();
					glBegin(GL_LINES);

					if (colour_mode!=COLOUR_HEAT)
					{
						if (t==PT_STKM2)
							glColor4f(100.0f/255.0f, 100.0f/255.0f, 1.0f, 1.0f);
						else
							glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
					}

					glVertex2f(nx, ny+3);
					glVertex2f(cplayer->legs[0], cplayer->legs[1]);

					glVertex2f(cplayer->legs[0], cplayer->legs[1]);
					glVertex2f(cplayer->legs[4], cplayer->legs[5]);

					glVertex2f(nx, ny+3);
					glVertex2f(cplayer->legs[8], cplayer->legs[9]);

					glVertex2f(cplayer->legs[8], cplayer->legs[9]);
					glVertex2f(cplayer->legs[12], cplayer->legs[13]);
					glEnd();
#else
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
					draw_line(nx, ny+3, cplayer->legs[0], cplayer->legs[1], legr, legg, legb, 255);
					draw_line(cplayer->legs[0], cplayer->legs[1], cplayer->legs[4], cplayer->legs[5], legr, legg, legb, 255);
					draw_line(nx, ny+3, cplayer->legs[8], cplayer->legs[9], legr, legg, legb, 255);
					draw_line(cplayer->legs[8], cplayer->legs[9], cplayer->legs[12], cplayer->legs[13], legr, legg, legb, 255);
					if (cplayer->rocketBoots)
					{
						for (int leg=0; leg<2; leg++)
						{
							int nx = cplayer->legs[leg*8+4], ny = cplayer->legs[leg*8+5];
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
#endif
				}
				if(pixel_mode & PMODE_FLAT)
				{
#ifdef OGLR
					flatV[cflatV++] = nx;
					flatV[cflatV++] = ny;
					flatC[cflatC++] = ((float)colr)/255.0f;
					flatC[cflatC++] = ((float)colg)/255.0f;
					flatC[cflatC++] = ((float)colb)/255.0f;
					flatC[cflatC++] = 1.0f;
					cflat++;
#else
					vid[ny*(VIDXRES)+nx] = PIXRGB(colr,colg,colb);
#endif
				}
				if(pixel_mode & PMODE_BLEND)
				{
#ifdef OGLR
					flatV[cflatV++] = nx;
					flatV[cflatV++] = ny;
					flatC[cflatC++] = ((float)colr)/255.0f;
					flatC[cflatC++] = ((float)colg)/255.0f;
					flatC[cflatC++] = ((float)colb)/255.0f;
					flatC[cflatC++] = ((float)cola)/255.0f;
					cflat++;
#else
					blendpixel(nx, ny, colr, colg, colb, cola);
#endif
				}
				if(pixel_mode & PMODE_ADD)
				{
#ifdef OGLR
					addV[caddV++] = nx;
					addV[caddV++] = ny;
					addC[caddC++] = ((float)colr)/255.0f;
					addC[caddC++] = ((float)colg)/255.0f;
					addC[caddC++] = ((float)colb)/255.0f;
					addC[caddC++] = ((float)cola)/255.0f;
					cadd++;
#else
					addpixel(nx, ny, colr, colg, colb, cola);
#endif
				}
				if(pixel_mode & PMODE_BLOB)
				{
#ifdef OGLR
					blobV[cblobV++] = nx;
					blobV[cblobV++] = ny;
					blobC[cblobC++] = ((float)colr)/255.0f;
					blobC[cblobC++] = ((float)colg)/255.0f;
					blobC[cblobC++] = ((float)colb)/255.0f;
					blobC[cblobC++] = 1.0f;
					cblob++;
#else
					vid[ny*(VIDXRES)+nx] = PIXRGB(colr,colg,colb);

					blendpixel(nx+1, ny, colr, colg, colb, 223);
					blendpixel(nx-1, ny, colr, colg, colb, 223);
					blendpixel(nx, ny+1, colr, colg, colb, 223);
					blendpixel(nx, ny-1, colr, colg, colb, 223);

					blendpixel(nx+1, ny-1, colr, colg, colb, 112);
					blendpixel(nx-1, ny-1, colr, colg, colb, 112);
					blendpixel(nx+1, ny+1, colr, colg, colb, 112);
					blendpixel(nx-1, ny+1, colr, colg, colb, 112);
#endif
				}
				if(pixel_mode & PMODE_GLOW)
				{
					int cola1 = (5*cola)/255;
#ifdef OGLR
					glowV[cglowV++] = nx;
					glowV[cglowV++] = ny;
					glowC[cglowC++] = ((float)colr)/255.0f;
					glowC[cglowC++] = ((float)colg)/255.0f;
					glowC[cglowC++] = ((float)colb)/255.0f;
					glowC[cglowC++] = 1.0f;
					cglow++;
#else
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
#endif
				}
				if(pixel_mode & PMODE_BLUR)
				{
#ifdef OGLR
					blurV[cblurV++] = nx;
					blurV[cblurV++] = ny;
					blurC[cblurC++] = ((float)colr)/255.0f;
					blurC[cblurC++] = ((float)colg)/255.0f;
					blurC[cblurC++] = ((float)colb)/255.0f;
					blurC[cblurC++] = 1.0f;
					cblur++;
#else
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
#endif
				}
				if(pixel_mode & PMODE_SPARK)
				{
					flicker = rand()%20;
#ifdef OGLR
					//Oh god, this is awful
					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 0.0f;
					lineV[clineV++] = fnx-5;
					lineV[clineV++] = fny;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 1.0f - ((float)flicker)/30;
					lineV[clineV++] = fnx;
					lineV[clineV++] = fny;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 0.0f;
					lineV[clineV++] = fnx+5;
					lineV[clineV++] = fny;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 0.0f;
					lineV[clineV++] = fnx;
					lineV[clineV++] = fny-5;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 1.0f - ((float)flicker)/30;
					lineV[clineV++] = fnx;
					lineV[clineV++] = fny;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 0.0f;
					lineV[clineV++] = fnx;
					lineV[clineV++] = fny+5;
					cline++;
#else
					gradv = 4*sim->parts[i].life + flicker;
					for (x = 0; gradv>0.5; x++) {
						addpixel(nx+x, ny, colr, colg, colb, gradv);
						addpixel(nx-x, ny, colr, colg, colb, gradv);

						addpixel(nx, ny+x, colr, colg, colb, gradv);
						addpixel(nx, ny-x, colr, colg, colb, gradv);
						gradv = gradv/1.5f;
					}
#endif
				}
				if(pixel_mode & PMODE_FLARE)
				{
					flicker = rand()%20;
#ifdef OGLR
					//Oh god, this is awful
					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 0.0f;
					lineV[clineV++] = fnx-10;
					lineV[clineV++] = fny;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 1.0f - ((float)flicker)/40;
					lineV[clineV++] = fnx;
					lineV[clineV++] = fny;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 0.0f;
					lineV[clineV++] = fnx+10;
					lineV[clineV++] = fny;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 0.0f;
					lineV[clineV++] = fnx;
					lineV[clineV++] = fny-10;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 1.0f - ((float)flicker)/30;
					lineV[clineV++] = fnx;
					lineV[clineV++] = fny;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 0.0f;
					lineV[clineV++] = fnx;
					lineV[clineV++] = fny+10;
					cline++;
#else
					gradv = flicker + fabs(parts[i].vx)*17 + fabs(sim->parts[i].vy)*17;
					blendpixel(nx, ny, colr, colg, colb, (gradv*4)>255?255:(gradv*4) );
					blendpixel(nx+1, ny, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					blendpixel(nx-1, ny, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					blendpixel(nx, ny+1, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					blendpixel(nx, ny-1, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					if (gradv>255) gradv=255;
					blendpixel(nx+1, ny-1, colr, colg, colb, gradv);
					blendpixel(nx-1, ny-1, colr, colg, colb, gradv);
					blendpixel(nx+1, ny+1, colr, colg, colb, gradv);
					blendpixel(nx-1, ny+1, colr, colg, colb, gradv);
					for (x = 1; gradv>0.5; x++) {
						addpixel(nx+x, ny, colr, colg, colb, gradv);
						addpixel(nx-x, ny, colr, colg, colb, gradv);
						addpixel(nx, ny+x, colr, colg, colb, gradv);
						addpixel(nx, ny-x, colr, colg, colb, gradv);
						gradv = gradv/1.2f;
					}
#endif
				}
				if(pixel_mode & PMODE_LFLARE)
				{
					flicker = rand()%20;
#ifdef OGLR
					//Oh god, this is awful
					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 0.0f;
					lineV[clineV++] = fnx-70;
					lineV[clineV++] = fny;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 1.0f - ((float)flicker)/30;
					lineV[clineV++] = fnx;
					lineV[clineV++] = fny;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 0.0f;
					lineV[clineV++] = fnx+70;
					lineV[clineV++] = fny;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 0.0f;
					lineV[clineV++] = fnx;
					lineV[clineV++] = fny-70;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 1.0f - ((float)flicker)/50;
					lineV[clineV++] = fnx;
					lineV[clineV++] = fny;
					cline++;

					lineC[clineC++] = ((float)colr)/255.0f;
					lineC[clineC++] = ((float)colg)/255.0f;
					lineC[clineC++] = ((float)colb)/255.0f;
					lineC[clineC++] = 0.0f;
					lineV[clineV++] = fnx;
					lineV[clineV++] = fny+70;
					cline++;
#else
					gradv = flicker + fabs(parts[i].vx)*17 + fabs(parts[i].vy)*17;
					blendpixel(nx, ny, colr, colg, colb, (gradv*4)>255?255:(gradv*4) );
					blendpixel(nx+1, ny, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					blendpixel(nx-1, ny, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					blendpixel(nx, ny+1, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					blendpixel(nx, ny-1, colr, colg, colb, (gradv*2)>255?255:(gradv*2) );
					if (gradv>255) gradv=255;
					blendpixel(nx+1, ny-1, colr, colg, colb, gradv);
					blendpixel(nx-1, ny-1, colr, colg, colb, gradv);
					blendpixel(nx+1, ny+1, colr, colg, colb, gradv);
					blendpixel(nx-1, ny+1, colr, colg, colb, gradv);
					for (x = 1; gradv>0.5; x++) {
						addpixel(nx+x, ny, colr, colg, colb, gradv);
						addpixel(nx-x, ny, colr, colg, colb, gradv);
						addpixel(nx, ny+x, colr, colg, colb, gradv);
						addpixel(nx, ny-x, colr, colg, colb, gradv);
						gradv = gradv/1.01f;
					}
#endif
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
						drad = (M_PI * ((float)orbl[r]) / 180.0f)*1.41f;
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
						drad = (M_PI * ((float)orbl[r]) / 180.0f)*1.41f;
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
#ifdef OGLR
					smokeV[csmokeV++] = nx;
					smokeV[csmokeV++] = ny;
					smokeC[csmokeC++] = ((float)firer)/255.0f;
					smokeC[csmokeC++] = ((float)fireg)/255.0f;
					smokeC[csmokeC++] = ((float)fireb)/255.0f;
					smokeC[csmokeC++] = ((float)firea)/255.0f;
					csmoke++;
#else
					firea /= 2;
					fire_r[ny/CELL][nx/CELL] = (firea*firer + (255-firea)*fire_r[ny/CELL][nx/CELL]) >> 8;
					fire_g[ny/CELL][nx/CELL] = (firea*fireg + (255-firea)*fire_g[ny/CELL][nx/CELL]) >> 8;
					fire_b[ny/CELL][nx/CELL] = (firea*fireb + (255-firea)*fire_b[ny/CELL][nx/CELL]) >> 8;
#endif
				}
				if(firea && (pixel_mode & FIRE_ADD))
				{
#ifdef OGLR
					fireV[cfireV++] = nx;
					fireV[cfireV++] = ny;
					fireC[cfireC++] = ((float)firer)/255.0f;
					fireC[cfireC++] = ((float)fireg)/255.0f;
					fireC[cfireC++] = ((float)fireb)/255.0f;
					fireC[cfireC++] = ((float)firea)/255.0f;
					cfire++;
#else
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
#endif
				}
				if(firea && (pixel_mode & FIRE_SPARK))
				{
#ifdef OGLR
					smokeV[csmokeV++] = nx;
					smokeV[csmokeV++] = ny;
					smokeC[csmokeC++] = ((float)firer)/255.0f;
					smokeC[csmokeC++] = ((float)fireg)/255.0f;
					smokeC[csmokeC++] = ((float)fireb)/255.0f;
					smokeC[csmokeC++] = ((float)firea)/255.0f;
					csmoke++;
#else
					firea /= 4;
					fire_r[ny/CELL][nx/CELL] = (firea*firer + (255-firea)*fire_r[ny/CELL][nx/CELL]) >> 8;
					fire_g[ny/CELL][nx/CELL] = (firea*fireg + (255-firea)*fire_g[ny/CELL][nx/CELL]) >> 8;
					fire_b[ny/CELL][nx/CELL] = (firea*fireb + (255-firea)*fire_b[ny/CELL][nx/CELL]) >> 8;
#endif
				}
			}
		}
	}
#ifdef OGLR

		//Go into array mode
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if(cflat)
		{
			// -- BEGIN FLAT -- //
			//Set point size (size of fire texture)
			glPointSize(1.0f);

			glColorPointer(4, GL_FLOAT, 0, &flatC[0]);
			glVertexPointer(2, GL_INT, 0, &flatV[0]);

			glDrawArrays(GL_POINTS, 0, cflat);

			//Clear some stuff we set
			// -- END FLAT -- //
		}

		if(cblob)
		{
			// -- BEGIN BLOB -- //
			glEnable( GL_POINT_SMOOTH ); //Blobs!
			glPointSize(2.5f);

			glColorPointer(4, GL_FLOAT, 0, &blobC[0]);
			glVertexPointer(2, GL_INT, 0, &blobV[0]);

			glDrawArrays(GL_POINTS, 0, cblob);

			//Clear some stuff we set
			glDisable( GL_POINT_SMOOTH );
			// -- END BLOB -- //
		}

		if(cglow || cblur)
		{
			// -- BEGIN GLOW -- //
			//Start and prepare fire program
			glEnable(GL_TEXTURE_2D);
			glUseProgram(fireProg);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, glowAlpha);
			glUniform1i(glGetUniformLocation(fireProg, "fireAlpha"), 0);

			glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

			//Make sure we can use texture coords on points
			glEnable(GL_POINT_SPRITE);
			glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
			glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

			//Set point size (size of fire texture)
			glPointSize(11.0f);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			if(cglow)
			{
				glColorPointer(4, GL_FLOAT, 0, &glowC[0]);
				glVertexPointer(2, GL_INT, 0, &glowV[0]);

				glDrawArrays(GL_POINTS, 0, cglow);
			}

			glPointSize(7.0f);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			if(cblur)
			{
				glBindTexture(GL_TEXTURE_2D, blurAlpha);

				glColorPointer(4, GL_FLOAT, 0, &blurC[0]);
				glVertexPointer(2, GL_INT, 0, &blurV[0]);

				glDrawArrays(GL_POINTS, 0, cblur);
			}

			//Clear some stuff we set
			glDisable(GL_POINT_SPRITE);
			glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
			glUseProgram(0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
			// -- END GLOW -- //
		}

		if(cadd)
		{
			// -- BEGIN ADD -- //
			//Set point size (size of fire texture)
			glPointSize(1.0f);

			glColorPointer(4, GL_FLOAT, 0, &addC[0]);
			glVertexPointer(2, GL_INT, 0, &addV[0]);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glDrawArrays(GL_POINTS, 0, cadd);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			//Clear some stuff we set
			// -- END ADD -- //
		}

		if(cline)
		{
			// -- BEGIN LINES -- //
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glEnable( GL_LINE_SMOOTH );
			glColorPointer(4, GL_FLOAT, 0, &lineC[0]);
			glVertexPointer(2, GL_FLOAT, 0, &lineV[0]);

			glDrawArrays(GL_LINE_STRIP, 0, cline);

			//Clear some stuff we set
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_LINE_SMOOTH);
			// -- END LINES -- //
		}

		if(cfire || csmoke)
		{
			// -- BEGIN FIRE -- //
			//Start and prepare fire program
			glEnable(GL_TEXTURE_2D);
			glUseProgram(fireProg);
			//glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fireAlpha);
			glUniform1i(glGetUniformLocation(fireProg, "fireAlpha"), 0);

			//Make sure we can use texture coords on points
			glEnable(GL_POINT_SPRITE);
			glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
			glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

			//Set point size (size of fire texture)
			glPointSize(CELL*3);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			if(cfire)
			{
				glColorPointer(4, GL_FLOAT, 0, &fireC[0]);
				glVertexPointer(2, GL_INT, 0, &fireV[0]);

				glDrawArrays(GL_POINTS, 0, cfire);
			}

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			if(csmoke)
			{
				glColorPointer(4, GL_FLOAT, 0, &smokeC[0]);
				glVertexPointer(2, GL_INT, 0, &smokeV[0]);

				glDrawArrays(GL_POINTS, 0, csmoke);
			}

			//Clear some stuff we set
			glDisable(GL_POINT_SPRITE);
			glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
			glUseProgram(0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
			// -- END FIRE -- //
		}

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		//Reset FBO
		glTranslated(0, -MENUSIZE, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevFbo);

		glBlendFunc(origBlendSrc, origBlendDst);
#endif
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
#ifdef OGLR
		GLint prevFbo;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
		glTranslated(0, MENUSIZE, 0);
		float femp_decor = ((float)emp_decor)/255.0f;
		/*int r=emp_decor*2.5, g=100+emp_decor*1.5, b=255;
		int a=(1.0*emp_decor/110)*255;
		if (r>255) r=255;
		if (g>255) g=255;
		if (b>255) g=255;
		if (a>255) a=255;*/
		glBegin(GL_QUADS);
		glColor4f(femp_decor*2.5f, 0.4f+femp_decor*1.5f, 1.0f+femp_decor*1.5f, femp_decor/0.44f);
		glVertex2f(0, MENUSIZE);
		glVertex2f(XRES, MENUSIZE);
		glVertex2f(XRES, WINDOWH);
		glVertex2f(0, WINDOWH);
		glEnd();
		glTranslated(0, -MENUSIZE, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevFbo);
#else
		int r=emp_decor*2.5, g=100+emp_decor*1.5, b=255;
		int a=(1.0*emp_decor/110)*255;
		if (r>255) r=255;
		if (g>255) g=255;
		if (b>255) g=255;
		if (a>255) a=255;
		for (j=0; j<YRES; j++)
			for (i=0; i<XRES; i++)
			{
				blendpixel(i, j, r, g, b, a);
			}
#endif
	}
}

void Renderer::draw_grav()
{
	int x, y, i, ca;
	float nx, ny, dist;

	if(!gravityFieldEnabled)
		return;

	for (y=0; y<YRES/CELL; y++)
	{
		for (x=0; x<XRES/CELL; x++)
		{
			ca = y*(XRES/CELL)+x;
			if(fabsf(sim->gravx[ca]) <= 0.001f && fabsf(sim->gravy[ca]) <= 0.001f)
				continue;
			nx = x*CELL;
			ny = y*CELL;
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
#ifndef OGLR
	if(!(display_mode & DISPLAY_AIR))
		return;
	int x, y, i, j;
	float (*pv)[XRES/CELL] = sim->air->pv;
	float (*hv)[XRES/CELL] = sim->air->hv;
	float (*vx)[XRES/CELL] = sim->air->vx;
	float (*vy)[XRES/CELL] = sim->air->vy;
	pixel c = 0;
	for (y=0; y<YRES/CELL; y++)
		for (x=0; x<XRES/CELL; x++)
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
				float ttemp = hv[y][x]+(-MIN_TEMP);
				int caddress = restrict_flt((int)( restrict_flt(ttemp, 0.0f, MAX_TEMP+(-MIN_TEMP)) / ((MAX_TEMP+(-MIN_TEMP))/1024) ) *3, 0.0f, (1024.0f*3)-3);
				c = PIXRGB((int)(color_data[caddress]*0.7f), (int)(color_data[caddress+1]*0.7f), (int)(color_data[caddress+2]*0.7f));
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
#else
	int sdl_scale = 1;
	GLuint airProg;
	GLint prevFbo;
	if(display_mode & DISPLAY_AIRC)
	{
		airProg = airProg_Cracker;
	}
	else if(display_mode & DISPLAY_AIRV)
	{
		airProg = airProg_Velocity;
	}
	else if(display_mode & DISPLAY_AIRP)
	{
		airProg = airProg_Pressure;
	}
	else
	{
		return;
	}

	glEnable( GL_TEXTURE_2D );
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
	glTranslated(0, MENUSIZE, 0);

	glUseProgram(airProg);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, airVX);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, XRES/CELL, YRES/CELL, GL_RED, GL_FLOAT, sim->air->vx);
	glUniform1i(glGetUniformLocation(airProg, "airX"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, airVY);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, XRES/CELL, YRES/CELL, GL_GREEN, GL_FLOAT, sim->air->vy);
	glUniform1i(glGetUniformLocation(airProg, "airY"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, airPV);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, XRES/CELL, YRES/CELL, GL_BLUE, GL_FLOAT, sim->air->pv);
	glUniform1i(glGetUniformLocation(airProg, "airP"), 2);
	glActiveTexture(GL_TEXTURE0);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2d(1, 1);
	glVertex3f(XRES*sdl_scale, YRES*sdl_scale, 1.0);
	glTexCoord2d(0, 1);
	glVertex3f(0, YRES*sdl_scale, 1.0);
	glTexCoord2d(0, 0);
	glVertex3f(0, 0, 1.0);
	glTexCoord2d(1, 0);
	glVertex3f(XRES*sdl_scale, 0, 1.0);
	glEnd();

	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTranslated(0, -MENUSIZE, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevFbo);
	glDisable( GL_TEXTURE_2D );
#endif
}

void Renderer::draw_grav_zones()
{
	if(!gravityZonesEnabled)
		return;

	int x, y, i, j;
	for (y=0; y<YRES/CELL; y++)
	{
		for (x=0; x<XRES/CELL; x++)
		{
			if(sim->grav->gravmask[y*(XRES/CELL)+x])
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

void Renderer::drawblob(int x, int y, unsigned char cr, unsigned char cg, unsigned char cb)
{
	blendpixel(x+1, y, cr, cg, cb, 112);
	blendpixel(x-1, y, cr, cg, cb, 112);
	blendpixel(x, y+1, cr, cg, cb, 112);
	blendpixel(x, y-1, cr, cg, cb, 112);

	blendpixel(x+1, y-1, cr, cg, cb, 64);
	blendpixel(x-1, y-1, cr, cg, cb, 64);
	blendpixel(x+1, y+1, cr, cg, cb, 64);
	blendpixel(x-1, y+1, cr, cg, cb, 64);
}

pixel Renderer::GetPixel(int x, int y)
{
	if (x<0 || y<0 || x>=VIDXRES || y>=VIDYRES)
		return 0;
#ifdef OGLR
	return 0;	
#else
	return vid[(y*VIDXRES)+x];
#endif
}

Renderer::Renderer(Graphics * g, Simulation * sim):
	sim(NULL),
	g(NULL),
	render_mode(0),
	colour_mode(0),
	display_mode(0),
	gravityZonesEnabled(false),
	gravityFieldEnabled(false),
	decorations_enable(1),
	blackDecorations(false),
	debugLines(false),
	sampleColor(0xFFFFFFFF),
	findingElement(0),
	mousePos(0, 0),
	zoomWindowPosition(0, 0),
	zoomScopePosition(0, 0),
	zoomScopeSize(32),
	zoomEnabled(false),
	ZFACTOR(8),
	gridSize(0)
{
	this->g = g;
	this->sim = sim;
#if !defined(OGLR)
#if defined(OGLI)
	vid = new pixel[VIDXRES*VIDYRES];
#else
	vid = g->vid;
#endif
	persistentVid = new pixel[VIDXRES*YRES];
	warpVid = new pixel[VIDXRES*VIDYRES];
#endif

	memset(fire_r, 0, sizeof(fire_r));
	memset(fire_g, 0, sizeof(fire_g));
	memset(fire_b, 0, sizeof(fire_b));

	//Set defauly display modes
	SetColourMode(COLOUR_DEFAULT);
	AddRenderMode(RENDER_BASC);
	AddRenderMode(RENDER_FIRE);
	AddRenderMode(RENDER_SPRK);

	//Render mode presets. Possibly load from config in future?
	renderModePresets = new RenderPreset[11];

	renderModePresets[0].Name = "Alternative Velocity Display";
	renderModePresets[0].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[0].RenderModes.push_back(RENDER_BASC);
	renderModePresets[0].DisplayModes.push_back(DISPLAY_AIRC);

	renderModePresets[1].Name = "Velocity Display";
	renderModePresets[1].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[1].RenderModes.push_back(RENDER_BASC);
	renderModePresets[1].DisplayModes.push_back(DISPLAY_AIRV);

	renderModePresets[2].Name = "Pressure Display";
	renderModePresets[2].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[2].RenderModes.push_back(RENDER_BASC);
	renderModePresets[2].DisplayModes.push_back(DISPLAY_AIRP);

	renderModePresets[3].Name = "Persistent Display";
	renderModePresets[3].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[3].RenderModes.push_back(RENDER_BASC);
	renderModePresets[3].DisplayModes.push_back(DISPLAY_PERS);

	renderModePresets[4].Name = "Fire Display";
	renderModePresets[4].RenderModes.push_back(RENDER_FIRE);
	renderModePresets[4].RenderModes.push_back(RENDER_SPRK);
	renderModePresets[4].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[4].RenderModes.push_back(RENDER_BASC);

	renderModePresets[5].Name = "Blob Display";
	renderModePresets[5].RenderModes.push_back(RENDER_FIRE);
	renderModePresets[5].RenderModes.push_back(RENDER_SPRK);
	renderModePresets[5].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[5].RenderModes.push_back(RENDER_BLOB);

	renderModePresets[6].Name = "Heat Display";
	renderModePresets[6].RenderModes.push_back(RENDER_BASC);
	renderModePresets[6].DisplayModes.push_back(DISPLAY_AIRH);
	renderModePresets[6].ColourMode = COLOUR_HEAT;

	renderModePresets[7].Name = "Fancy Display";
	renderModePresets[7].RenderModes.push_back(RENDER_FIRE);
	renderModePresets[7].RenderModes.push_back(RENDER_SPRK);
	renderModePresets[7].RenderModes.push_back(RENDER_GLOW);
	renderModePresets[7].RenderModes.push_back(RENDER_BLUR);
	renderModePresets[7].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[7].RenderModes.push_back(RENDER_BASC);
	renderModePresets[7].DisplayModes.push_back(DISPLAY_WARP);

	renderModePresets[8].Name = "Nothing Display";
	renderModePresets[8].RenderModes.push_back(RENDER_BASC);

	renderModePresets[9].Name = "Heat Gradient Display";
	renderModePresets[9].RenderModes.push_back(RENDER_BASC);
	renderModePresets[9].ColourMode = COLOUR_GRAD;

	renderModePresets[10].Name = "Life Gradient Display";
	renderModePresets[10].RenderModes.push_back(RENDER_BASC);
	renderModePresets[10].ColourMode = COLOUR_LIFE;

	//Prepare the graphics cache
	graphicscache = (gcache_item *)malloc(sizeof(gcache_item)*PT_NUM);
	memset(graphicscache, 0, sizeof(gcache_item)*PT_NUM);

	int fireColoursCount = 4;
	pixel fireColours[] = {PIXPACK(0xAF9F0F), PIXPACK(0xDFBF6F), PIXPACK(0x60300F), PIXPACK(0x000000)};
	float fireColoursPoints[] = {1.0f, 0.9f, 0.5f, 0.0f};

	int plasmaColoursCount = 5;
	pixel plasmaColours[] = {PIXPACK(0xAFFFFF), PIXPACK(0xAFFFFF), PIXPACK(0x301060), PIXPACK(0x301040), PIXPACK(0x000000)};
	float plasmaColoursPoints[] = {1.0f, 0.9f, 0.5f, 0.25, 0.0f};

	flm_data = Graphics::GenerateGradient(fireColours, fireColoursPoints, fireColoursCount, 200);
	plasma_data = Graphics::GenerateGradient(plasmaColours, plasmaColoursPoints, plasmaColoursCount, 200);

#ifdef OGLR
	//FBO Texture
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &partsFboTex);
	glBindTexture(GL_TEXTURE_2D, partsFboTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, XRES, YRES, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	//FBO
	glGenFramebuffers(1, &partsFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, partsFbo);
	glEnable(GL_BLEND);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, partsFboTex, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Reset framebuffer binding
	glDisable(GL_TEXTURE_2D);

	//Texture for air to be drawn
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &airBuf);
	glBindTexture(GL_TEXTURE_2D, airBuf);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, XRES/CELL, YRES/CELL, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	//Zoom texture
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &zoomTex);
	glBindTexture(GL_TEXTURE_2D, zoomTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	//Texture for velocity maps for gravity
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &partsTFX);
	glBindTexture(GL_TEXTURE_2D, partsTFX);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, XRES/CELL, YRES/CELL, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	glGenTextures(1, &partsTFY);
	glBindTexture(GL_TEXTURE_2D, partsTFY);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, XRES/CELL, YRES/CELL, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	//Texture for velocity maps for air
	//TODO: Combine all air maps into 3D array or structs
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &airVX);
	glBindTexture(GL_TEXTURE_2D, airVX);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, XRES/CELL, YRES/CELL, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	glGenTextures(1, &airVY);
	glBindTexture(GL_TEXTURE_2D, airVY);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, XRES/CELL, YRES/CELL, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	glGenTextures(1, &airPV);
	glBindTexture(GL_TEXTURE_2D, airPV);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, XRES/CELL, YRES/CELL, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	//Fire alpha texture
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &fireAlpha);
	glBindTexture(GL_TEXTURE_2D, fireAlpha);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, CELL*3, CELL*3, 0, GL_ALPHA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	//Glow alpha texture
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &glowAlpha);
	glBindTexture(GL_TEXTURE_2D, glowAlpha);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 11, 11, 0, GL_ALPHA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);


	//Blur Alpha texture
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &blurAlpha);
	glBindTexture(GL_TEXTURE_2D, blurAlpha);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 7, 7, 0, GL_ALPHA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	//Temptexture
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &textTexture);
	glBindTexture(GL_TEXTURE_2D, textTexture);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	loadShaders();
#endif
	prepare_alpha(CELL, 1.0f);
}

void Renderer::CompileRenderMode()
{
	int old_render_mode = render_mode;
	render_mode = 0;
	for (size_t i = 0; i < render_modes.size(); i++)
		render_mode |= render_modes[i];

	//If firemode is removed, clear the fire display
	if(!(render_mode & FIREMODE) && (old_render_mode & FIREMODE))
	{
		ClearAccumulation();
	}
}

void Renderer::ClearAccumulation()
{
	std::fill(fire_r[0]+0, fire_r[(YRES/CELL)-1]+((XRES/CELL)-1), 0);
	std::fill(fire_g[0]+0, fire_g[(YRES/CELL)-1]+((XRES/CELL)-1), 0);
	std::fill(fire_b[0]+0, fire_b[(YRES/CELL)-1]+((XRES/CELL)-1), 0);
#ifndef OGLR
	std::fill(persistentVid, persistentVid+(VIDXRES*YRES), 0);
#endif
}

void Renderer::AddRenderMode(unsigned int mode)
{
	for (size_t i = 0; i < render_modes.size(); i++)
	{
		if(render_modes[i] == mode)
		{
			return;
		}
	}
	render_modes.push_back(mode);
	CompileRenderMode();
}

void Renderer::RemoveRenderMode(unsigned int mode)
{
	for (size_t i = 0; i < render_modes.size(); i++)
	{
		if(render_modes[i] == mode)
		{
			render_modes.erase(render_modes.begin() + i);
			i = 0;
		}
	}
	CompileRenderMode();
}

void Renderer::SetRenderMode(std::vector<unsigned int> render)
{
	render_modes = render;
	CompileRenderMode();
}

std::vector<unsigned int> Renderer::GetRenderMode()
{
	return render_modes;
}

void Renderer::CompileDisplayMode()
{
	int old_display_mode = display_mode;
	display_mode = 0;
	for (size_t i = 0; i < display_modes.size(); i++)
		display_mode |= display_modes[i];
	if (!(display_mode & DISPLAY_PERS) && (old_display_mode & DISPLAY_PERS))
	{
		ClearAccumulation();
	}
}

void Renderer::AddDisplayMode(unsigned int mode)
{
	for (size_t i = 0; i < display_modes.size(); i++)
	{
		if (display_modes[i] == mode)
		{
			return;
		}
		if (display_modes[i] & DISPLAY_AIR)
		{
			display_modes.erase(display_modes.begin()+i);
		}
	}
	display_modes.push_back(mode);
	CompileDisplayMode();
}

void Renderer::RemoveDisplayMode(unsigned int mode)
{
	for (size_t i = 0; i < display_modes.size(); i++)
	{
		if (display_modes[i] == mode)
		{
			display_modes.erase(display_modes.begin() + i);
			i = 0;
		}
	}
	CompileDisplayMode();
}

void Renderer::SetDisplayMode(std::vector<unsigned int> display)
{
	display_modes = display;
	CompileDisplayMode();
}

std::vector<unsigned int> Renderer::GetDisplayMode()
{
	return display_modes;
}

void Renderer::SetColourMode(unsigned int mode)
{
	colour_mode = mode;
}

unsigned int Renderer::GetColourMode()
{
	return colour_mode;
}

VideoBuffer Renderer::DumpFrame()
{
#ifdef OGLR
#elif defined(OGLI) 
	VideoBuffer newBuffer(XRES, YRES);
	std::copy(vid, vid+(XRES*YRES), newBuffer.Buffer);
	return newBuffer;
#else
	VideoBuffer newBuffer(XRES, YRES);
	for(int y = 0; y < YRES; y++)
	{
		std::copy(vid+(y*WINDOWW), vid+(y*WINDOWW)+XRES, newBuffer.Buffer+(y*XRES));
	}
	return newBuffer;
#endif
}

Renderer::~Renderer()
{
	delete[] renderModePresets;

#if !defined(OGLR)
#if defined(OGLI)
	delete[] vid;
#endif
	delete[] persistentVid;
	delete[] warpVid;
#endif
	free(graphicscache);
	free(flm_data);
	free(plasma_data);
}

#define PIXELMETHODS_CLASS Renderer

#ifdef OGLR
#include "OpenGLDrawMethods.inl"
#else
#include "RasterDrawMethods.inl"
#endif

#undef PIXELMETHODS_CLASS

