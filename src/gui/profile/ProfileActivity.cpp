#include <algorithm>
#include "ProfileActivity.h"
#include "gui/interface/Button.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Label.h"
#include "gui/interface/AvatarButton.h"
#include "gui/interface/ScrollPanel.h"
#include "gui/interface/Keys.h"
#include "gui/Style.h"
#include "client/Client.h"
#include "client/UserInfo.h"
#include "client/requestbroker/RequestListener.h"

ProfileActivity::ProfileActivity(std::string username) :
	WindowActivity(ui::Point(-1, -1), ui::Point(236, 300)),
	loading(false),
	saving(false)
{
	editable = Client::Ref().GetAuthUser().ID && Client::Ref().GetAuthUser().Username == username;


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
			if (!a->loading && !a->saving && a->editable)
			{
				sender_->Enabled = false;
				sender_->SetText("Saving...");
				a->saving = true;
				a->info.location = ((ui::Textbox*)a->location)->GetText();
				a->info.biography = ((ui::Textbox*)a->bio)->GetText();
				RequestBroker::Ref().Start(Client::Ref().SaveUserInfoAsync(a->info), a);
			}
		}
	};


	ui::Button * closeButton = new ui::Button(ui::Point(0, Size.Y-15), ui::Point(Size.X, 15), "Close");
	closeButton->SetActionCallback(new CloseAction(this));
	if(editable)
	{
		closeButton->Size.X = (Size.X/2)+1;

		ui::Button * saveButton = new ui::Button(ui::Point(Size.X/2, Size.Y-15), ui::Point(Size.X/2, 15), "Save");
		saveButton->SetActionCallback(new SaveAction(this));
		AddComponent(saveButton);
	}

	AddComponent(closeButton);

	loading = true;
	RequestBroker::Ref().Start(Client::Ref().GetUserInfoAsync(username), this);
}

void ProfileActivity::setUserInfo(UserInfo newInfo)
{
	class EditAvatarAction: public ui::ButtonAction
	{
		ProfileActivity * a;
	public:
		EditAvatarAction(ProfileActivity * a) : a(a) {  }
		void ActionCallback(ui::Button * sender_)
		{
			OpenURI("http://" SERVER "/Profile/Avatar.html");
		}
	};

	info = newInfo;

	if (!info.biography.length() && !editable)
		info.biography = "\bgNot Provided";
	if (!info.location.length() && !editable)
		info.location = "\bgNot Provided";
	if (!info.website.length() && !editable)
		info.location = "\bgNot Provided";
	//if (!info.age.length() && !editable)
	//	info.age = "\bgNot Provided";


	ui::ScrollPanel * scrollPanel = new ui::ScrollPanel(ui::Point(1, 1), ui::Point(Size.X-2, Size.Y-16));
	AddComponent(scrollPanel);


	ui::AvatarButton * avatar = new ui::AvatarButton(ui::Point((Size.X-40)-8, 8), ui::Point(40, 40), info.username);
	scrollPanel->AddChild(avatar);

	int currentY = 5;
	if (editable)
	{
		ui::Button * editAvatar = new ui::Button(ui::Point(Size.X - (40 + 16 + 75), currentY), ui::Point(75, 15), "Edit Avatar");
		editAvatar->SetActionCallback(new EditAvatarAction(this));
		scrollPanel->AddChild(editAvatar);
	}
	ui::Label * title = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X-8-(40+8+75), 15), info.username);
	title->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	scrollPanel->AddChild(title);
	currentY += 20;


	ui::Label * locationTitle = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X-8-(40+8), 15), "Location");
	locationTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	scrollPanel->AddChild(locationTitle);
	currentY += 17;

	if (editable)
	{
		location = new ui::Textbox(ui::Point(4, currentY), ui::Point(Size.X-8-(40+8), 17), info.location);
	}
	else
	{
		location = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X-8-(40+8), 12), info.location);
		location->SetTextColour(ui::Colour(180, 180, 180));
	}
	location->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	scrollPanel->AddChild(location);
	this->location = location;
	currentY += 10+location->Size.Y;
	
	ui::Label * bioTitle = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X-8, 15), "Biography");
	bioTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	scrollPanel->AddChild(bioTitle);
	currentY += 17;

	if (editable)
	{
		bio = new ui::Textbox(ui::Point(8, currentY), ui::Point(Size.X-16, -1), info.biography);
	}
	else
	{
		bio = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X-8, -1), info.biography);
		bio->SetTextColour(ui::Colour(180, 180, 180));
	}
	bio->SetMultiline(true);
	bio->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	bio->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	bio->AutoHeight();
	scrollPanel->AddChild(bio);
	currentY += 10+bio->Size.Y;

	scrollPanel->InnerSize = ui::Point(Size.X, currentY);
}

void ProfileActivity::OnResponseReady(void * userDataPtr, int identifier)
{
	if (loading)
	{
		loading = false;
		setUserInfo(*(UserInfo*)userDataPtr);
		delete (UserInfo*)userDataPtr;
	}
	else if (saving)
	{
		Exit();
	}
}

void ProfileActivity::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
}

void ProfileActivity::OnTryExit(ExitMethod method)
{
	Exit();
}

ProfileActivity::~ProfileActivity()
{
	RequestBroker::Ref().DetachRequestListener(this);
}

