#include "PreviewView.h"

#include "simulation/SaveRenderer.h"

#include "client/Client.h"
#include "client/SaveInfo.h"
#include "client/GameSave.h"
#include "client/http/AddCommentRequest.h"
#include "client/http/ReportSaveRequest.h"

#include "gui/dialogues/TextPrompt.h"
#include "gui/profile/ProfileActivity.h"
#include "gui/interface/ScrollPanel.h"
#include "gui/interface/AvatarButton.h"
#include "gui/preview/PreviewController.h"
#include "gui/preview/PreviewModel.h"
#include "gui/interface/Button.h"
#include "gui/interface/CopyTextButton.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Engine.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/interface/Point.h"
#include "gui/interface/Window.h"
#include "gui/Style.h"

#include "common/tpt-rand.h"
#include "common/platform/Platform.h"
#include "Format.h"
#include "Misc.h"

#include "graphics/VideoBuffer.h"
#include "SimulationConfig.h"
#include <SDL.h>

PreviewView::PreviewView(std::unique_ptr<VideoBuffer> newSavePreview):
	ui::Window(ui::Point(-1, -1), ui::Point((XRES/2)+210, (YRES/2)+150)),
	submitCommentButton(nullptr),
	addCommentBox(nullptr),
	commentWarningLabel(nullptr),
	userIsAuthor(false),
	doOpen(false),
	doError(false),
	doErrorMessage(""),
	showAvatars(true),
	prevPage(false),
	commentBoxHeight(20),
	commentHelpText(false)
{
	if (newSavePreview)
	{
		newSavePreview->Resize(RES / 2, true);
		savePreview = std::move(newSavePreview);
	}
	showAvatars = ui::Engine::Ref().ShowAvatars;

	favButton = new ui::Button(ui::Point(50, Size.Y-19), ui::Point(51, 19), "Fav");
	favButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	favButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	favButton->SetTogglable(true);
	favButton->SetIcon(IconFavourite);
	favButton->SetActionCallback({ [this] {
		favButton->SetToggleState(true);
		favButton->Appearance.BackgroundPulse = true;
		c->FavouriteSave();
	} });
	favButton->Enabled = Client::Ref().GetAuthUser().UserID?true:false;
	AddComponent(favButton);

	reportButton = new ui::Button(ui::Point(100, Size.Y-19), ui::Point(51, 19), "Report");
	reportButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	reportButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	reportButton->SetIcon(IconReport);
	reportButton->SetActionCallback({ [this] {
		new TextPrompt("Report Save", "Things to consider when reporting:\n\bw1)\bg When reporting stolen saves, please include the ID of the original save.\n\bw2)\bg Do not ask for saves to be removed from front page unless they break the rules.\n\bw3)\bg You may report saves for comments or tags too (including your own saves)", "", "[reason]", true, { [this](String const &resultText) {
			if (reportSaveRequest)
			{
				return;
			}
			reportSaveRequest = std::make_unique<http::ReportSaveRequest>(c->SaveID(), resultText);
			reportSaveRequest->Start();
		} });
	} });
	reportButton->Enabled = Client::Ref().GetAuthUser().UserID?true:false;
	AddComponent(reportButton);

	openButton = new ui::Button(ui::Point(0, Size.Y-19), ui::Point(51, 19), "Open");
	openButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	openButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	openButton->SetIcon(IconOpen);
	openButton->SetActionCallback({ [this] { c->DoOpen(); } });
	AddComponent(openButton);

	browserOpenButton = new ui::Button(ui::Point((XRES/2)-107, Size.Y-19), ui::Point(108, 19), "Open in browser");
	browserOpenButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	browserOpenButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	browserOpenButton->SetIcon(IconOpen);
	browserOpenButton->SetActionCallback({ [this] { c->OpenInBrowser(); } });
	AddComponent(browserOpenButton);

	loadErrorButton = new ui::Button({ 0, 0 }, ui::Point(148, 19), "Error loading save");
	loadErrorButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	loadErrorButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	loadErrorButton->SetIcon(IconDelete);
	loadErrorButton->SetActionCallback({ [this] { ShowLoadError(); } });
	loadErrorButton->Visible = false;
	AddComponent(loadErrorButton);

	missingElementsButton = new ui::Button({ 0, 0 }, ui::Point(148, 19), "Missing custom elements");
	missingElementsButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	missingElementsButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	missingElementsButton->SetIcon(IconReport);
	missingElementsButton->SetActionCallback({ [this] { ShowMissingCustomElements(); } });
	missingElementsButton->Visible = false;
	AddComponent(missingElementsButton);

	if(showAvatars)
		saveNameLabel = new ui::Label(ui::Point(39, (YRES/2)+4), ui::Point(265, 16), "");
	else
		saveNameLabel = new ui::Label(ui::Point(5, (YRES/2)+4), ui::Point(300, 16), "");
	saveNameLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	saveNameLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(saveNameLabel);

	if(showAvatars)
		saveDescriptionLabel = new ui::Label(ui::Point(5, (YRES/2)+4+15+21), ui::Point((XRES/2)-10, Size.Y-((YRES/2)+4+15+17)-25), "");
	else
		saveDescriptionLabel = new ui::Label(ui::Point(5, (YRES/2)+4+15+19), ui::Point((XRES/2)-10, Size.Y-((YRES/2)+4+15+17)-23), "");
	saveDescriptionLabel->SetMultiline(true);
	saveDescriptionLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	saveDescriptionLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	saveDescriptionLabel->SetTextColour(ui::Colour(180, 180, 180));
	AddComponent(saveDescriptionLabel);

	if(showAvatars)
		authorDateLabel = new ui::Label(ui::Point(39, (YRES/2)+4+15), ui::Point(200, 16), "");
	else
		authorDateLabel = new ui::Label(ui::Point(5, (YRES/2)+4+15), ui::Point(220, 16), "");
	authorDateLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	authorDateLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(authorDateLabel);

	if(showAvatars)
	{
		avatarButton = new ui::AvatarButton(ui::Point(4, (YRES/2)+4), ui::Point(34, 34), "");
		avatarButton->SetActionCallback({ [this] {
			if (avatarButton->GetUsername().size() > 0)
			{
				new ProfileActivity(avatarButton->GetUsername());
			}
		} });
		AddComponent(avatarButton);
	}

	viewsLabel = new ui::Label(ui::Point((XRES/2)-88, (YRES/2)+4+15), ui::Point(88, 16), "");
	viewsLabel->Appearance.HorizontalAlign = ui::Appearance::AlignRight;
	viewsLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(viewsLabel);

	pageInfo = new ui::Label(ui::Point((XRES/2) + 85, Size.Y+1), ui::Point(70, 16), "Page 1 of 1");
	pageInfo->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	AddComponent(pageInfo);

	commentsPanel = new ui::ScrollPanel(ui::Point((XRES/2)+1, 1), ui::Point((Size.X-(XRES/2))-2, Size.Y-commentBoxHeight));
	AddComponent(commentsPanel);

	swearWords.insert("fuck");
	swearWords.insert("bitch");
	swearWords.insert("shit ");
	swearWords.insert("asshole");
	swearWords.insert("dick");
	swearWords.insert("cunt");
	swearWords.insert(" nigger");
	swearWords.insert("faggot");
	swearWords.insert("dumbass");
}

