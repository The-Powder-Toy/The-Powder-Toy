#ifndef OPTIONSCONTROLLER_H_
#define OPTIONSCONTROLLER_H_

#include "Controller.h"
#include "simulation/Simulation.h"
#include "OptionsView.h"
#include "OptionsModel.h"

class GameModel;
class OptionsModel;
class OptionsView;
class OptionsController {
	GameModel * gModel;
	OptionsView * view;
	OptionsModel * model;
	ControllerCallback * callback;
	int depth3d;
public:
	bool HasExited;
	OptionsController(GameModel * gModel_, ControllerCallback * callback_);
	void SetHeatSimulation(bool state);
	void SetAmbientHeatSimulation(bool state);
	void SetNewtonianGravity(bool state);
	void SetWaterEqualisation(bool state);
	void SetGravityMode(int gravityMode);
	void SetAirMode(int airMode);
	void SetEdgeMode(int edgeMode);
	void SetFullscreen(bool fullscreen);
	void SetScale(bool scale);
	void SetFastQuit(bool fastquit);
	void SetShowAvatars(bool showAvatars);
	void Set3dDepth(int depth);
	void Exit();
	OptionsView * GetView();
	virtual ~OptionsController();
};

#endif /* OPTIONSCONTROLLER_H_ */
