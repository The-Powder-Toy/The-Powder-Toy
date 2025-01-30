#include "ProfileActivity.h"
#include "client/Client.h"
#include "client/http/SaveUserInfoRequest.h"
#include "client/http/GetUserInfoRequest.h"
#include "common/platform/Platform.h"
#include "gui/Style.h"
#include "gui/interface/AvatarButton.h"
#include "gui/interface/Button.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/interface/Label.h"
#include "gui/interface/ScrollPanel.h"
#include "gui/interface/Textbox.h"
#include "Config.h"

ProfileActivity::ProfileActivity(ByteString username) :
	WindowActivity(ui::Point(-1, -1), ui::Point(236, 300)),
	loading(false),
	saving(false),
	doError(false),
	doErrorMessage("")
{
	editable = Client::Ref().GetAuthUser().UserID && Client::Ref().GetAuthUser().Username == username;

	ui::Button * closeButton = new ui::Button(ui::Point(0, Size.Y-15), ui::Point(Size.X, 15), "Close");
	closeButton->SetActionCallback({ [this] {
		Exit();
	} });
	if(editable)
	{
		closeButton->Size.X = (Size.X/2)+1;

		ui::Button * saveButton = new ui::Button(ui::Point(Size.X/2, Size.Y-15), ui::Point(Size.X/2, 15), "Save");
		saveButton->SetActionCallback({ [this, saveButton] {
			if (!loading && !saving && editable)
			{
				saveButton->Enabled = false;
				saveButton->SetText("Saving...");
				saving = true;
				info.location = location->GetText();
				info.biography = bio->GetText();
				saveUserInfoRequest = std::make_unique<http::SaveUserInfoRequest>(info);
				saveUserInfoRequest->Start();
			}
		} });
		AddComponent(saveButton);
	}

	AddComponent(closeButton);

	loading = true;

	getUserInfoRequest = std::make_unique<http::GetUserInfoRequest>(username);
	getUserInfoRequest->Start();
}

