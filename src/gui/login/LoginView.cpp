#include "LoginView.h"

#include "graphics/Graphics.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Keys.h"
#include "gui/Style.h"

class LoginView::LoginAction : public ui::ButtonAction
{
	LoginView * v;
public:
	LoginAction(LoginView * _v) { v = _v; }
	void ActionCallback(ui::Button * sender)
	{
		v->c->Login(v->usernameField->GetText(), v->passwordField->GetText());
	}
};

class LoginView::CancelAction : public ui::ButtonAction
{
	LoginView * v;
public:
	CancelAction(LoginView * _v) { v = _v; }
	void ActionCallback(ui::Button * sender)
	{
		v->c->Exit();
	}
};

LoginView::LoginView():
	ui::Window(ui::Point(-1, -1), ui::Point(200, 87)),
	loginButton(new ui::Button(ui::Point(200-100, 87-17), ui::Point(100, 17), "Sign in")),
	cancelButton(new ui::Button(ui::Point(0, 87-17), ui::Point(101, 17), "Sign Out")),
	titleLabel(new ui::Label(ui::Point(4, 5), ui::Point(200-16, 16), "Server login")),
	infoLabel(new ui::Label(ui::Point(8, 67), ui::Point(200-16, 16), "")),
	usernameField(new ui::Textbox(ui::Point(8, 25), ui::Point(200-16, 17), Client::Ref().GetAuthUser().Username, "[username]")),
	passwordField(new ui::Textbox(ui::Point(8, 46), ui::Point(200-16, 17), "", "[password]")),
	targetSize(0, 0)
{
	targetSize = Size;
	FocusComponent(usernameField);
	
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
	loginButton->SetActionCallback(new LoginAction(this));
	AddComponent(cancelButton);
	cancelButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	cancelButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	cancelButton->SetActionCallback(new CancelAction(this));
	AddComponent(titleLabel);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	
	AddComponent(usernameField);
	usernameField->Appearance.icon = IconUsername;
	usernameField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	usernameField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(passwordField);
	passwordField->Appearance.icon = IconPassword;
	passwordField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	passwordField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	passwordField->SetHidden(true);
}

void LoginView::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	switch(key)
	{
	case SDLK_TAB:
		if(IsFocused(usernameField))
			FocusComponent(passwordField);
		else
			FocusComponent(usernameField);
		break;
	}
}

void LoginView::OnTryExit(ExitMethod method)
{
	CloseActiveWindow();
}

void LoginView::NotifyStatusChanged(LoginModel * sender)
{
	if (infoLabel->Visible)
		targetSize.Y = 87;
	infoLabel->SetText(sender->GetStatusText());
	infoLabel->AutoHeight();
	if (sender->GetStatusText().length())
	{
		targetSize.Y += infoLabel->Size.Y+2;
		infoLabel->Visible = true;
	}
	if(sender->GetStatus())
	{
		c->Exit();
	}
}

void LoginView::OnTick(float dt)
{
	//if(targetSize != Size)
	{
		ui::Point difference = targetSize-Size;
		if(difference.X!=0)
		{
			int xdiff = difference.X/5;
			if(xdiff == 0)
				xdiff = 1*isign(difference.X);
			Size.X += xdiff;
		}
		if(difference.Y!=0)
		{
			int ydiff = difference.Y/5;
			if(ydiff == 0)
				ydiff = 1*isign(difference.Y);
			Size.Y += ydiff;
		}
		
		loginButton->Position.Y = Size.Y-17;
		cancelButton->Position.Y = Size.Y-17;
	}
}

void LoginView::OnDraw()
{
	Graphics * g = GetGraphics();
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
}

LoginView::~LoginView() {
	RemoveComponent(titleLabel);
	RemoveComponent(loginButton);
	RemoveComponent(cancelButton);
	RemoveComponent(usernameField);
	RemoveComponent(passwordField);
	RemoveComponent(infoLabel);
	delete cancelButton;
	delete loginButton;
	delete titleLabel;
	delete usernameField;
	delete passwordField;
	delete infoLabel;
}

