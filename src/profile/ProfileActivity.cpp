#include <algorithm>
#include "ProfileActivity.h"
#include "interface/Button.h"
#include "interface/Textbox.h"
#include "interface/Label.h"
#include "interface/Keys.h"
#include "Style.h"
#include "client/Client.h"
#include "client/requestbroker/RequestListener.h"

ProfileActivity::ProfileActivity(std::string username) :
	WindowActivity(ui::Point(-1, -1), ui::Point(236, 302))
{
	bool editable = Client::Ref().GetAuthUser().ID && Client::Ref().GetAuthUser().Username == username;


	class CloseAction: public ui::ButtonAction
	{
		ProfileActivity * a;
	public:
		CloseAction(ProfileActivity * a) : a(a) {  }
		void ActionCallback(ui::Button * sender_)
		{
			a->Exit();
		}
	};

	class SaveAction: public ui::ButtonAction
	{
		ProfileActivity * a;
	public:
		SaveAction(ProfileActivity * a) : a(a) {  }
		void ActionCallback(ui::Button * sender_)
		{
		}
	};

	ui::Button * closeButton = new ui::Button(ui::Point(0, Size.Y-15), ui::Point((Size.X/2)+1, 15), "Close");
	closeButton->SetActionCallback(new CloseAction(this));

	if(editable)
	{
		ui::Button * saveButton = new ui::Button(ui::Point(Size.X/2, Size.Y-15), ui::Point(Size.X/2, 15), "Save");
		saveButton->SetActionCallback(new SaveAction(this));
		AddComponent(saveButton);
	}

	AddComponent(closeButton);

	RequestBroker::Ref().Start(Client::Ref().GetUserInfoAsync(username), this);
}

void ProfileActivity::OnResponseReady(void * userDataPtr)
{
	exit(0);
}

void ProfileActivity::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
}

ProfileActivity::~ProfileActivity() {
	RequestBroker::Ref().DetachRequestListener(this);
}

