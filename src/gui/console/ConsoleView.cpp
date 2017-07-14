#include "ConsoleView.h"
#include "gui/interface/Keys.h"

ConsoleView::ConsoleView():
	ui::Window(ui::Point(0, 0), ui::Point(WINDOWW, 150)),
	commandField(NULL)
{
	class CommandHighlighter: public ui::TextboxAction
	{
		ConsoleView * v;
	public:
		CommandHighlighter(ConsoleView * v_) { v = v_; }
		virtual void TextChangedCallback(ui::Textbox * sender)
		{
			sender->SetDisplayText(v->c->FormatCommand(sender->GetText()));
		}
	};
	commandField = new ui::Textbox(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "");
	commandField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	commandField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	commandField->SetActionCallback(new CommandHighlighter(this));
	AddComponent(commandField);
	FocusComponent(commandField);
	commandField->SetBorder(false);
}

void ConsoleView::DoKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	switch(key)
	{
	case SDLK_ESCAPE:
	case '`':
		if (character != '~')
			c->CloseConsole();
		else
			Window::DoKeyPress(key, character, shift, ctrl, alt);
		break;
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
		c->EvaluateCommand(commandField->GetText());
		commandField->SetText("");
		commandField->SetDisplayText("");
		break;
	case SDLK_DOWN:
		c->NextCommand();
		break;
	case SDLK_UP:
		c->PreviousCommand();
		break;
	default:
		Window::DoKeyPress(key, character, shift, ctrl, alt);
		break;
	}
}

void ConsoleView::NotifyPreviousCommandsChanged(ConsoleModel * sender)
{
	for (size_t i = 0; i < commandList.size(); i++)
	{
		RemoveComponent(commandList[i]);
		delete commandList[i];
	}
	commandList.clear();
	std::deque<ConsoleCommand> commands = sender->GetPreviousCommands();
	int currentY = Size.Y - 32;
	if(commands.size())
		for(int i = commands.size()-1; i >= 0; i--)
		{
			if(currentY <= 0)
				break;
			ui::Label * tempLabel = new ui::Label(ui::Point(Size.X/2, currentY), ui::Point(Size.X/2, 16), commands[i].ReturnValue);
			tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
			tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
			commandList.push_back(tempLabel);
			AddComponent(tempLabel);
			tempLabel = new ui::Label(ui::Point(0, currentY), ui::Point(Size.X/2, 16), commands[i].Command);
			tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
			tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
			commandList.push_back(tempLabel);
			AddComponent(tempLabel);
			currentY-=16;
		}
}

void ConsoleView::NotifyCurrentCommandChanged(ConsoleModel * sender)
{
	commandField->SetText(sender->GetCurrentCommand().Command);
	commandField->SetDisplayText(c->FormatCommand(commandField->GetText()));
}


void ConsoleView::OnDraw()
{
	Graphics * g = GetGraphics();
	g->fillrect(Position.X, Position.Y, Size.X, Size.Y, 0, 0, 0, 110);
	g->draw_line(Position.X, Position.Y+Size.Y-16, Position.X+Size.X, Position.Y+Size.Y-16, 255, 255, 255, 160);
	g->draw_line(Position.X, Position.Y+Size.Y, Position.X+Size.X, Position.Y+Size.Y, 255, 255, 255, 200);
}

ConsoleView::~ConsoleView() {
}

