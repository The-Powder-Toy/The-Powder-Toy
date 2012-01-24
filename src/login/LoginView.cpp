/*
 * LoginView.cpp
 *
 *  Created on: Jan 24, 2012
 *      Author: Simon
 */

#include "LoginView.h"

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

LoginView::LoginView():
	ui::Window(ui::Point(-1, -1), ui::Point(200, 100)),
	loginButton(new ui::Button(ui::Point(200-50, 100-16), ui::Point(50, 16), "Login")),
	cancelButton(new ui::Button(ui::Point(0, 100-16), ui::Point(50, 16), "Cancel")),
	titleLabel(new ui::Label(ui::Point(4, 2), ui::Point(200-16, 16), "Server login")),
	usernameField(new ui::Textbox(ui::Point(8, 20), ui::Point(200-16, 16), "")),
	passwordField(new ui::Textbox(ui::Point(8, 40), ui::Point(200-16, 16), "")),
	infoLabel(new ui::Label(ui::Point(8, 60), ui::Point(200-16, 16), ""))
{
	AddComponent(loginButton);
	loginButton->SetAlignment(AlignCentre, AlignBottom);
	loginButton->SetActionCallback(new LoginAction(this));
	AddComponent(cancelButton);
	cancelButton->SetAlignment(AlignCentre, AlignBottom);
	AddComponent(titleLabel);
	titleLabel->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(usernameField);
	usernameField->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(passwordField);
	passwordField->SetAlignment(AlignLeft, AlignBottom);
	passwordField->SetHidden(true);
	infoLabel->SetAlignment(AlignCentre, AlignBottom);
	AddComponent(infoLabel);
}

void LoginView::NotifyStatusChanged(LoginModel * sender)
{
	infoLabel->SetText(sender->GetStatusText());
}

void LoginView::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
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

