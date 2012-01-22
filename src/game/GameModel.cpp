#include "interface/Engine.h"
#include "GameModel.h"
#include "GameView.h"
#include "simulation/Simulation.h"
#include "Renderer.h"
#include "interface/Point.h"
#include "Brush.h"

GameModel::GameModel():
	activeElement(1),
	sim(NULL),
	ren(NULL),
	currentSave(NULL),
	currentBrush(new Brush(ui::Point(4, 4)))
{
	sim = new Simulation();
	ren = new Renderer(ui::Engine::Ref().g, sim);
}

GameModel::~GameModel()
{
	delete sim;
	delete ren;
}

Brush * GameModel::GetBrush()
{
	return currentBrush;
}

void GameModel::AddObserver(GameView * observer){
	observers.push_back(observer);

	observer->NotifySimulationChanged(this);
	observer->NotifyRendererChanged(this);
	observer->NotifyPausedChanged(this);
	observer->NotifySaveChanged(this);
	observer->NotifyBrushChanged(this);
}

int GameModel::GetActiveElement()
{
	return activeElement;
}

void GameModel::SetActiveElement(int element)
{
	activeElement = element;
}

Save * GameModel::GetSave()
{
	return currentSave;
}
void GameModel::SetSave(Save * newSave)
{
	currentSave = newSave;
	notifySaveChanged();
}

Simulation * GameModel::GetSimulation()
{
	return sim;
}

Renderer * GameModel::GetRenderer()
{
	return ren;
}

void GameModel::SetPaused(bool pauseState)
{
	sim->sys_pause = pauseState?1:0;
	notifyPausedChanged();
}

bool GameModel::GetPaused()
{
	return sim->sys_pause?true:false;
}

void GameModel::ClearSimulation()
{
	sim->clear_sim();
}

void GameModel::notifyRendererChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyRendererChanged(this);
	}
}

void GameModel::notifySaveChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySaveChanged(this);
	}
}

void GameModel::notifySimulationChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySimulationChanged(this);
	}
}

void GameModel::notifyPausedChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyPausedChanged(this);
	}
}

void GameModel::notifyBrushChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyBrushChanged(this);
	}
}
