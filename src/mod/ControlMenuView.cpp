#include "ControlMenuView.h"

#include "graphics/Graphics.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Keys.h"
#include "gui/Style.h"

#include "Misc.h"

ControlMenuView::ControlMenuView():
	ui::Window(ui::Point(-1, -1), ui::Point(200, 87)),
	loginButton(new ui::Button(ui::Point(200 - 100, 87 - 17), ui::Point(100, 17), "Sign in")),
	cancelButton(new ui::Button(ui::Point(0, 87 - 17), ui::Point(101, 17), "Sign Out")),
	titleLabel(new ui::Label(ui::Point(4, 5), ui::Point(200 - 16, 16), "Server login")),
	infoLabel(new ui::Label(ui::Point(8, 67), ui::Point(200 - 16, 16), "")),
	targetSize(0, 0)
{
	targetSize = Size;

	infoLabel->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	infoLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	infoLabel->SetMultiline(true);
	infoLabel->Visible = false;
	AddComponent(infoLabel);

	AddComponent(loginButton);
	SetOkayButton(loginButton);
	loginButton->Appearance.HorizontalAlign = ui::Appearance::AlignRight;
	loginButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	loginButton->Appearance.TextInactive = style::Colour::ConfirmButton;
	//loginButton->SetActionCallback({ [this] { c->Login(usernameField->GetText().ToUtf8(), passwordField->GetText().ToUtf8()); } });
	AddComponent(cancelButton);
	cancelButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	cancelButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	//cancelButton->SetActionCallback({ [this] { c->Exit(); } });
	AddComponent(titleLabel);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
}

void ControlMenuView::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
	switch (key)
	{
	default:
		break;
	}
}

void ControlMenuView::OnTryExit(ExitMethod method)
{
	CloseActiveWindow();
}


void ControlMenuView::NotifyStatusChanged(ControlMenuModel* sender)
{
	if (infoLabel->Visible)
		targetSize.Y = 87;
	/*
	infoLabel->SetText(sender->GetStatusText());
	infoLabel->AutoHeight();
	if (sender->GetStatusText().length())
	{
		targetSize.Y += infoLabel->Size.Y + 2;
		infoLabel->Visible = true;
	}
	if (sender->GetStatus())
	{
		c->Exit();
	}
	*/
}


void ControlMenuView::OnTick(float dt)
{
	//if(targetSize != Size)
	{
		ui::Point difference = targetSize - Size;
		if (difference.X != 0)
		{
			int xdiff = difference.X / 5;
			if (xdiff == 0)
				xdiff = 1 * isign(difference.X);
			Size.X += xdiff;
		}
		if (difference.Y != 0)
		{
			int ydiff = difference.Y / 5;
			if (ydiff == 0)
				ydiff = 1 * isign(difference.Y);
			Size.Y += ydiff;
		}

		loginButton->Position.Y = Size.Y - 17;
		cancelButton->Position.Y = Size.Y - 17;
	}
}

void ControlMenuView::OnDraw()
{
	Graphics* g = GetGraphics();
	g->clearrect(Position.X - 2, Position.Y - 2, Size.X + 3, Size.Y + 3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
}

ControlMenuView::~ControlMenuView() {
	RemoveComponent(titleLabel);
	RemoveComponent(loginButton);
	RemoveComponent(cancelButton);
	RemoveComponent(infoLabel);
	delete cancelButton;
	delete loginButton;
	delete titleLabel;
	delete infoLabel;
}