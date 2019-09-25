static int Aa;
#if 0 // temporarily disabled so it doesn't interfere with what I'm doing -- LBPHacker
#include "KeyconfigController.h"

#include "KeyconfigView.h"
#include "Controller.h"
#include "client/Client.h"
#include "gui/options/OptionsController.h"
#include "KeyconfigModel.h"

KeyconfigController::KeyconfigController(OptionsController* _parent):
	HasExited(false)
{
	parent = _parent;
	view = new KeyconfigView();
	model = new KeyconfigModel();
	model->AddObserver(view);
	view->AttachController(this);
	LoadBindingPrefs();
	view->BuildKeyBindingsListView();
}

KeyconfigView* KeyconfigController::GetView()
{
	return view;
}

void KeyconfigController::CreateModel(BindingModel _model)
{
	model->CreateModel(_model);
}

void KeyconfigController::Save()
{
	model->Save();
}

void KeyconfigController::ChangeModel(BindingModel _model)
{
	model->RemoveModelByIndex(_model.index);
	model->AddModel(_model);
}

void KeyconfigController::Exit()
{
	view->CloseActiveWindow();
	parent->NotifyKeyBindingsChanged();
	HasExited = true;
}

void KeyconfigController::LoadBindingPrefs()
{
	model->LoadBindingPrefs();
}

std::vector<BindingModel> KeyconfigController::GetBindingPrefs()
{
	return model->GetBindingPrefs();
}

void KeyconfigController::NotifyBindingsChanged()
{
	bool hasConflict = model->HasConflictingCombo();
	model->NotifyBindingsChanged(hasConflict);
}

void KeyconfigController::ForceHasConflict()
{
	view->OnKeyCombinationChanged(true);
}

void KeyconfigController::NotifyKeyReleased()
{
	view->OnKeyReleased();
}

void KeyconfigController::PopBindingByFunctionId(int functionId)
{
	model->PopBindingByFunctionId(functionId);
}

void KeyconfigController::ResetToDefaults()
{
	model->WriteDefaultPrefs(true);
}

bool KeyconfigController::FunctionHasShortcut(int functionId)
{
	return model->FunctionHasShortcut(functionId);
}

KeyconfigController::~KeyconfigController()
{
	view->CloseActiveWindow();
	delete view;
	delete callback;
	delete model;
}
#endif