void PreviewView::AttachController(PreviewController * controller)
{
	c = controller;

	int textWidth = Graphics::TextSize("Click the box below to copy the save ID").X - 1;
	saveIDLabel = new ui::Label(ui::Point((Size.X-textWidth-20)/2, Size.Y+5), ui::Point(textWidth+20, 16), "Click the box below to copy the save ID");
	saveIDLabel->SetTextColour(ui::Colour(150, 150, 150));
	saveIDLabel->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	AddComponent(saveIDLabel);

	textWidth = Graphics::TextSize(String::Build(c->SaveID())).X - 1;
	saveIDLabel2 = new ui::Label(ui::Point((Size.X-textWidth-20)/2-37, Size.Y+22), ui::Point(40, 16), "Save ID:");
	AddComponent(saveIDLabel2);

	saveIDButton = new ui::CopyTextButton(ui::Point((Size.X-textWidth-10)/2, Size.Y+20), ui::Point(textWidth+10, 18), String::Build(c->SaveID()), saveIDLabel);
	AddComponent(saveIDButton);
}

void PreviewView::commentBoxAutoHeight()
{
	if(!addCommentBox)
		return;
	int textWidth = Graphics::TextSize(addCommentBox->GetText().c_str()).X - 1;
	if (commentHelpText || textWidth+15 > Size.X-(XRES/2)-48)
	{
		addCommentBox->Appearance.VerticalAlign = ui::Appearance::AlignTop;

		int oldSize = addCommentBox->Size.Y;
		addCommentBox->AutoHeight();
		int newSize = addCommentBox->Size.Y+2;
		addCommentBox->Size.Y = oldSize;

		commentBoxHeight = newSize+22;
		commentBoxPositionX.SetTarget((XRES/2)+4);
		commentBoxPositionY.SetTarget(float(Size.Y-(newSize+21)));
		commentBoxSizeX.SetTarget(float(Size.X-(XRES/2)-8));
		commentBoxSizeY.SetTarget(float(newSize));

		if (commentWarningLabel && commentHelpText && !commentWarningLabel->Visible && addCommentBox->Position.Y+addCommentBox->Size.Y < Size.Y-14)
		{
			commentWarningLabel->Visible = true;
		}
	}
	else
	{
		commentBoxHeight = 20;
		addCommentBox->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;

		commentBoxPositionX.SetTarget((XRES/2)+4);
		commentBoxPositionY.SetTarget(float(Size.Y-19));
		commentBoxSizeX.SetTarget(float(Size.X-(XRES/2)-48));
		commentBoxSizeY.SetTarget(17);

		if (commentWarningLabel && commentWarningLabel->Visible)
		{
			commentWarningLabel->Visible = false;
		}
	}
}

