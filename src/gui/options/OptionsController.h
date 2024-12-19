#pragma once
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
	void SetThreadedRendering(bool newThreadedRendering);
	void SetFullscreen(bool fullscreen);
	void SetChangeResolution(bool newChangeResolution);
	void SetForceIntegerScaling(bool forceIntegerScaling);
	void SetBlurryScaling(bool newBlurryScaling);
	void SetScale(int scale);
	void SetGraveExitsConsole(bool graveExitsConsole);
	void SetNativeClipoard(bool nativeClipoard);
	void SetResizable(bool resizable);
	void SetFastQuit(bool fastquit);
	void SetGlobalQuit(bool newGlobalQuit);
	void SetDecoSpace(int decoSpace);
	void SetShowAvatars(bool showAvatars);
	void SetMouseClickrequired(bool mouseClickRequired);
	void SetIncludePressure(bool includePressure);
	void SetPerfectCircle(bool perfectCircle);
	void SetMomentumScroll(bool momentumScroll);
	void SetRedirectStd(bool newRedirectStd);
	
	void Exit();
	OptionsView * GetView();
	virtual ~OptionsController();
};
