#include "DebugLines.h"
#include "gui/game/GameView.h"
#include "gui/game/GameController.h"
#include "graphics/Graphics.h"
#include "SimulationConfig.h"

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

		g->BlendLine({ 0, drawPoint1.Y }, { XRES, drawPoint1.Y }, 0xFFFFFF_rgb .WithAlpha(120));
		g->BlendLine({ drawPoint1.X, 0 }, { drawPoint1.X, YRES }, 0xFFFFFF_rgb .WithAlpha(120));

		g->BlendLine({ 0, drawPoint2.Y }, { XRES, drawPoint2.Y }, 0xFFFFFF_rgb .WithAlpha(120));
		g->BlendLine({ drawPoint2.X, 0 }, { drawPoint2.X, YRES }, 0xFFFFFF_rgb .WithAlpha(120));

		String info;
		info = String::Build(drawPoint2.X, " x ", drawPoint2.Y);
		g->BlendTextOutline({ drawPoint2.X+(drawPoint2.X>drawPoint1.X?3:-(g->TextSize(info).X-1)-3), drawPoint2.Y+(drawPoint2.Y<drawPoint1.Y?-10:3) }, info, 0xFFFFFF_rgb .WithAlpha(200));

		info = String::Build(drawPoint1.X, " x ", drawPoint1.Y);
		g->BlendTextOutline({ drawPoint1.X+(drawPoint2.X<drawPoint1.X?3:-(g->TextSize(info).X-1)-2), drawPoint1.Y+(drawPoint2.Y>drawPoint1.Y?-10:3) }, info, 0xFFFFFF_rgb .WithAlpha(200));

		info = String::Build(std::abs(drawPoint2.X-drawPoint1.X));
		g->BlendTextOutline({ (drawPoint1.X+drawPoint2.X)/2-(g->TextSize(info).X-1)/2, drawPoint1.Y+(drawPoint2.Y>drawPoint1.Y?-10:3) }, info, 0xFFFFFF_rgb .WithAlpha(200));

		info = String::Build(std::abs(drawPoint2.Y-drawPoint1.Y));
		g->BlendTextOutline({ drawPoint1.X+(drawPoint2.X<drawPoint1.X?3:-(g->TextSize(info).X-1)-2), (drawPoint1.Y+drawPoint2.Y)/2-3 }, info, 0xFFFFFF_rgb .WithAlpha(200));
	}
}

DebugLines::~DebugLines()
{

}
