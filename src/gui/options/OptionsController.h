#ifndef OPTIONSCONTROLLER_H_
#define OPTIONSCONTROLLER_H_
#include "Config.h"

#include <functional>

class GameModel;
class OptionsModel;
class OptionsView;
class OptionsController
{
	GameModel * gModel;
	OptionsView * view;
	OptionsModel * model;
	std::function<void ()> onDone;
public:
	bool HasExited;
	OptionsController(GameModel * gModel_, std::function<void ()> onDone = nullptr);
	void SetHeatSimulation(bool state);
	void SetAmbientHeatSimulation(bool state);
	void SetNewtonianGravity(bool state);
	void SetWaterEqualisation(bool state);
	void SetGravityMode(int gravityMode);
	void SetCustomGravityX(float x);
	void SetCustomGravityY(float y);
	void SetAirMode(int airMode);
	void SetAmbientAirTemperature(float ambientAirTemp);
	void SetEdgeMode(int edgeMode);
	void SetTemperatureScale(int temperatureScale);
	void SetFullscreen(bool fullscreen);
	void SetAltFullscreen(bool altFullscreen);
	void SetForceIntegerScaling(bool forceIntegerScaling);
	void SetScale(int scale);
	void SetResizable(bool resizable);
	void SetFastQuit(bool fastquit);
	void SetDecoSpace(int decoSpace);
	void SetShowAvatars(bool showAvatars);
	void SetMouseClickrequired(bool mouseClickRequired);
	void SetIncludePressure(bool includePressure);
	void SetPerfectCircle(bool perfectCircle);
	void SetMomentumScroll(bool momentumScroll);
	
	void Exit();
	OptionsView * GetView();
	virtual ~OptionsController();
};

#endif /* OPTIONSCONTROLLER_H_ */
