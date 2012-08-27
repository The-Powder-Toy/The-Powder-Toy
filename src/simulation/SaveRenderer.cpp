/*
 * SaveRenderer.cpp
 *
 *  Created on: Apr 3, 2012
 *      Author: Simon
 */

#include "SaveRenderer.h"
#include "client/GameSave.h"
#include "graphics/Graphics.h"
#include "Simulation.h"
#include "graphics/Renderer.h"
#include "search/Thumbnail.h"


SaveRenderer::SaveRenderer(){
	g = new Graphics();
	sim = new Simulation();
	ren = new Renderer(g, sim);
	ren->decorations_enable = false;

#if defined(OGLR) || defined(OGLI)
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &fboTex);
	glBindTexture(GL_TEXTURE_2D, fboTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, XRES, YRES, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	//FBO
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glEnable(GL_BLEND);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Reset framebuffer binding
	glDisable(GL_TEXTURE_2D);
#endif
}

Thumbnail * SaveRenderer::Render(GameSave * save, bool decorations)
{
	int width, height;
	Thumbnail * tempThumb;
	width = save->blockWidth;
	height = save->blockHeight;
	bool doCollapse = save->Collapsed();
	
	g->Acquire();
	g->Clear();
	sim->clear_sim();
	
	if(!sim->Load(save))
	{
		ren->decorations_enable = decorations;
#if defined(OGLR) || defined(OGLI)
		pixel * pData = NULL;
		unsigned char * texData = NULL;

		glTranslated(0, MENUSIZE, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	    glClear(GL_COLOR_BUFFER_BIT);
	    
	    ren->clearScreen(1.0f);
	    ren->ClearAccumulation();
		ren->RenderBegin();
		ren->RenderEnd();

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glTranslated(0, -MENUSIZE, 0);

		glEnable( GL_TEXTURE_2D );
		glBindTexture(GL_TEXTURE_2D, fboTex);

		pData = new pixel[XRES*YRES];
		texData = new unsigned char[(XRES*YRES)*PIXELSIZE];
		std::fill(texData, texData+(XRES*YRES)*PIXELSIZE, 0xDD);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
		glDisable(GL_TEXTURE_2D);

		for(int x = 0; x < width*CELL; x++)
		{
			for(int y = 0; y < height*CELL; y++)
			{
				unsigned char red = texData[((((YRES-1-y)*XRES)+x)*4)];
				unsigned char green = texData[((((YRES-1-y)*XRES)+x)*4)+1];
				unsigned char blue = texData[((((YRES-1-y)*XRES)+x)*4)+2];

				pData[(y*(width*CELL))+x] = PIXRGBA(red, green, blue, 255);
			}
		}

		tempThumb = new Thumbnail(0, 0, pData, ui::Point(width*CELL, height*CELL));
		delete[] pData;
		delete[] texData;
		pData = NULL;
#else
		pixel * pData = NULL;
		pixel * dst;
		pixel * src = g->vid;

	    ren->ClearAccumulation();
		ren->RenderBegin();
		ren->RenderEnd();
	
		pData = (pixel *)malloc(PIXELSIZE * ((width*CELL)*(height*CELL)));
		dst = pData;
		for(int i = 0; i < height*CELL; i++)
		{
			memcpy(dst, src, (width*CELL)*PIXELSIZE);
			dst+=(width*CELL);///PIXELSIZE;
			src+=XRES+BARSIZE;
		}
		tempThumb = new Thumbnail(0, 0, pData, ui::Point(width*CELL, height*CELL));
		if(pData)
			free(pData);
#endif
	}
	if(doCollapse)
		save->Collapse();
	g->Release();
	return tempThumb;
}

Thumbnail * SaveRenderer::Render(unsigned char * saveData, int dataSize, bool decorations)
{
	GameSave * tempSave;
	try {
		tempSave = new GameSave((char*)saveData, dataSize);
	} catch (std::exception & e) {
		
		//Todo: make this look a little less shit
		VideoBuffer buffer(64, 64);
		buffer.BlendCharacter(32, 32, 'x', 255, 255, 255, 255);
		
		Thumbnail * thumb = new Thumbnail(0, 0, buffer.Buffer, ui::Point(64, 64));
		
		return thumb;
	}
	Thumbnail * thumb = Render(tempSave, decorations);
	delete tempSave;
	return thumb;
}

SaveRenderer::~SaveRenderer() {
	// TODO Auto-generated destructor stub
}

