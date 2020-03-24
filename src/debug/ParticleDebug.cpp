#include "ParticleDebug.h"

#include "gui/game/GameModel.h"

#include "simulation/Simulation.h"

ParticleDebug::ParticleDebug(unsigned int id, Simulation * sim, GameModel * model):
	DebugInfo(id),
	sim(sim),
	model(model)
{

}

void ParticleDebug::Debug(int mode, int x, int y)
{
	int debug_currentParticle = sim->debug_currentParticle;
	int i = 0;
	String logmessage;

	if (mode == 0)
	{
		if (!sim->NUM_PARTS)
			return;
		i = debug_currentParticle;
		while (i < NPART && !sim->parts[i].type)
			i++;
		if (i == NPART)
			logmessage = "End of particles reached, updated sim"_i18n;
		else
			logmessage = String::Build("Updated particle #"_i18n, i);
	}
	else if (mode == 1)
	{
		if (x < 0 || x >= XRES || y < 0 || y >= YRES || !sim->pmap[y][x] || (i = ID(sim->pmap[y][x])) < debug_currentParticle)
		{
			i = NPART;
			auto updated = i18nMulti("Updated particles from #", " to end, updated sim");
			logmessage = String::Build(updated[0], debug_currentParticle, updated[1]);
		}
		else
		{
			auto updated = i18nMulti("Updated particles #", "through #");
			logmessage = String::Build(updated[0], debug_currentParticle, updated[1], i);
		}
	}
	model->Log(logmessage, false);

	if (sim->debug_currentParticle == 0)
	{
		sim->framerender = 1;
		sim->BeforeSim();
		sim->framerender = 0;
	}
	sim->UpdateParticles(debug_currentParticle, i);
	if (i < NPART-1)
		sim->debug_currentParticle = i+1;
	else
	{
		sim->AfterSim();
		sim->debug_currentParticle = 0;
	}
}

bool ParticleDebug::KeyPress(int key, int scan, bool shift, bool ctrl, bool alt, ui::Point currentMouse)
{
	if (key == 'f')
	{
		model->SetPaused(1);
		if (alt)
		{
			Debug(0, 0, 0);
		}
		else if (shift)
		{
			ui::Point mouse = currentMouse;
			if (mouse.X >= XRES)
				mouse.X = XRES-1;
			else if (mouse.X < 0)
				mouse.X = 0;
			if (mouse.Y >= YRES)
				mouse.Y = YRES-1;
			else if (mouse.Y < 0)
				mouse.Y = 0;

			mouse = model->AdjustZoomCoords(mouse);
			Debug(1, mouse.X, mouse.Y);
		}
		else
		{
			if (ctrl)
				return true;
			if (sim->debug_currentParticle > 0)
			{
				sim->UpdateParticles(sim->debug_currentParticle, NPART);
				sim->AfterSim();
				auto updated = i18nMulti("Updated particles from #", " to end, updated sim");
				String logmessage = String::Build(updated[0], sim->debug_currentParticle, updated[1]);
				model->Log(logmessage, false);
				sim->debug_currentParticle = 0;
			}
			else
			{
				model->FrameStep(1);
			}
		}
		return false;
	}
	return true;
}

ParticleDebug::~ParticleDebug()
{

}
