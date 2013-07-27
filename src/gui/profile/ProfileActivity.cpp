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
			if(!a->loading && !a->saving && a->editable)
			{
				sender_->Enabled = false;
				sender_->SetText("Saving...");
				a->saving = true;
				a->info.Location = ((ui::Textbox*)a->location)->GetText();
				a->info.Biography = ((ui::Textbox*)a->bio)->GetText();
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

	if(!info.Biography.length() && !editable)
		info.Biography = "\bg(no bio)";

	if(!info.Location.length() && !editable)
		info.Location = "\bg(no location)";

	ui::AvatarButton * avatar = new ui::AvatarButton(ui::Point((Size.X-40)-8, 8), ui::Point(40, 40), info.Username);
	AddComponent(avatar);

	int currentY = 5;
	if(editable)
	{
		ui::Button * editAvatar = new ui::Button(ui::Point(Size.X - (40 + 16 + 75), currentY), ui::Point(75, 15), "Edit Avatar");
		editAvatar->SetActionCallback(new EditAvatarAction(this));
		AddComponent(editAvatar);
	}
	ui::Label * title = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X-8-(40+8+75), 15), info.Username);
	title->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(title);
	currentY += 20;


	ui::Label * locationTitle = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X-8-(40+8), 15), "Location");
	locationTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(locationTitle);
	currentY += 17;

	if(editable)
	{
		ui::Textbox * location = new ui::Textbox(ui::Point(8, currentY), ui::Point(Size.X-16-(40+8), 17), info.Location);
		location->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		AddComponent(location);
		this->location = location;
		currentY += 10+location->Size.Y;
	}
	else
	{
		ui::Label * location = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X-8-(40+8), 12), info.Location);
		location->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		location->SetTextColour(ui::Colour(180, 180, 180));
		AddComponent(location);
		this->location = location;
		currentY += 10+location->Size.Y;
	}
	
	ui::Label * bioTitle = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X-8, 15), "Biography");
	bioTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(bioTitle);
	currentY += 17;

	if(editable)
	{
		ui::Textbox * bio = new ui::Textbox(ui::Point(8, currentY), ui::Point(Size.X-16, Size.Y-30-currentY), info.Biography);
		bio->SetMultiline(true);
		bio->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		bio->Appearance.VerticalAlign = ui::Appearance::AlignTop;
		AddComponent(bio);
		currentY += 10+bio->Size.Y;
		this->bio = bio;
	}
	else
	{
		ui::Label * bio = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X-8, -1), info.Biography);
		bio->SetMultiline(true);
		bio->SetTextColour(ui::Colour(180, 180, 180));
		bio->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		currentY += 10+bio->Size.Y;
		if(currentY > Size.Y - 20)
		{
			ui::ScrollPanel * scrollPanel = new ui::ScrollPanel(ui::Point(1, bio->Position.Y), ui::Point(Size.X-2, Size.Y-30-bio->Position.Y));
			AddComponent(scrollPanel);
			bio->Position = ui::Point(4, 4);
			scrollPanel->AddChild(bio);
			scrollPanel->InnerSize = ui::Point(Size.X, bio->Size.Y+8);
		}
		else
		{
			AddComponent(bio);
		}
		this->bio = bio;
	}

	//exit(0);
}

void ProfileActivity::OnResponseReady(void * userDataPtr, int identifier)
{
	if(loading)
	{
		loading = false;
		setUserInfo(*(UserInfo*)userDataPtr);
		delete (UserInfo*)userDataPtr;
	}
	else if(saving)
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

ProfileActivity::~ProfileActivity() {
	RequestBroker::Ref().DetachRequestListener(this);
}

