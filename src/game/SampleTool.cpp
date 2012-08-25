#include <iostream>
#include "graphics/Graphics.h"
#include "Tool.h"
#include "GameModel.h"
#include "interface/Colour.h"

VideoBuffer * SampleTool::GetIcon(int toolID, int width, int height)
{
	VideoBuffer * newTexture = new VideoBuffer(width, height);
	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{
			pixel pc =  x==0||x==width-1||y==0||y==height-1 ? PIXPACK(0xA0A0A0) : PIXPACK(0x000000);
			newTexture->SetPixel(x, y, PIXR(pc), PIXG(pc), PIXB(pc), 255);
		}
	}
	newTexture->SetCharacter((width/2)-5, (height/2)-5, 0xE6, 255, 255, 255, 255);
	newTexture->BlendCharacter((width/2)-5, (height/2)-5, 0xE7, 100, 180, 255, 255);
	return newTexture;
}

void SampleTool::Draw(Simulation * sim, Brush * brush, ui::Point position)
{
	if(gameModel->GetColourSelectorVisibility())
	{
		pixel colour = gameModel->GetRenderer()->GetPixel(position.X, position.Y);
		gameModel->SetColourSelectorColour(ui::Colour(PIXR(colour), PIXG(colour), PIXB(colour), 255));
	}
	else
	{
		int particleType = 0;
		if(sim->pmap[position.Y][position.X])
			particleType = sim->parts[sim->pmap[position.Y][position.X]>>8].type;
		else if(sim->photons[position.Y][position.X])
			particleType = sim->parts[sim->photons[position.Y][position.X]>>8].type;

		if(particleType)
		{
			Tool * elementTool = gameModel->GetElementTool(particleType);
			if(elementTool)
				gameModel->SetActiveTool(0, elementTool);
		}
	}
}
