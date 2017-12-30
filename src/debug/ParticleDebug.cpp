#include <sstream>
#include "ParticleDebug.h"
#include "gui/interface/Engine.h"
#include "gui/game/GameView.h"
#include "gui/game/GameController.h"

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
	std::stringstream logmessage;

	if (mode == 0)
	{
		if (!sim->NUM_PARTS)
			return;
		i = debug_currentParticle;
		while (i < NPART && !sim->parts[i].type)
			i++;
		if (i == NPART)
			logmessage << "End of particles reached, updated sim";
		else
			logmessage << "Updated particle #" << i;
	}
	else if (mode == 1)
	{
		if (x < 0 || x >= XRES || y < 0 || y >= YRES || !sim->pmap[y][x] || (i = ID(sim->pmap[y][x])) < debug_currentParticle)
		{
			i = NPART;
			logmessage << "Updated particles from #" << debug_currentParticle << " to end, updated sim";
		}
		else
			logmessage << "Updated particles #" << debug_currentParticle << " through #" << i;
	}
	model->Log(logmessage.str(), false);

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

bool ParticleDebug::KeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt, ui::Point currentMouse)
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
				std::stringstream logmessage;
				logmessage << "Updated particles from #" << sim->debug_currentParticle << " to end, updated sim";
				model->Log(logmessage.str(), false);
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
