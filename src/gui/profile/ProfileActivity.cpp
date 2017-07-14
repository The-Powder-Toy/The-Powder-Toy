#include <algorithm>
#include "ProfileActivity.h"
#include "gui/interface/Button.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Label.h"
#include "gui/interface/AvatarButton.h"
#include "gui/interface/ScrollPanel.h"
#include "gui/interface/Keys.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/Style.h"
#include "client/Client.h"
#include "client/UserInfo.h"
#include "client/requestbroker/RequestListener.h"
#include "Format.h"
#include "Platform.h"

ProfileActivity::ProfileActivity(std::string username) :
	WindowActivity(ui::Point(-1, -1), ui::Point(236, 300)),
	loading(false),
	saving(false),
	doError(false),
	doErrorMessage("")
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
	public:
		void ActionCallback(ui::Button * sender_)
		{
			Platform::OpenURI("http://" SERVER "/Profile/Avatar.html");
		}
	};

	info = newInfo;

	if (!info.biography.length() && !editable)
		info.biography = "\bgNot Provided";
	if (!info.location.length() && !editable)
		info.location = "\bgNot Provided";
	if (!info.website.length())
		info.website = "\bgNot Provided";

	// everything is on a large scroll panel
	scrollPanel = new ui::ScrollPanel(ui::Point(1, 1), ui::Point(Size.X-2, Size.Y-16));
	AddComponent(scrollPanel);
	int currentY = 5;

	// username label
	ui::Label * title = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X-8-(40+8+75), 15), info.username);
	title->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	scrollPanel->AddChild(title);

	// avatar
	ui::AvatarButton * avatar = new ui::AvatarButton(ui::Point((Size.X-40)-8, 5), ui::Point(40, 40), info.username);
	scrollPanel->AddChild(avatar);

	// edit avatar button
	if (editable)
	{
		ui::Button * editAvatar = new ui::Button(ui::Point(Size.X - (40 + 16 + 75), currentY), ui::Point(75, 15), "Edit Avatar");
		editAvatar->SetActionCallback(new EditAvatarAction());
		scrollPanel->AddChild(editAvatar);
	}
	currentY += 23;

	// age
	ui::Label * ageTitle = new ui::Label(ui::Point(4, currentY), ui::Point(18, 15), "Age:");
	ageTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	ageTitle->SetTextColour(ui::Colour(180, 180, 180));
	scrollPanel->AddChild(ageTitle);

	// can't figure out how to tell a null from a 0 in the json library we use
	ui::Label *age = new ui::Label(ui::Point(8+ageTitle->Size.X, currentY), ui::Point(40, 15), info.age ? format::NumberToString<int>(info.age) : "\bgNot Provided");
	age->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	scrollPanel->AddChild(age);
	currentY += 2+age->Size.Y;

	// location
	ui::Label * locationTitle = new ui::Label(ui::Point(4, currentY), ui::Point(45, 15), "Location:");
	locationTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	locationTitle->SetTextColour(ui::Colour(180, 180, 180));
	scrollPanel->AddChild(locationTitle);

	if (editable)
		location = new ui::Textbox(ui::Point(8+locationTitle->Size.X, currentY), ui::Point(Size.X-locationTitle->Size.X-16, 17), info.location);
	else
		location = new ui::Label(ui::Point(4+locationTitle->Size.X, currentY), ui::Point(Size.X-locationTitle->Size.X-14, 17), info.location);
	location->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	scrollPanel->AddChild(location);
	currentY += 2+location->Size.Y;

	// website
	ui::Label * websiteTitle = new ui::Label(ui::Point(4, currentY), ui::Point(38, 15), "Website:");
	websiteTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	websiteTitle->SetTextColour(ui::Colour(180, 180, 180));
	scrollPanel->AddChild(websiteTitle);

	ui::Label *website = new ui::Label(ui::Point(8+websiteTitle->Size.X, currentY), ui::Point(Size.X-websiteTitle->Size.X-16, 15), info.website);
	website->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	scrollPanel->AddChild(website);
	currentY += 2+website->Size.Y;

	// saves
	ui::Label * savesTitle = new ui::Label(ui::Point(4, currentY), ui::Point(35, 15), "Saves:");
	savesTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	savesTitle->SetTextColour(ui::Colour(180, 180, 180));
	scrollPanel->AddChild(savesTitle);
	currentY += savesTitle->Size.Y;

		// saves count
		ui::Label * saveCountTitle = new ui::Label(ui::Point(12, currentY), ui::Point(30, 15), "Count:");
		saveCountTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		saveCountTitle->SetTextColour(ui::Colour(180, 180, 180));
		scrollPanel->AddChild(saveCountTitle);

		ui::Label *savesCount = new ui::Label(ui::Point(12+saveCountTitle->Size.X, currentY), ui::Point(Size.X-saveCountTitle->Size.X-16, 15), format::NumberToString<int>(info.saveCount));
		savesCount->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		scrollPanel->AddChild(savesCount);
		currentY += savesCount->Size.Y;

		// average score
		ui::Label * averageScoreTitle = new ui::Label(ui::Point(12, currentY), ui::Point(70, 15), "Average Score:");
		averageScoreTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		averageScoreTitle->SetTextColour(ui::Colour(180, 180, 180));
		scrollPanel->AddChild(averageScoreTitle);

		ui::Label *averageScore = new ui::Label(ui::Point(12+averageScoreTitle->Size.X, currentY), ui::Point(Size.X-averageScoreTitle->Size.X-16, 15), format::NumberToString<float>(info.averageScore));
		averageScore->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		scrollPanel->AddChild(averageScore);
		currentY += averageScore->Size.Y;

		// highest score
		ui::Label * highestScoreTitle = new ui::Label(ui::Point(12, currentY), ui::Point(69, 15), "Highest Score:");
		highestScoreTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		highestScoreTitle->SetTextColour(ui::Colour(180, 180, 180));
		scrollPanel->AddChild(highestScoreTitle);

		ui::Label *highestScore = new ui::Label(ui::Point(12+highestScoreTitle->Size.X, currentY), ui::Point(Size.X-highestScoreTitle->Size.X-16, 15), format::NumberToString<int>(info.highestScore));
		highestScore->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		scrollPanel->AddChild(highestScore);
		currentY += 2+highestScore->Size.Y;
	
	// biograhy
	ui::Label * bioTitle = new ui::Label(ui::Point(4, currentY), ui::Point(50, 15), "Biography:");
	bioTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	bioTitle->SetTextColour(ui::Colour(180, 180, 180));
	scrollPanel->AddChild(bioTitle);
	currentY += 17;

	class BioChangedAction: public ui::TextboxAction
	{
	public:
		ProfileActivity * profileActivity;
		BioChangedAction(ProfileActivity * profileActivity_) { profileActivity = profileActivity_; }
		virtual void TextChangedCallback(ui::Textbox * sender)
		{
			profileActivity->ResizeArea();
		}
	};

	if (editable)
	{
		bio = new ui::Textbox(ui::Point(4, currentY), ui::Point(Size.X-12, -1), info.biography);
		((ui::Textbox*)bio)->SetInputType(ui::Textbox::Multiline);
		((ui::Textbox*)bio)->SetActionCallback(new BioChangedAction(this));
		((ui::Textbox*)bio)->SetLimit(20000);
	}
	else
		bio = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X-12, -1), info.biography);
	bio->SetMultiline(true);
	bio->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	bio->Appearance.VerticalAlign = ui::Appearance::AlignTop;
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

void ProfileActivity::OnResponseFailed(int identifier)
{
	doError = true;
	if (loading)
		doErrorMessage = "Could not load user info: " + Client::Ref().GetLastError();
	else if (saving)
		doErrorMessage = "Could not save user info: " + Client::Ref().GetLastError();
}

void ProfileActivity::OnTick(float dt)
{
	if (doError)
	{
		ErrorMessage::Blocking("Error", doErrorMessage);
		Exit();
	}
}

void ProfileActivity::OnDraw()
{
	Graphics * g = GetGraphics();
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
}

void ProfileActivity::OnTryExit(ExitMethod method)
{
	Exit();
}

void ProfileActivity::ResizeArea()
{
	int oldSize = scrollPanel->InnerSize.Y;
	scrollPanel->InnerSize = ui::Point(Size.X, bio->Position.Y + bio->Size.Y + 10);
	// auto scroll as ScrollPanel size increases
	if (oldSize+scrollPanel->ViewportPosition.Y == scrollPanel->Size.Y)
		scrollPanel->SetScrollPosition(scrollPanel->InnerSize.Y-scrollPanel->Size.Y);
}

ProfileActivity::~ProfileActivity()
{
	RequestBroker::Ref().DetachRequestListener(this);
}