bool PreviewView::CheckSwearing(String text)
{
	for (std::set<String>::iterator iter = swearWords.begin(), end = swearWords.end(); iter != end; iter++)
		if (text.Contains(*iter))
			return true;
	return false;
}

void PreviewView::CheckComment()
{
	if (!commentWarningLabel)
		return;
	String text = addCommentBox->GetText().ToLower();
	if (addCommentRequest)
	{
		commentWarningLabel->SetText("Submitting comment...");
		commentHelpText = true;
	}
	else if (!userIsAuthor && (text.Contains("stolen") || text.Contains("copied")))
	{
		if (!commentHelpText)
		{
			if (interfaceRng()%2)
				commentWarningLabel->SetText("Stolen? Report the save instead");
			else
				commentWarningLabel->SetText("Please report stolen saves");
			commentHelpText = true;
		}
	}
	else if (userIsAuthor && text.Contains("vote"))
	{
		commentWarningLabel->SetText("Do not ask for votes");
		commentHelpText = true;
	}
	else if (CheckSwearing(text))
	{
		if (!commentHelpText)
		{
			if (interfaceRng()%2)
				commentWarningLabel->SetText("Please do not swear");
			else
				commentWarningLabel->SetText("Bad language may be deleted");
			commentHelpText = true;
		}
	}
	else
	{
		commentHelpText = false;
		commentWarningLabel->Visible = false;
	}
}

void PreviewView::DoDraw()
{
	Graphics * g = GetGraphics();
	if (!c->GetFromUrl())
	{
		Window::DoDraw();
		for (size_t i = 0; i < commentTextComponents.size(); i++)
		{
			int linePos = commentTextComponents[i]->Position.Y+commentsPanel->ViewportPosition.Y+commentTextComponents[i]->Size.Y+4;
			if (linePos > 0 && linePos < Size.Y-commentBoxHeight)
			g->BlendLine(
					Position + Vec2{ 1+XRES/2, linePos },
					Position + Vec2{ Size.X-2, linePos },
					0xFFFFFF_rgb .WithAlpha(100));
		}
	}
	if (c->GetDoOpen())
	{
		g->BlendFilledRect(RectSized(Position + Size / 2 - Vec2{ 101, 26 }, { 202, 52 }), 0x000000_rgb .WithAlpha(210));
		g->BlendRect(RectSized(Position + Size / 2 - Vec2{ 100, 25 }, Vec2{ 200, 50 }), 0xFFFFFF_rgb .WithAlpha(180));
		g->BlendText(Position + Vec2{(Size.X/2)-((Graphics::TextSize("Loading save...").X - 1)/2), (Size.Y/2)-5}, "Loading save...", style::Colour::InformationTitle.NoAlpha().WithAlpha(255));
	}
	if (!c->GetFromUrl())
	{
		g->DrawRect(RectSized(Position, Size), 0xFFFFFF_rgb);
	}
}

