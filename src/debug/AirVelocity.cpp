#include "AirVelocity.h"
#include "gui/game/GameView.h"
#include "gui/game/GameController.h"
#include "gui/interface/Engine.h"
#include "simulation/Simulation.h"
#include "simulation/ElementClasses.h"
#include "graphics/Graphics.h"

AirVelocity::AirVelocity(unsigned int id, const Simulation *newSim, GameView *newView, GameController *newController) :
	DebugInfo(id), sim(newSim), view(newView), controller(newController)
{
}

void AirVelocity::Draw()
{
	auto *g = ui::Engine::Ref().g;
	ui::Point pos = controller->PointTranslate(view->GetCurrentMouse());

	float velx = sim->vx[pos.Y/CELL][pos.X/CELL];
	float vely = sim->vy[pos.Y/CELL][pos.X/CELL];
	int endx = pos.X + (int)(10.0f*velx);
	int endy = pos.Y + (int)(10.0f*vely);
	
	//Air velocity line near cursor
	g->XorLine({pos.X, pos.Y}, {endx, endy});

	//Air velocity magnitude
	float vlen = std::sqrt(velx*velx + vely*vely);
	if (vlen > 0.001f)
	{
		StringBuilder velocity;
		velocity << Format::Precision(2) << vlen;

		int width = Graphics::TextSize(velocity.Build()).X;
		int height = Graphics::TextSize(velocity.Build()).Y;

		//Rectangle in polar coordinates, here diff is radius
		int dx = endx - pos.X;
		int dy = endy - pos.Y;
		float diff = 0;
		if (width*std::abs(dy) < height*std::abs(dx))
			diff = ((float)width)/((float)std::abs(dx));
		else
			diff = (dy == 0) ? 0 : (((float)height)/((float)std::abs(dy)));

		float circx = (diff/1.5f) * (endx - pos.X) + endx;
		float circy = (diff/1.5f) * (endy - pos.Y) + endy;

		g->BlendText({(int)circx - width/2, (int)circy - height/2 + 2}, velocity.Build(), 0xFFFFFF_rgb .WithAlpha(255));
	}
}
