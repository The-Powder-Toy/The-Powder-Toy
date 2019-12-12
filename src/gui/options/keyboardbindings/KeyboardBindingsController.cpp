#include "KeyboardBindingsController.h"

#include "KeyboardBindingsView.h"
#include "Controller.h"
#include "client/Client.h"
#include "../OptionsController.h"
#include "KeyboardBindingsModel.h"

KeyboardBindingsController::KeyboardBindingsController(OptionsController* _parent):
	HasExited(false)
{
	parent = _parent;
	view = new KeyboardBindingsView();
	model = new KeyboardBindingsModel();
	model->AddObserver(view);
	view->AttachController(this);
	LoadBindingPrefs();
	view->BuildKeyBindingsListView();
}

KeyboardBindingsView* KeyboardBindingsController::GetView()
{
	return view;
}

void KeyboardBindingsController::CreateModel(BindingModel _model)
{
	model->CreateModel(_model);
}

void KeyboardBindingsController::Save()
{
	model->Save();
}

void KeyboardBindingsController::ChangeModel(BindingModel _model)
{
	model->RemoveModelByIndex(_model.index);
	model->AddModel(_model);
}

void KeyboardBindingsController::Exit()
{
	view->CloseActiveWindow();
	parent->NotifyKeyBindingsChanged();
	HasExited = true;
}

void KeyboardBindingsController::LoadBindingPrefs()
{
	model->LoadBindingPrefs();
}

std::vector<BindingModel> KeyboardBindingsController::GetBindingPrefs()
{
	return model->GetBindingPrefs();
}

void KeyboardBindingsController::NotifyBindingsChanged()
{
	bool hasConflict = model->HasConflictingCombo();
	model->NotifyBindingsChanged(hasConflict);
}

void KeyboardBindingsController::ForceHasConflict()
{
	view->OnKeyCombinationChanged(true);
}

void KeyboardBindingsController::NotifyKeyReleased()
{
	view->OnKeyReleased();
}

void KeyboardBindingsController::PopBindingByFunctionId(int functionId)
{
	model->PopBindingByFunctionId(functionId);
}

void KeyboardBindingsController::ResetToDefaults()
{
	model->WriteDefaultPrefs(true);
}

bool KeyboardBindingsController::FunctionHasShortcut(int functionId)
{
	return model->FunctionHasShortcut(functionId);
}

KeyboardBindingsController::~KeyboardBindingsController()
{
	view->CloseActiveWindow();
	delete view;
	delete callback;
	delete model;
}