void PreviewView::OnDraw()
{
	Graphics * g = GetGraphics();

	//Window Background+Outline
	g->DrawFilledRect(RectSized(Position - Vec2{ 1, 1 }, Size + Vec2{ 2, 2 }), 0x000000_rgb);

	//Save preview (top-left)
	if (savePreview)
	{
		g->BlendImage(savePreview->Data(), 0xFF, RectSized(Position + Vec2(1, 1) + (RES / 2 - savePreview->Size()) / 2, savePreview->Size()));
	}
	g->BlendRect(RectSized(Position, RES / 2 + Vec2{ 1, 1 }), 0xFFFFFF_rgb .WithAlpha(100));
	g->DrawLine(Position + Vec2{ XRES/2, 1 }, Position + Vec2{ XRES/2, Size.Y-2 }, 0xC8C8C8_rgb);

	if(votesUp || votesDown)
	{
		float ryf;
		int nyu, nyd;
		int lv = (votesUp>votesDown)?votesUp:votesDown;
		lv = (lv>10)?lv:10;

		if (50>lv)
		{
			ryf = 50.0f/((float)lv);
			nyu = int(votesUp*ryf);
			nyd = int(votesDown*ryf);
		}
		else
		{
			ryf = ((float)lv)/50.0f;
			nyu = int(votesUp/ryf);
			nyd = int(votesDown/ryf);
		}
		nyu = nyu>50?50:nyu;
		nyd = nyd>50?50:nyd;

		g->DrawFilledRect(RectSized(Position + RES / 2 + Vec2{ -56, 3 }, Vec2{ 54, 7 }), 0x006B0A_rgb);
		g->DrawFilledRect(RectSized(Position + RES / 2 + Vec2{ -56, 9 }, Vec2{ 54, 7 }), 0x6B0A00_rgb);
		g->DrawRect(RectSized(Position + Vec2{ (XRES/2)-56, (YRES/2)+3 }, { 54, 7 }), 0x808080_rgb);
		g->DrawRect(RectSized(Position + Vec2{ (XRES/2)-56, (YRES/2)+9 }, { 54, 7 }), 0x808080_rgb);

		g->DrawFilledRect(RectSized(Position + RES / 2 + Vec2{ -4-nyu, 5 }, Vec2{ nyu, 3 }), 0x39BB39_rgb);
		g->DrawFilledRect(RectSized(Position + RES / 2 + Vec2{ -4-nyd, 11 }, Vec2{ nyd, 3 }), 0xBB3939_rgb);
	}
}

void PreviewView::OnTick()
{
	if(addCommentBox)
	{
		addCommentBox->Position.X = commentBoxPositionX;
		addCommentBox->Position.Y = commentBoxPositionY;

		if(addCommentBox->Size.X != commentBoxSizeX)
		{
			addCommentBox->Size.X = commentBoxSizeX;
			addCommentBox->Invalidate();
			commentBoxAutoHeight(); //make sure textbox height is correct after resizes
			addCommentBox->resetCursorPosition(); //make sure cursor is in correct position after resizes
		}
		if(addCommentBox->Size.Y != commentBoxSizeY)
		{
			addCommentBox->Size.Y = commentBoxSizeY;
			addCommentBox->Invalidate();
		}
		commentsPanel->Size.Y = addCommentBox->Position.Y-1;
	}

	c->Update();
	if (doError)
	{
		openButton->Enabled = false;
		loadErrorButton->Visible = true;
		UpdateLoadStatus();
	}

	if (reportSaveRequest && reportSaveRequest->CheckDone())
	{
		try
		{
			reportSaveRequest->Finish();
			c->Exit();
			new InformationMessage("Information", "Report submitted", false);
		}
		catch (const http::RequestError &ex)
		{
			new ErrorMessage("Error", "Unable to file report: " + ByteString(ex.what()).FromUtf8());
		}
		reportSaveRequest.reset();
	}
	if (addCommentRequest && addCommentRequest->CheckDone())
	{
		try
		{
			addCommentRequest->Finish();
			addCommentBox->SetText("");
			c->CommentAdded();
		}
		catch (const http::RequestError &ex)
		{
			new ErrorMessage("Error submitting comment", ByteString(ex.what()).FromUtf8());
		}
		isSubmittingComment = false;
		CheckCommentSubmitEnabled();
		commentBoxAutoHeight();
		addCommentRequest.reset();
		CheckComment();
	}
}