void ProfileActivity::setUserInfo(UserInfo newInfo)
{
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
	ui::Label * title = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X-8-(40+16+75), 15), info.username.FromUtf8());
	title->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	scrollPanel->AddChild(title);

	// avatar
	ui::AvatarButton * avatar = new ui::AvatarButton(ui::Point((Size.X-40)-8, 5), ui::Point(40, 40), info.username);
	scrollPanel->AddChild(avatar);

	// edit avatar button
	if (editable)
	{
		ui::Button * editAvatar = new ui::Button(ui::Point(Size.X - (40 + 16 + 75), currentY), ui::Point(75, 15), "Edit Avatar");
		editAvatar->SetActionCallback({ [] {
			Platform::OpenURI(ByteString::Build(SERVER, "/Profile/Avatar.html"));
		} });
		scrollPanel->AddChild(editAvatar);
	}
	currentY += 23;

	// age
	ui::Label * ageTitle = new ui::Label(ui::Point(4, currentY), ui::Point(23, 15), "Age:");
	ageTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	ageTitle->SetTextColour(ui::Colour(180, 180, 180));
	scrollPanel->AddChild(ageTitle);

	// can't figure out how to tell a null from a 0 in the json library we use
	ui::Label *age = new ui::Label(ui::Point(5+ageTitle->Size.X, currentY), ui::Point(Size.X-ageTitle->Size.X-56, 15), info.age ? String::Build(info.age) : "\bgNot Provided");
	age->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	scrollPanel->AddChild(age);
	currentY += 2+age->Size.Y;

	// location
	ui::Label * locationTitle = new ui::Label(ui::Point(4, currentY), ui::Point(45, 15), "Location:");
	locationTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	locationTitle->SetTextColour(ui::Colour(180, 180, 180));
	scrollPanel->AddChild(locationTitle);

	if (editable)
	{
		location = new ui::Textbox(ui::Point(5+locationTitle->Size.X, currentY), ui::Point(Size.X-locationTitle->Size.X-16, 17), info.location);
		((ui::Textbox*)location)->SetLimit(40);
	}
	else
	{
		location = new ui::Label(ui::Point(5+locationTitle->Size.X, currentY), ui::Point(Size.X-locationTitle->Size.X-14, 17), info.location);
	}
	location->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	scrollPanel->AddChild(location);
	currentY += 2+location->Size.Y;

	// website
	ui::Label * websiteTitle = new ui::Label(ui::Point(1, currentY), ui::Point(42, 15), "Website:");
	websiteTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	websiteTitle->SetTextColour(ui::Colour(180, 180, 180));
	scrollPanel->AddChild(websiteTitle);

	ui::Label *website = new ui::Label(ui::Point(2+websiteTitle->Size.X, currentY), ui::Point(Size.X-websiteTitle->Size.X-16, 15), info.website.FromUtf8());
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
		ui::Label * saveCountTitle = new ui::Label(ui::Point(12, currentY), ui::Point(32, 15), "Count:");
		saveCountTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		saveCountTitle->SetTextColour(ui::Colour(180, 180, 180));
		scrollPanel->AddChild(saveCountTitle);

		ui::Label *savesCount = new ui::Label(ui::Point(13+saveCountTitle->Size.X, currentY), ui::Point(Size.X-saveCountTitle->Size.X-24, 15), String::Build(info.saveCount));
		savesCount->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		scrollPanel->AddChild(savesCount);
		currentY += savesCount->Size.Y;

		// average score
		ui::Label * averageScoreTitle = new ui::Label(ui::Point(12, currentY), ui::Point(72, 15), "Average Score:");
		averageScoreTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		averageScoreTitle->SetTextColour(ui::Colour(180, 180, 180));
		scrollPanel->AddChild(averageScoreTitle);

		ui::Label *averageScore = new ui::Label(ui::Point(13+averageScoreTitle->Size.X, currentY), ui::Point(Size.X-averageScoreTitle->Size.X-24, 15), String::Build(info.averageScore));
		averageScore->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		scrollPanel->AddChild(averageScore);
		currentY += averageScore->Size.Y;

		// highest score
		ui::Label * highestScoreTitle = new ui::Label(ui::Point(12, currentY), ui::Point(71, 15), "Highest Score:");
		highestScoreTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		highestScoreTitle->SetTextColour(ui::Colour(180, 180, 180));
		scrollPanel->AddChild(highestScoreTitle);

		ui::Label *highestScore = new ui::Label(ui::Point(13+highestScoreTitle->Size.X, currentY), ui::Point(Size.X-highestScoreTitle->Size.X-24, 15), String::Build(info.highestScore));
		highestScore->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		scrollPanel->AddChild(highestScore);
		currentY += 2+highestScore->Size.Y;

	// biograhy
	ui::Label * bioTitle = new ui::Label(ui::Point(4, currentY), ui::Point(50, 15), "Biography:");
	bioTitle->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	bioTitle->SetTextColour(ui::Colour(180, 180, 180));
	scrollPanel->AddChild(bioTitle);
	currentY += 17;

	if (editable)
	{
		bio = new ui::Textbox(ui::Point(4, currentY), ui::Point(Size.X-12, -1), info.biography);
		((ui::Textbox*)bio)->SetInputType(ui::Textbox::Multiline);
		((ui::Textbox*)bio)->SetActionCallback({ [this] { ResizeArea(); } });
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

void ProfileActivity::OnTick()
{
	if (doError)
	{
		new ErrorMessage("Error", doErrorMessage, { [this]() {
			Exit();
		} });
	}

	if (saveUserInfoRequest && saveUserInfoRequest->CheckDone())
	{
		try
		{
			saveUserInfoRequest->Finish();
			Exit();
		}
		catch (const http::RequestError &ex)
		{
			doError = true;
			doErrorMessage = "Could not save user info: " + ByteString(ex.what()).FromUtf8();
		}
		saveUserInfoRequest.reset();
	}
	if (getUserInfoRequest && getUserInfoRequest->CheckDone())
	{
		try
		{
			auto userInfo = getUserInfoRequest->Finish();
			loading = false;
			setUserInfo(userInfo);
		}
		catch (const http::RequestError &ex)
		{
			doError = true;
			doErrorMessage = "Could not load user info: " + ByteString(ex.what()).FromUtf8();
		}
		getUserInfoRequest.reset();
	}
}

void ProfileActivity::OnDraw()
{
	Graphics * g = GetGraphics();
	g->DrawFilledRect(RectSized(Position - Vec2{ 1, 1 }, Size + Vec2{ 2, 2 }), 0x000000_rgb);
	g->DrawRect(RectSized(Position, Size), 0xFFFFFF_rgb);
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
}

