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
	if(command.length())
	{
		if (command.substr(0, 6) == "!load ")
			CloseConsole();
		int returnCode = commandInterface->Command(command);
		consoleModel->AddLastCommand(ConsoleCommand(command, returnCode, commandInterface->GetLastError()));
	}
	else
		CloseConsole();
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
	size_t cIndex = consoleModel->GetCurrentCommandIndex();
	if (cIndex < consoleModel->GetPreviousCommands().size())
		consoleModel->SetCurrentCommandIndex(cIndex+1);
}

void ConsoleController::PreviousCommand()
{
	size_t cIndex = consoleModel->GetCurrentCommandIndex();
	if(cIndex > 0)
		consoleModel->SetCurrentCommandIndex(cIndex-1);
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
	delete callback;
	delete consoleModel;
	delete consoleView;
}

