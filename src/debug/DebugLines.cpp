#include "DebugLines.h"
#include "gui/interface/Engine.h"
#include "gui/game/GameView.h"
#include "gui/game/GameController.h"

LineDebug::LineDebug(GameView * view, GameController * controller):
	view(view),
	controller(controller)
{

}

void LineDebug::Draw()
{
	Graphics * g = ui::Engine::Ref().g;

	if (view->GetDrawingLine())
	{
		ui::Point drawPoint1 = controller->PointTranslate(view->GetLineStartCoords()), drawPoint2 = controller->PointTranslate(view->GetLineFinishCoords());
		if (view->GetDrawSnap())
			drawPoint2 = view->lineSnapCoords(drawPoint1, drawPoint2);
		//g->draw_line(drawPoint1.X, drawPoint1.Y, drawPoint2.X, drawPoint2.Y, 255, 0, 255, 255);

		g->draw_line(0, drawPoint1.Y, XRES, drawPoint1.Y, 255, 255, 255, 120);
		g->draw_line(drawPoint1.X, 0, drawPoint1.X, YRES, 255, 255, 255, 120);

		g->draw_line(0, drawPoint2.Y, XRES, drawPoint2.Y, 255, 255, 255, 120);
		g->draw_line(drawPoint2.X, 0, drawPoint2.X, YRES, 255, 255, 255, 120);

		stringstream info;
		info << drawPoint2.X << " x " << drawPoint2.Y;
		g->drawtext_outline(drawPoint2.X+(drawPoint2.X>drawPoint1.X?3:-g->textwidth(info.str().c_str())-3), drawPoint2.Y+(drawPoint2.Y<drawPoint1.Y?-10:3), info.str().c_str(), 255, 255, 255, 200);

		info.str("");
		info << drawPoint1.X << " x " << drawPoint1.Y;
		g->drawtext_outline(drawPoint1.X+(drawPoint2.X<drawPoint1.X?3:-g->textwidth(info.str().c_str())-2), drawPoint1.Y+(drawPoint2.Y>drawPoint1.Y?-10:3), info.str().c_str(), 255, 255, 255, 200);

		info.str("");
		info << std::abs(drawPoint2.X-drawPoint1.X);
		g->drawtext_outline((drawPoint1.X+drawPoint2.X)/2-g->textwidth(info.str().c_str())/2, drawPoint1.Y+(drawPoint2.Y>drawPoint1.Y?-10:3), info.str().c_str(), 255, 255, 255, 200);

		info.str("");
		info << std::abs(drawPoint2.Y-drawPoint1.Y);
		g->drawtext_outline(drawPoint1.X+(drawPoint2.X<drawPoint1.X?3:-g->textwidth(info.str().c_str())-2), (drawPoint1.Y+drawPoint2.Y)/2-3, info.str().c_str(), 255, 255, 255, 200);
	}
	/*int yBottom = YRES-10;
	int xStart = 10;

	std::string maxValString;
	std::string halfValString;


	float maxVal = 255;
	float scale = 1.0f;
	int bars = 0;
	for(int i = 0; i < PT_NUM; i++)
	{
		if(sim->elements[i].Enabled)
		{
			if(maxVal < sim->elementCount[i])
				maxVal = sim->elementCount[i];
			bars++;
		}
	}
	maxAverage = (maxAverage*(1.0f-0.015f)) + (0.015f*maxVal);
	scale = 255.0f/maxAverage;

	maxValString = format::NumberToString<int>(maxAverage);
	halfValString = format::NumberToString<int>(maxAverage/2);
	

	g->fillrect(xStart-5, yBottom - 263, bars+10+Graphics::textwidth(maxValString.c_str())+10, 255 + 13, 0, 0, 0, 180);

	bars = 0;
	for(int i = 0; i < PT_NUM; i++)
	{
		if(sim->elements[i].Enabled)
		{
			float count = sim->elementCount[i];
			int barSize = (count * scale - 0.5f);
			int barX = bars;//*2;

			g->draw_line(xStart+barX, yBottom+3, xStart+barX, yBottom+2, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour), 255);
			if(sim->elementCount[i])	
			{
				if(barSize > 256)
				{
					barSize = 256;
					g->blendpixel(xStart+barX, yBottom-barSize-3, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour), 255);
					g->blendpixel(xStart+barX, yBottom-barSize-5, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour), 255);
					g->blendpixel(xStart+barX, yBottom-barSize-7, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour), 255);
				} else {			

					g->draw_line(xStart+barX, yBottom-barSize-3, xStart+barX, yBottom-barSize-2, 255, 255, 255, 180);
				}
				g->draw_line(xStart+barX, yBottom-barSize, xStart+barX, yBottom, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour), 255);
			}
			bars++;
		}
	}

	g->drawtext(xStart + bars + 5, yBottom-5, "0", 255, 255, 255, 255);
	g->drawtext(xStart + bars + 5, yBottom-132, halfValString, 255, 255, 255, 255);
	g->drawtext(xStart + bars + 5, yBottom-260, maxValString, 255, 255, 255, 255);*/
}

LineDebug::~LineDebug()
{

}
