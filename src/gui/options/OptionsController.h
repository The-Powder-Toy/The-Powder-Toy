#ifndef OPTIONSCONTROLLER_H_
#define OPTIONSCONTROLLER_H_

class ControllerCallback;
class GameModel;
class OptionsModel;
class OptionsView;
class OptionsController
{
	GameModel * gModel;
	OptionsView * view;
	OptionsModel * model;
	ControllerCallback * callback;
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
	void SetAltFullscreen(bool altFullscreen);
	void SetForceIntegerScaling(bool forceIntegerScaling);
	void SetScale(int scale);
	void SetResizable(bool resizable);
	void SetFastQuit(bool fastquit);
	void SetShowAvatars(bool showAvatars);
	void SetMouseClickrequired(bool mouseClickRequired);
	
	void Exit();
	OptionsView * GetView();
	virtual ~OptionsController();
};

#endif /* OPTIONSCONTROLLER_H_ */
