#include <stack>
#include "ConsoleController.h"

ConsoleController::ConsoleController(ControllerCallback * callback, CommandInterface * commandInterface):
	HasDone(false)
{
	consoleModel = new ConsoleModel();
	consoleView = new ConsoleView();
	consoleView->AttachController(this);
	consoleModel->AddObserver(consoleView);

	this->callback = callback;
	this->commandInterface = commandInterface;
}

void ConsoleController::EvaluateCommand(std::string command)
{
	consoleModel->ProcessResult(command, FormatCommand(command), commandInterface->Command(command));
}

void ConsoleController::CloseConsole()
{
	if(ui::Engine::Ref().GetWindow() == consoleView)
		ui::Engine::Ref().CloseWindow();
}

std::string ConsoleController::FormatCommand(std::string command)
{
	return commandInterface->FormatCommand(command);
}

void ConsoleController::NextCommand()
{
	consoleModel->NextCommand();
}

void ConsoleController::PreviousCommand()
{
	consoleModel->PreviousCommand();
}

void ConsoleController::Exit()
{
	if(ui::Engine::Ref().GetWindow() == consoleView)
		ui::Engine::Ref().CloseWindow();
	if(callback)
		callback->ControllerExit();
	HasDone = true;
}

ConsoleView * ConsoleController::GetView()
{
	return consoleView;
}

ConsoleController::~ConsoleController() {
	if(ui::Engine::Ref().GetWindow() == consoleView)
		ui::Engine::Ref().CloseWindow();
	if(callback)
		delete callback;
	delete consoleModel;
	delete consoleView;
}