void PreviewView::OnTryExit(ExitMethod method)
{
	c->Exit();
}

void PreviewView::OnMouseWheel(int x, int y, int d)
{
	if(commentsPanel->GetScrollLimit() == 1 && d < 0)
		c->NextCommentPage();
	if(commentsPanel->GetScrollLimit() == -1 && d > 0)
	{
		if (c->PrevCommentPage())
			prevPage = true;
	}

}

void PreviewView::OnMouseUp(int x, int y, unsigned int button)
{
	//if mouse is on the scrollbar or farther right, and you are at the top of bottom of a page, change pages
	if (x > Position.X+commentsPanel->Position.X+commentsPanel->Size.X-6)
	{
		if (commentsPanel->GetScrollLimit() == 1)
			c->NextCommentPage();
		if (commentsPanel->GetScrollLimit() == -1)
		{
			if (c->PrevCommentPage())
				prevPage = true;
		}
	}
}

void PreviewView::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
	if ((key == SDLK_KP_ENTER || key == SDLK_RETURN) && (!addCommentBox || !addCommentBox->IsFocused()))
		openButton->DoAction();
}

void PreviewView::ShowLoadError()
{
	new ErrorMessage("Error loading save", doErrorMessage, {});
}

void PreviewView::ShowMissingCustomElements()
{
	StringBuilder sb;
	sb << "This save uses custom elements that are not currently available. Make sure that you use the mod and/or have all the scripts the save requires to fully load.";
	auto remainingIds = missingElements.ids;
	if (missingElements.identifiers.size())
	{
		sb << "\n\nA list of identifiers of missing custom elements follows, which may help you determine how to fix this problem.\n";
		for (auto &[ identifier, id ] : missingElements.identifiers)
		{
			sb << "\n - " << identifier.FromUtf8();
			remainingIds.erase(id); // remove ids from the missing id set that are already covered by unknown identifiers
		}
	}
	if (remainingIds.size())
	{
		sb << "\n\nA list of element IDs of missing custom elements with no identifier associated follows. This can only be fixed by the author of the save.\n";
		for (auto id : remainingIds)
		{
			sb << "\n - " << id;
		}
	}
	new InformationMessage("Missing custom elements", sb.Build(), true);
}

void PreviewView::UpdateLoadStatus()
{
	auto y = YRES / 2 - 22;
	auto showButton = [&y](ui::Button *button) {
		if (button->Visible)
		{
			button->Position = { XRES / 2 - button->Size.X - 3, y };
			y -= button->Size.Y + 3;
		}
	};
	showButton(missingElementsButton);
	showButton(loadErrorButton);
}

void PreviewView::NotifySaveChanged(PreviewModel * sender)
{
	favButton->Appearance.BackgroundPulse = false;
	auto *save = sender->GetSaveInfo();
	if(save)
	{
		votesUp = save->votesUp;
		votesDown = save->votesDown;
		saveNameLabel->SetText(save->name);
		String dateType;
		if (save->updatedDate == save->createdDate)
			dateType = "Created:";
		else
			dateType = "Updated:";
		if (showAvatars)
		{
			avatarButton->SetUsername(save->userName);
			authorDateLabel->SetText("\bw" + save->userName.FromUtf8() + " \bg" + dateType + " \bw" + format::UnixtimeToDateMini(save->updatedDate).FromAscii());
		}
		else
		{
			authorDateLabel->SetText("\bgAuthor:\bw " + save->userName.FromUtf8() + " \bg" + dateType + " \bw" + format::UnixtimeToDateMini(save->updatedDate).FromAscii());
		}
		if (Client::Ref().GetAuthUser().UserID && save->userName == Client::Ref().GetAuthUser().Username)
			userIsAuthor = true;
		else
			userIsAuthor = false;
		viewsLabel->SetText(String::Build("\bgViews:\bw ", save->Views));
		saveDescriptionLabel->SetText(save->Description);
		if(save->Favourite)
		{
			favButton->Enabled = true;
			favButton->SetToggleState(true);
		}
		else if(Client::Ref().GetAuthUser().UserID)
		{
			favButton->Enabled = true;
			favButton->SetToggleState(false);
		}
		else
		{
			favButton->SetToggleState(false);
			favButton->Enabled = false;
		}

		if(save->GetGameSave())
		{
			missingElements = save->GetGameSave()->missingElements;
			RendererSettings rendererSettings;
			rendererSettings.decorationLevel = RendererSettings::decorationAntiClickbait;
			savePreview = SaveRenderer::Ref().Render(save->GetGameSave(), true, rendererSettings);
			if (savePreview)
				savePreview->ResizeToFit(RES / 2, true);
			missingElementsButton->Visible = missingElements;
			UpdateLoadStatus();
		}
		else if (!sender->GetCanOpen())
			openButton->Enabled = false;
	}
	else
	{
		votesUp = 0;
		votesDown = 0;
		saveNameLabel->SetText("");
		authorDateLabel->SetText("");
		saveDescriptionLabel->SetText("");
		favButton->SetToggleState(false);
		favButton->Enabled = false;
		if (!sender->GetCanOpen())
			openButton->Enabled = false;
	}
}

