#ifndef KEYCONFIGSCONTROLLER_H
#define KEYCONFIGSCONTROLLER_H

#include <vector>

class ControllerCallback;
class KeyconfigView;
class GameModel;
class OptionsController;
class KeyconfigModel;
struct BindingModel;

class KeyconfigController
{
	ControllerCallback * callback;
	KeyconfigView* view;
	KeyconfigModel* model;
	OptionsController* parent;
public:
	bool HasExited;
	KeyconfigController(OptionsController* _parent);	
	void Exit();
	KeyconfigView * GetView();
	virtual ~KeyconfigController();
	void AddModel(BindingModel model);
	void CreateModel(BindingModel model);
	void ChangeModel(BindingModel model);
	void Save();
	void ForceHasConflict();
	void NotifyKeyReleased();
	void OnKeyReleased();
	void NotifyBindingsChanged();
	void PopBindingByFunctionId(int functionId);
	bool FunctionHasShortcut(int functionId);
	void ResetToDefaults();
	
	void LoadBindingPrefs();
	std::vector<BindingModel> GetBindingPrefs();
};

#endif /* KEYCONFIGSCONTROLLER_H */
