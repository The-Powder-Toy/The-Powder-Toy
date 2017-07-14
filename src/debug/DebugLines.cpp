#include "DebugLines.h"
#include "gui/game/GameView.h"
#include "gui/game/GameController.h"

DebugLines::DebugLines(unsigned int id, GameView * view, GameController * controller):
	DebugInfo(id),
	view(view),
	controller(controller)
{

}

void DebugLines::Draw()
{
	Graphics * g = view->GetGraphics();

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

		std::stringstream info;
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
}

DebugLines::~DebugLines()
{

}