void PreviewView::submitComment()
{
	if (addCommentBox)
	{
		String comment = addCommentBox->GetText();
		if (comment.length() == 0)
		{
			c->RefreshComments();
			isRefreshingComments = true;
		}
		else if (comment.length() < 4)
		{
			new ErrorMessage("Error", "Comment is too short");
		}
		else
		{
			isSubmittingComment = true;
			FocusComponent(nullptr);

			addCommentRequest = std::make_unique<http::AddCommentRequest>(c->SaveID(), comment);
			addCommentRequest->Start();

			CheckComment();
		}

		CheckCommentSubmitEnabled();
	}
}

void PreviewView::CheckCommentSubmitEnabled()
{
	if (submitCommentButton)
		submitCommentButton->Enabled = !isRefreshingComments && !isSubmittingComment;
}

void PreviewView::NotifyCommentBoxEnabledChanged(PreviewModel * sender)
{
	if(addCommentBox)
	{
		RemoveComponent(addCommentBox);
		delete addCommentBox;
		addCommentBox = nullptr;
	}
	if(submitCommentButton)
	{
		RemoveComponent(submitCommentButton);
		delete submitCommentButton;
		submitCommentButton = nullptr;
	}
	if(sender->GetCommentBoxEnabled())
	{
		addCommentBox = new ui::Textbox(ui::Point((XRES/2)+4, Size.Y-19), ui::Point(Size.X-(XRES/2)-48, 17), "", "Add Comment");
		commentBoxPositionX.SetTarget(addCommentBox->Position.X);
		commentBoxPositionX.SetValue(addCommentBox->Position.X);
		commentBoxPositionY.SetTarget(addCommentBox->Position.Y);
		commentBoxPositionY.SetValue(addCommentBox->Position.Y);
		commentBoxSizeX.SetTarget(addCommentBox->Size.X);
		commentBoxSizeX.SetValue(addCommentBox->Size.X);
		commentBoxSizeY.SetTarget(addCommentBox->Size.Y);
		commentBoxSizeY.SetValue(addCommentBox->Size.Y);
		addCommentBox->SetActionCallback({ [this] {
			CheckComment();
			commentBoxAutoHeight();
		} });
		addCommentBox->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		addCommentBox->SetMultiline(true);
		addCommentBox->SetLimit(1000);
		AddComponent(addCommentBox);
		submitCommentButton = new ui::Button(ui::Point(Size.X-40, Size.Y-19), ui::Point(40, 19), "Submit");
		submitCommentButton->SetActionCallback({ [this] { submitComment(); } });
		AddComponent(submitCommentButton);

		commentWarningLabel = new ui::Label(ui::Point((XRES/2)+4, Size.Y-19), ui::Point(Size.X-(XRES/2)-48, 16), "If you see this it is a bug");
		commentWarningLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		commentWarningLabel->SetTextColour(ui::Colour(255, 0, 0));
		commentWarningLabel->Visible = false;
		AddComponent(commentWarningLabel);
	}
	else
	{
		submitCommentButton = new ui::Button(ui::Point(XRES/2, Size.Y-19), ui::Point(Size.X-(XRES/2), 19), "Login to comment");
		submitCommentButton->SetActionCallback({ [this] { c->ShowLogin(); } });
		AddComponent(submitCommentButton);
	}
}

void PreviewView::SaveLoadingError(String errorMessage)
{
	doError = true;
	doErrorMessage = errorMessage;
	Platform::MarkPresentable();
}

