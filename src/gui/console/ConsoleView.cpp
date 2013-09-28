#include "ConsoleView.h"
#include "Misc.h"
#include <algorithm>
#include "gui/interface/Keys.h"

ConsoleView::ConsoleView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES+BARSIZE, 320))
{
	class CommandCallback: public ui::TextboxAction
	{
		ConsoleView * v;
	public:
		CommandCallback(ConsoleView * v_) { v = v_; }
		virtual void TextChangedCallback(ui::Textbox * sender)
		{
			v->ResizePrompt();
			sender->SetDisplayText(wordwrap(v->c->FormatCommand(sender->GetText()),sender->Size.X-(sender->Appearance.Margin.Left+sender->Appearance.Margin.Right)));
		}
	};
	class TransparentScrollPanel: public ui::ScrollPanel
	{
	public:
		TransparentScrollPanel(ui::Point position, ui::Point size):
			ScrollPanel(position, size)
		{
		}
		void Draw(const ui::Point& screenPos)
		{
			using namespace ui;
			XDraw(screenPos);
#ifdef OGLI
			GLint lastVid;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastVid);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, myVid);
			glClearColor(0.7f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
#else
			Graphics * g = ui::Engine::Ref().g;
			pixel * lastVid = g->vid;
			g->vid = myVid;
			for (int row = 0; row < Size.Y; row++)
				std::copy(lastVid+((screenPos.Y+row)*(XRES+BARSIZE)+screenPos.X), lastVid+((screenPos.Y+row)*(XRES+BARSIZE)+screenPos.X+Size.X), myVid+(row*(XRES+BARSIZE)));
#endif
			
			// attempt to draw all children
			for(int i = 0; i < children.size(); ++i)
			{
				// the component must be visible
				if(children[i]->Visible)
				{
					//check if the component is in the screen, draw if it is
					if( children[i]->Position.X + ViewportPosition.X + children[i]->Size.X >= 0 &&
						children[i]->Position.Y + ViewportPosition.Y + children[i]->Size.Y >= 0 &&
						children[i]->Position.X + ViewportPosition.X < ui::Engine::Ref().GetWidth() &&
						children[i]->Position.Y + ViewportPosition.Y < ui::Engine::Ref().GetHeight() )
					{
						Point scrpos = /*screenPos + */children[i]->Position + ViewportPosition;
						children[i]->Draw(scrpos);
					}
				}
			}

#ifdef OGLI
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lastVid);

			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, myVidTex);

			int x = screenPos.X, y = screenPos.Y;
			int h = Size.Y, w = Size.X;

			double texX = double(Size.X)/double(XRES+BARSIZE), texY = 1, texYB = 1-(double(Size.Y)/double(YRES+MENUSIZE));

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glBegin(GL_QUADS);
			glTexCoord2d(0, texYB);
			glVertex2f(x, y+h);
			glTexCoord2d(texX, texYB);
			glVertex2f(x+w, y+h);
			glTexCoord2d(texX, texY);
			glVertex2f(x+w, y);
			glTexCoord2d(0, texY);
			glVertex2f(x, y);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
#else
			g->vid = lastVid;
			for (int row = 0; row < Size.Y; row++)
				std::copy(myVid+(row*(XRES+BARSIZE)), myVid+(row*(XRES+BARSIZE))+Size.X, lastVid+((screenPos.Y+row)*(XRES+BARSIZE))+screenPos.X);
#endif
		}		
	};
	commandField = new ui::Textbox(ui::Point(17, Size.Y-16), ui::Point(Size.X-17, 16), "");
	commandField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	commandField->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	commandField->SetMultiline(true);
	commandField->SetActionCallback(new CommandCallback(this));
	commandField->SetBorder(false);
	AddComponent(commandField);
	FocusComponent(commandField);
	
	promptLabel = new ui::Label(ui::Point(0, Size.Y-16), ui::Point(10, 16), "> ");
	promptLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	promptLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	AddComponent(promptLabel);

	history = new TransparentScrollPanel(ui::Point(0, 0), ui::Point(Size.X, Size.Y-17));
	AddComponent(history);

	commandHistory = new ui::Label(ui::Point(17, 0), ui::Point(Size.X-17, 1), "");
	commandHistory->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	commandHistory->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	commandHistory->SetMultiline(true);
	history->AddChild(commandHistory);
	
	promptHistory = new ui::Label(ui::Point(0, 0), ui::Point(23, 1), "");
	promptHistory->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	promptHistory->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	promptHistory->SetMultiline(true);
	history->AddChild(promptHistory);
}

void ConsoleView::DoKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	switch(key)
	{
	case KEY_ESCAPE:
	case '`':
		if (character != '~')
			c->CloseConsole();
		else
			Window::DoKeyPress(key, character, shift, ctrl, alt);
		break;
	case KEY_RETURN:
	case KEY_ENTER:
		c->EvaluateCommand(commandField->GetText());
		break;
	case KEY_DOWN:
		c->NextCommand();
		break;
	case KEY_UP:
		c->PreviousCommand();
		break;
	default:
		Window::DoKeyPress(key, character, shift, ctrl, alt);
		break;
	}
}

void ConsoleView::NotifyHistoryChanged(ConsoleModel * sender, std::string command, std::string prompthistory, std::string History)
{
	commandField->SetDisplayText("");
	commandField->SetText(command);
	ResizePrompt();
	promptHistory->SetText(prompthistory);
	promptHistory->AutoHeight();
	commandHistory->SetText(History);
	commandHistory->AutoHeight();
	history->SetScrollPosition(history->InnerSize.Y = commandHistory->Size.Y);
}

void ConsoleView::ResizePrompt()
{
	std::string command = commandField->GetText();
	std::string prompt = ">";
	int newlines = std::count(command.begin(), command.end(), '\n');
	for(int i=0;i<newlines;i++)
		prompt += "\n>>";
	promptLabel->SetText(prompt);
	commandField->AutoHeight();
	commandField->Size.Y = std::max(commandField->Size.Y+2, 16);
	history->Size.Y = promptLabel->Position.Y = commandField->Position.Y = Size.Y-commandField->Size.Y;
}

void ConsoleView::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->fillrect(Position.X, Position.Y, Size.X, Size.Y, 0, 61, 76, 180);
	g->draw_line(Position.X, Position.Y+Size.Y, Position.X+Size.X, Position.Y+Size.Y, 255, 255, 255, 200);
}

ConsoleView::~ConsoleView() {
}

