#ifndef OPTIONSMODEL_H_
#define OPTIONSMODEL_H_
#include <vector>
#include "OptionsView.h"
#include "simulation/Simulation.h"

class GameModel;
class Simulation;
class OptionsView;
class OptionsModel {
	GameModel * gModel;
	Simulation * sim;
	std::vector<OptionsView*> observers;
	void notifySettingsChanged();
public:
	OptionsModel(GameModel * gModel);
	void AddObserver(OptionsView* view);
	bool GetHeatSimulation();
	void SetHeatSimulation(bool state);
	bool GetAmbientHeatSimulation();
	void SetAmbientHeatSimulation(bool state);
	bool GetNewtonianGravity();
	void SetNewtonianGravity(bool state);
	bool GetWaterEqualisation();
	void SetWaterEqualisation(bool state);
	bool GetShowAvatars();
	void SetShowAvatars(bool state);
	int GetAirMode();
	void SetAirMode(int airMode);
	int GetEdgeMode();
	void SetEdgeMode(int edgeMode);
	int GetGravityMode();
	void SetGravityMode(int gravityMode);
	int GetScale();
	void SetScale(int scale);
	bool GetFullscreen();
	void SetFullscreen(bool fullscreen);
	bool GetFastQuit();
	void SetFastQuit(bool fastquit);
	virtual ~OptionsModel();
};

#endif /* OPTIONSMODEL_H_ */