void PreviewView::NotifyCommentsPageChanged(PreviewModel * sender)
{
	pageInfo->SetText(String::Build("Page ", sender->GetCommentsPageNum(), " of ", sender->GetCommentsPageCount()));
}

void PreviewView::NotifyCommentsChanged(PreviewModel * sender)
{
	auto commentsPtr = sender->GetComments();

	for (size_t i = 0; i < commentComponents.size(); i++)
	{
		commentsPanel->RemoveChild(commentComponents[i]);
		delete commentComponents[i];
	}
	commentComponents.clear();
	commentTextComponents.clear();
	commentsPanel->InnerSize = ui::Point(0, 0);

	isRefreshingComments = false;
	CheckCommentSubmitEnabled();

	if (commentsPtr)
	{
		auto &comments = *commentsPtr;
		for (size_t i = 0; i < commentComponents.size(); i++)
		{
			commentsPanel->RemoveChild(commentComponents[i]);
			delete commentComponents[i];
		}
		commentComponents.clear();
		commentTextComponents.clear();

		int currentY = 0;//-yOffset;
		ui::Label * tempUsername;
		ui::Label * tempComment;
		ui::AvatarButton * tempAvatar;
		for (size_t i = 0; i < comments.size(); i++)
		{
			if (showAvatars)
			{
				tempAvatar = new ui::AvatarButton(ui::Point(2, currentY+7), ui::Point(26, 26), comments[i].authorName);
				tempAvatar->SetActionCallback({ [tempAvatar] {
					if (tempAvatar->GetUsername().size() > 0)
					{
						new ProfileActivity(tempAvatar->GetUsername());
					}
				} });
				commentComponents.push_back(tempAvatar);
				commentsPanel->AddChild(tempAvatar);
			}

			auto authorNameFormatted = comments[i].authorName.FromUtf8();
			if (comments[i].authorElevation != User::ElevationNone || comments[i].authorName == "jacobot")
			{
				authorNameFormatted = "\bt" + authorNameFormatted;
			}
			else if (comments[i].authorIsBanned)
			{
				authorNameFormatted = "\bg" + authorNameFormatted;
			}
			else if (Client::Ref().GetAuthUser().UserID && Client::Ref().GetAuthUser().Username == comments[i].authorName)
			{
				authorNameFormatted = "\bo" + authorNameFormatted;
			}
			else if (sender->GetSaveInfo() && sender->GetSaveInfo()->GetUserName() == comments[i].authorName)
			{
				authorNameFormatted = "\bl" + authorNameFormatted;
			}
			if (showAvatars)
				tempUsername = new ui::Label(ui::Point(31, currentY+8), ui::Point(Size.X-((XRES/2) + 13 + 26), 14), authorNameFormatted);
			else
				tempUsername = new ui::Label(ui::Point(5, currentY+8), ui::Point(Size.X-((XRES/2) + 13), 14), authorNameFormatted);
			tempUsername->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
			tempUsername->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
			currentY += 16;

			commentComponents.push_back(tempUsername);
			commentsPanel->AddChild(tempUsername);

			if (showAvatars)
				tempComment = new ui::Label(ui::Point(31, currentY+5), ui::Point(Size.X-((XRES/2) + 13 + 26), -1), comments[i].content);
			else
				tempComment = new ui::Label(ui::Point(5, currentY+5), ui::Point(Size.X-((XRES/2) + 13), -1), comments[i].content);
			tempComment->SetMultiline(true);
			tempComment->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
			tempComment->Appearance.VerticalAlign = ui::Appearance::AlignTop;
			tempComment->SetTextColour(ui::Colour(180, 180, 180));
			currentY += tempComment->Size.Y+4;

			commentComponents.push_back(tempComment);
			commentsPanel->AddChild(tempComment);
			commentTextComponents.push_back(tempComment);
		}

		commentsPanel->InnerSize = ui::Point(commentsPanel->Size.X, currentY+4);
		if (prevPage)
		{
			prevPage = false;
			commentsPanel->SetScrollPosition(currentY);
		}
	}
	//else if (sender->GetCommentsLoaded())
	//	ErrorMessage::Blocking("Error loading comments", Client::Ref().GetLastError());
}

PreviewView::~PreviewView()
{
	if(addCommentBox)
	{
		RemoveComponent(addCommentBox);
		delete addCommentBox;
	}
	if(submitCommentButton)
	{
		RemoveComponent(submitCommentButton);
		delete submitCommentButton;
	}
}
