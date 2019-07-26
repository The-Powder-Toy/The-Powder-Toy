#ifndef KEYBOARDBINDINGSCONTROLLER_H
#define KEYBOARDBINDINGSCONTROLLER_H

#include <vector>

class ControllerCallback;
class KeyboardBindingsView;
class GameModel;
class OptionsController;
class KeyboardBindingsModel;
struct BindingModel;

class KeyboardBindingsController
{
	ControllerCallback * callback;
	KeyboardBindingsView* view;
	KeyboardBindingsModel* model;
	OptionsController* parent;
public:
	bool HasExited;
	KeyboardBindingsController(OptionsController* _parent);	
	void Exit();
	KeyboardBindingsView * GetView();
	virtual ~KeyboardBindingsController();
	void AddModel(BindingModel model);
	void CreateModel(BindingModel model);
	void ChangeModel(BindingModel model);
	void Save();
	void ForceHasConflict();
	void NotifyKeyReleased();
	void OnKeyReleased();
	void NotifyBindingsChanged();
	void PopBindingByFunctionId(int32_t functionId);
	bool FunctionHasShortcut(int32_t functionId);
	void ResetToDefaults();
	
	void LoadBindingPrefs();
	std::vector<BindingModel> GetBindingPrefs();
};

#endif /* KEYBOARDBINDINGSCONTROLLER_H */
