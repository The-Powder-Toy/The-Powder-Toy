#include "PreviewView.h"

#include "simulation/SaveRenderer.h"

#include "client/Client.h"
#include "client/SaveInfo.h"

#include "gui/dialogues/TextPrompt.h"
#include "gui/profile/ProfileActivity.h"
#include "gui/interface/ScrollPanel.h"
#include "gui/interface/AvatarButton.h"
#include "gui/preview/PreviewController.h"
#include "gui/preview/PreviewModel.h"
#include "gui/interface/Button.h"
#include "gui/interface/Keys.h"
#include "gui/interface/CopyTextButton.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/interface/Point.h"
#include "gui/interface/Window.h"
#include "gui/Style.h"

#include "common/tpt-rand.h"
#include "Comment.h"
#include "Format.h"
#include "Misc.h"

#ifdef GetUserName
# undef GetUserName // dammit windows
#endif

PreviewView::PreviewView():
	ui::Window(ui::Point(-1, -1), ui::Point((XRES/2)+210, (YRES/2)+150)),
	savePreview(NULL),
	submitCommentButton(NULL),
	addCommentBox(NULL),
	commentWarningLabel(NULL),
	userIsAuthor(false),
	doOpen(false),
	doError(false),
	doErrorMessage(""),
	showAvatars(true),
	prevPage(false),
	commentBoxHeight(20),
	commentHelpText(false)
{
	showAvatars = Client::Ref().GetPrefBool("ShowAvatars", true);

	favButton = new ui::Button(ui::Point(50, Size.Y-19), ui::Point(51, 19), "Fav");
	favButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	favButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	favButton->SetIcon(IconFavourite);
	favButton->SetActionCallback({ [this] { c->FavouriteSave(); } });
	favButton->Enabled = Client::Ref().GetAuthUser().UserID?true:false;
	AddComponent(favButton);

	reportButton = new ui::Button(ui::Point(100, Size.Y-19), ui::Point(51, 19), "Report");
	reportButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	reportButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	reportButton->SetIcon(IconReport);
	reportButton->SetActionCallback({ [this] {
		new TextPrompt("Report Save", "Things to consider when reporting:\n\bw1)\bg When reporting stolen saves, please include the ID of the original save.\n\bw2)\bg Do not ask for saves to be removed from front page unless they break the rules.\n\bw3)\bg You may report saves for comments or tags too (including your own saves)", "", "[reason]", true, { [this](String const &resultText) {
			c->Report(resultText);
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

	if(showAvatars)
		saveNameLabel = new ui::Label(ui::Point(39, (YRES/2)+4), ui::Point(100, 16), "");
	else
		saveNameLabel = new ui::Label(ui::Point(5, (YRES/2)+4), ui::Point(100, 16), "");
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
		authorDateLabel = new ui::Label(ui::Point(39, (YRES/2)+4+15), ui::Point(180, 16), "");
	else
		authorDateLabel = new ui::Label(ui::Point(5, (YRES/2)+4+15), ui::Point(200, 16), "");
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

	viewsLabel = new ui::Label(ui::Point((XRES/2)-80, (YRES/2)+4+15), ui::Point(80, 16), "");
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

	int textWidth = Graphics::textwidth("Click the box below to copy the save ID");
	saveIDLabel = new ui::Label(ui::Point((Size.X-textWidth-20)/2, Size.Y+5), ui::Point(textWidth+20, 16), "Click the box below to copy the save ID");
	saveIDLabel->SetTextColour(ui::Colour(150, 150, 150));
	saveIDLabel->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	AddComponent(saveIDLabel);

	textWidth = Graphics::textwidth(String::Build(c->SaveID()));
	saveIDLabel2 = new ui::Label(ui::Point((Size.X-textWidth-20)/2-37, Size.Y+22), ui::Point(40, 16), "Save ID:");
	AddComponent(saveIDLabel2);

	saveIDButton = new ui::CopyTextButton(ui::Point((Size.X-textWidth-10)/2, Size.Y+20), ui::Point(textWidth+10, 18), String::Build(c->SaveID()), saveIDLabel);
	AddComponent(saveIDButton);
}

void PreviewView::commentBoxAutoHeight()
{
	if(!addCommentBox)
		return;
	int textWidth = Graphics::textwidth(addCommentBox->GetText().c_str());
	if (commentHelpText || textWidth+15 > Size.X-(XRES/2)-48)
	{
		addCommentBox->Appearance.VerticalAlign = ui::Appearance::AlignTop;

		int oldSize = addCommentBox->Size.Y;
		addCommentBox->AutoHeight();
		int newSize = addCommentBox->Size.Y+2;
		addCommentBox->Size.Y = oldSize;

		commentBoxHeight = newSize+22;
		commentBoxPositionX = (XRES/2)+4;
		commentBoxPositionY = Size.Y-(newSize+21);
		commentBoxSizeX = Size.X-(XRES/2)-8;
		commentBoxSizeY = newSize;

		if (commentWarningLabel && commentHelpText && !commentWarningLabel->Visible && addCommentBox->Position.Y+addCommentBox->Size.Y < Size.Y-14)
		{
			commentWarningLabel->Visible = true;
		}
	}
	else
	{
		commentBoxHeight = 20;
		addCommentBox->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;

		commentBoxPositionX = (XRES/2)+4;
		commentBoxPositionY = Size.Y-19;
		commentBoxSizeX = Size.X-(XRES/2)-48;
		commentBoxSizeY = 17;

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
	if (!userIsAuthor && (text.Contains("stolen") || text.Contains("copied")))
	{
		if (!commentHelpText)
		{
			if (random_gen()%2)
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
			if (random_gen()%2)
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
	Window::DoDraw();
	Graphics * g = GetGraphics();
	for (size_t i = 0; i < commentTextComponents.size(); i++)
	{
		int linePos = commentTextComponents[i]->Position.Y+commentsPanel->ViewportPosition.Y+commentTextComponents[i]->Size.Y+4;
		if (linePos > 0 && linePos < Size.Y-commentBoxHeight)
		g->draw_line(
				Position.X+1+XRES/2,
				Position.Y+linePos,
				Position.X+Size.X-2,
				Position.Y+linePos,
				255, 255, 255, 100);
	}
	if (c->GetDoOpen())
	{
		g->fillrect(Position.X+(Size.X/2)-101, Position.Y+(Size.Y/2)-26, 202, 52, 0, 0, 0, 210);
		g->drawrect(Position.X+(Size.X/2)-100, Position.Y+(Size.Y/2)-25, 200, 50, 255, 255, 255, 180);
		g->drawtext(Position.X+(Size.X/2)-(Graphics::textwidth("Loading save...")/2), Position.Y+(Size.Y/2)-5, "Loading save...", style::Colour::InformationTitle.Red, style::Colour::InformationTitle.Green, style::Colour::InformationTitle.Blue, 255);
	}
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);

}

void PreviewView::OnDraw()
{
	Graphics * g = GetGraphics();

	//Window Background+Outline
	g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);

	//Save preview (top-left)
	if(savePreview && savePreview->Buffer)
	{
		g->draw_image(savePreview, (Position.X+1)+(((XRES/2)-savePreview->Width)/2), (Position.Y+1)+(((YRES/2)-savePreview->Height)/2), 255);
	}
	g->drawrect(Position.X, Position.Y, (XRES/2)+1, (YRES/2)+1, 255, 255, 255, 100);
	g->draw_line(Position.X+XRES/2, Position.Y+1, Position.X+XRES/2, Position.Y+Size.Y-2, 200, 200, 200, 255);

	if(votesUp || votesDown)
	{
		float ryf;
		int nyu, nyd;
		int lv = (votesUp>votesDown)?votesUp:votesDown;
		lv = (lv>10)?lv:10;

		if (50>lv)
		{
			ryf = 50.0f/((float)lv);
			nyu = votesUp*ryf;
			nyd = votesDown*ryf;
		}
		else
		{
			ryf = ((float)lv)/50.0f;
			nyu = votesUp/ryf;
			nyd = votesDown/ryf;
		}
		nyu = nyu>50?50:nyu;
		nyd = nyd>50?50:nyd;

		g->fillrect(Position.X+(XRES/2)-55, Position.Y+(YRES/2)+3, 53, 7, 0, 107, 10, 255);
		g->fillrect(Position.X+(XRES/2)-55, Position.Y+(YRES/2)+9, 53, 7, 107, 10, 0, 255);
		g->drawrect(Position.X+(XRES/2)-55, Position.Y+(YRES/2)+3, 53, 7, 128, 128, 128, 255);
		g->drawrect(Position.X+(XRES/2)-55, Position.Y+(YRES/2)+9, 53, 7, 128, 128, 128, 255);

		g->fillrect(Position.X+(XRES/2)-4-nyu, Position.Y+(YRES/2)+5, nyu, 3, 57, 187, 57, 255);
		g->fillrect(Position.X+(XRES/2)-4-nyd, Position.Y+(YRES/2)+11, nyd, 3, 187, 57, 57, 255);
	}
}

void PreviewView::OnTick(float dt)
{
	if(addCommentBox)
	{
		ui::Point positionDiff = ui::Point(commentBoxPositionX, commentBoxPositionY)-addCommentBox->Position;
		ui::Point sizeDiff = ui::Point(commentBoxSizeX, commentBoxSizeY)-addCommentBox->Size;

		if(positionDiff.X!=0)
		{
			int xdiff = positionDiff.X/5;
			if(xdiff == 0)
				xdiff = 1*isign(positionDiff.X);
			addCommentBox->Position.X += xdiff;
		}
		if(positionDiff.Y!=0)
		{
			int ydiff = positionDiff.Y/5;
			if(ydiff == 0)
				ydiff = 1*isign(positionDiff.Y);
			addCommentBox->Position.Y += ydiff;
		}

		if(sizeDiff.X!=0)
		{
			int xdiff = sizeDiff.X/5;
			if(xdiff == 0)
				xdiff = 1*isign(sizeDiff.X);
			addCommentBox->Size.X += xdiff;
			addCommentBox->Invalidate();
			commentBoxAutoHeight(); //make sure textbox height is correct after resizes
			addCommentBox->resetCursorPosition(); //make sure cursor is in correct position after resizes
		}
		if(sizeDiff.Y!=0)
		{
			int ydiff = sizeDiff.Y/5;
			if(ydiff == 0)
				ydiff = 1*isign(sizeDiff.Y);
			addCommentBox->Size.Y += ydiff;
			addCommentBox->Invalidate();
		}
		commentsPanel->Size.Y = addCommentBox->Position.Y-1;
	}

	c->Update();
	if (doError)
	{
		ErrorMessage::Blocking("Error loading save", doErrorMessage);
		c->Exit();
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

void PreviewView::NotifySaveChanged(PreviewModel * sender)
{
	SaveInfo * save = sender->GetSaveInfo();
	delete savePreview;
	savePreview = NULL;
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
			favButton->SetText("Unfav");
		}
		else if(Client::Ref().GetAuthUser().UserID)
		{
			favButton->Enabled = true;
			favButton->SetText("Fav");
		}
		else
		{
			favButton->SetText("Fav");
			favButton->Enabled = false;
		}

		if(save->GetGameSave())
		{
			savePreview = SaveRenderer::Ref().Render(save->GetGameSave(), false, true);

			if(savePreview && savePreview->Buffer && !(savePreview->Width == XRES/2 && savePreview->Height == YRES/2))
			{
				pixel * oldData = savePreview->Buffer;
				float factorX = ((float)XRES/2)/((float)savePreview->Width);
				float factorY = ((float)YRES/2)/((float)savePreview->Height);
				float scaleFactor = factorY < factorX ? factorY : factorX;
				savePreview->Buffer = Graphics::resample_img(oldData, savePreview->Width, savePreview->Height, savePreview->Width*scaleFactor, savePreview->Height*scaleFactor);
				delete[] oldData;
				savePreview->Width *= scaleFactor;
				savePreview->Height *= scaleFactor;
			}
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
		favButton->Enabled = false;
		if (!sender->GetCanOpen())
			openButton->Enabled = false;
	}
}

void PreviewView::submitComment()
{
	if(addCommentBox)
	{
		String comment = addCommentBox->GetText();
		submitCommentButton->Enabled = false;
		addCommentBox->SetText("");
		addCommentBox->SetPlaceholder("Submitting comment"); //This doesn't appear to ever show since no separate thread is created
		FocusComponent(NULL);

		if (!c->SubmitComment(comment))
			addCommentBox->SetText(comment);

		addCommentBox->SetPlaceholder("Add comment");
		submitCommentButton->Enabled = true;

		commentBoxAutoHeight();
	}
}

void PreviewView::NotifyCommentBoxEnabledChanged(PreviewModel * sender)
{
	if(addCommentBox)
	{
		RemoveComponent(addCommentBox);
		delete addCommentBox;
		addCommentBox = NULL;
	}
	if(submitCommentButton)
	{
		RemoveComponent(submitCommentButton);
		delete submitCommentButton;
		submitCommentButton = NULL;
	}
	if(sender->GetCommentBoxEnabled())
	{
		commentBoxPositionX = (XRES/2)+4;
		commentBoxPositionY = Size.Y-19;
		commentBoxSizeX = Size.X-(XRES/2)-48;
		commentBoxSizeY = 17;

		addCommentBox = new ui::Textbox(ui::Point((XRES/2)+4, Size.Y-19), ui::Point(Size.X-(XRES/2)-48, 17), "", "Add Comment");
		addCommentBox->SetActionCallback({ [this] {
			CheckComment();
			commentBoxAutoHeight();
		} });
		addCommentBox->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		addCommentBox->SetMultiline(true);
		AddComponent(addCommentBox);
		submitCommentButton = new ui::Button(ui::Point(Size.X-40, Size.Y-19), ui::Point(40, 19), "Submit");
		submitCommentButton->SetActionCallback({ [this] { submitComment(); } });
		//submitCommentButton->Enabled = false;
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
}

void PreviewView::NotifyCommentsPageChanged(PreviewModel * sender)
{
	pageInfo->SetText(String::Build("Page ", sender->GetCommentsPageNum(), " of ", sender->GetCommentsPageCount()));
}

void PreviewView::NotifyCommentsChanged(PreviewModel * sender)
{
	std::vector<SaveComment*> * comments = sender->GetComments();

	for (size_t i = 0; i < commentComponents.size(); i++)
	{
		commentsPanel->RemoveChild(commentComponents[i]);
		delete commentComponents[i];
	}
	commentComponents.clear();
	commentTextComponents.clear();
	commentsPanel->InnerSize = ui::Point(0, 0);

	if (comments)
	{
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
		for (size_t i = 0; i < comments->size(); i++)
		{
			if (showAvatars)
			{
				tempAvatar = new ui::AvatarButton(ui::Point(2, currentY+7), ui::Point(26, 26), comments->at(i)->authorName);
				tempAvatar->SetActionCallback({ [tempAvatar] {
					if (tempAvatar->GetUsername().size() > 0)
					{
						new ProfileActivity(tempAvatar->GetUsername());
					}
				} });
				commentComponents.push_back(tempAvatar);
				commentsPanel->AddChild(tempAvatar);
			}

			if (showAvatars)
				tempUsername = new ui::Label(ui::Point(31, currentY+3), ui::Point(Size.X-((XRES/2) + 13 + 26), 16), comments->at(i)->authorNameFormatted.FromUtf8());
			else
				tempUsername = new ui::Label(ui::Point(5, currentY+3), ui::Point(Size.X-((XRES/2) + 13), 16), comments->at(i)->authorNameFormatted.FromUtf8());
			tempUsername->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
			tempUsername->Appearance.VerticalAlign = ui::Appearance::AlignBottom;
			if (Client::Ref().GetAuthUser().UserID && Client::Ref().GetAuthUser().Username == comments->at(i)->authorName)
				tempUsername->SetTextColour(ui::Colour(255, 255, 100));
			else if (sender->GetSaveInfo() && sender->GetSaveInfo()->GetUserName() == comments->at(i)->authorName)
				tempUsername->SetTextColour(ui::Colour(255, 100, 100));
			currentY += 16;

			commentComponents.push_back(tempUsername);
			commentsPanel->AddChild(tempUsername);

			if (showAvatars)
				tempComment = new ui::Label(ui::Point(31, currentY+5), ui::Point(Size.X-((XRES/2) + 13 + 26), -1), comments->at(i)->comment);
			else
				tempComment = new ui::Label(ui::Point(5, currentY+5), ui::Point(Size.X-((XRES/2) + 13), -1), comments->at(i)->comment);
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
	delete savePreview;
}
