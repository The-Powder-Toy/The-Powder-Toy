#include "LoginView.h"
#include "Config.h"
#include "LoginModel.h"
#include "LoginController.h"
#include "graphics/Graphics.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/RichLabel.h"
#include "gui/interface/Textbox.h"
#include "gui/Style.h"
#include "client/Client.h"
#include "Misc.h"
#include <SDL.h>

constexpr auto defaultSize = ui::Point(200, 87);

LoginView::LoginView():
	ui::Window(ui::Point(-1, -1), defaultSize),
	loginButton(new ui::Button(ui::Point(200-100, 87-17), ui::Point(100, 17), "Sign in")),
	cancelButton(new ui::Button(ui::Point(0, 87-17), ui::Point(101, 17), "Sign Out")),
	titleLabel(new ui::Label(ui::Point(4, 5), ui::Point(200-16, 16), "Server login")),
	infoLabel(new ui::RichLabel(ui::Point(6, 67), ui::Point(200-12, 16), "")),
	usernameField(new ui::Textbox(ui::Point(8, 25), ui::Point(200-16, 17), Client::Ref().GetAuthUser().Username.FromUtf8(), "[username]")),
	passwordField(new ui::Textbox(ui::Point(8, 46), ui::Point(200-16, 17), "", "[password]"))
{
	targetSize.SetTarget(Size.Y);
	targetSize.SetValue(Size.Y);
	FocusComponent(usernameField);

	infoLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	infoLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	infoLabel->SetMultiline(true);
	infoLabel->Visible = false;
	AddComponent(infoLabel);

	AddComponent(loginButton);
	SetOkayButton(loginButton);
	loginButton->Appearance.HorizontalAlign = ui::Appearance::AlignRight;
	loginButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	loginButton->Appearance.TextInactive = style::Colour::ConfirmButton;
	loginButton->SetActionCallback({ [this] {
		c->Login(usernameField->GetText().ToUtf8(), passwordField->GetText().ToUtf8());
	} });
	AddComponent(cancelButton);
	cancelButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	cancelButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	cancelButton->SetActionCallback({ [this] {
		c->Logout();
	} });
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

void LoginView::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
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
	auto statusText = sender->GetStatusText();
	auto notWorking = sender->GetStatus() != loginWorking;
	auto userID = Client::Ref().GetAuthUser().UserID;
	if (!statusText.size() && !userID && notWorking)
	{
		statusText = String::Build("Don't have an account? {a:", SERVER, "/Register.html", "|\btRegister here\x0E}.");
	}
	infoLabel->Visible = statusText.size();
	infoLabel->SetText(statusText);
	infoLabel->AutoHeight();
	loginButton->Enabled = notWorking;
	cancelButton->Enabled = notWorking && userID;
	usernameField->Enabled = notWorking;
	passwordField->Enabled = notWorking;
	if (infoLabel->Visible)
	{
		targetSize.SetTarget(defaultSize.Y + infoLabel->Size.Y);
	}
	else
	{
		targetSize.SetTarget(defaultSize.Y);
	}
	if (sender->GetStatus() == loginSucceeded)
	{
		c->Exit();
	}
}

void LoginView::OnTick()
{
	c->Tick();
	Size.Y = targetSize.GetValue();
	loginButton->Position.Y = Size.Y-17;
	cancelButton->Position.Y = Size.Y-17;
}

void LoginView::OnDraw()
{
	Graphics * g = GetGraphics();
	g->DrawFilledRect(RectSized(Position - Vec2{ 1, 1 }, Size + Vec2{ 2, 2 }), 0x000000_rgb);
	g->DrawRect(RectSized(Position, Size), 0xFFFFFF_rgb);
}
