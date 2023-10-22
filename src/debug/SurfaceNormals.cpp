#include "SurfaceNormals.h"
#include "gui/game/GameView.h"
#include "gui/game/GameController.h"
#include "gui/interface/Engine.h"
#include "simulation/Simulation.h"
#include "simulation/ElementClasses.h"
#include "graphics/Graphics.h"

SurfaceNormals::SurfaceNormals(unsigned int id, Simulation *newSim, GameView *newView, GameController *newController) :
	DebugInfo(id), sim(newSim), view(newView), controller(newController)
{
}

void SurfaceNormals::Draw()
{
	auto *g = ui::Engine::Ref().g;
	ui::Point pos = controller->PointTranslate(view->GetCurrentMouse());
	auto p = sim->photons[pos.Y][pos.X];
	if (!p)
	{
		p = sim->pmap[pos.Y][pos.X];
	}
	if (!p)
	{
		return;
	}
	auto i = ID(p);
	auto t = TYP(p);
	auto &parts = sim->parts;
	auto x = int(parts[i].x + 0.5f);
	auto y = int(parts[i].y + 0.5f);
	auto mr = sim->PlanMove(i, x, y, false);
	if (t == PT_PHOT)
	{
		if (parts[i].flags & FLAG_SKIPMOVE)
		{
			return;
		}
		if (sim->eval_move(PT_PHOT, mr.fin_x, mr.fin_y, NULL))
		{
			int rt = TYP(sim->pmap[mr.fin_y][mr.fin_x]);
			int lt = TYP(sim->pmap[y][x]);
			int rt_glas = (rt == PT_GLAS) || (rt == PT_BGLA);
			int lt_glas = (lt == PT_GLAS) || (lt == PT_BGLA);
			if ((rt_glas && !lt_glas) || (lt_glas && !rt_glas))
			{
				t |= REFRACT;
			}
		}
	}
	auto gn = sim->get_normal_interp(t, parts[i].x, parts[i].y, mr.vx, mr.vy);
	if (!gn.success)
	{
		return;
	}
	g->XorLine({ x, y }, { mr.fin_x, mr.fin_y });
	g->XorLine({ mr.fin_x, mr.fin_y }, { int((mr.fin_x + gn.nx * SURF_RANGE) + 0.5f), int((mr.fin_y + gn.ny * SURF_RANGE) + 0.5f) });
	g->XorLine({ mr.fin_x, mr.fin_y }, { gn.lx, gn.ly });
	g->XorLine({ mr.fin_x, mr.fin_y }, { gn.rx, gn.ry });
}
