#pragma once
#include <memory>
#include <set>
#include <vector>
#include "common/String.h"
#include "gui/interface/Window.h"
#include "simulation/MissingElements.h"

namespace http
{
	class AddCommentRequest;
	class ReportSaveRequest;
}

namespace ui
{
	class Button;
	class CopyTextButton;
	class Label;
	class Textbox;
	class ScrollPanel;
	class AvatarButton;
}

class VideoBuffer;
class PreviewModel;
class PreviewController;
class PreviewView: public ui::Window
{
	PreviewController *c{};
	MissingElements missingElements;
	std::unique_ptr<VideoBuffer> savePreview;
	ui::Button *openButton{};
	ui::Button *browserOpenButton{};
	ui::Button *favButton{};
	ui::Button *reportButton{};
	ui::Button *submitCommentButton{};
	ui::Button *loadErrorButton{};
	ui::Button *missingElementsButton{};
	ui::Textbox *addCommentBox{};
	ui::Label *commentWarningLabel{};
	ui::Label *saveNameLabel{};
	ui::Label *authorDateLabel{};
	ui::AvatarButton *avatarButton{};
	ui::Label *pageInfo{};
	ui::Label *saveDescriptionLabel{};
	ui::Label *viewsLabel{};
	ui::Label *saveIDLabel{};
	ui::Label *saveIDLabel2{};
	ui::CopyTextButton *saveIDButton{};
	ui::ScrollPanel *commentsPanel{};
	std::vector<ui::Component*> commentComponents;
	std::vector<ui::Component*> commentTextComponents;
	int votesUp;
	int votesDown;
	bool userIsAuthor;
	bool doOpen;
	bool doError;
	String doErrorMessage;
	bool showAvatars;
	bool prevPage;
	bool isSubmittingComment = false;
	bool isRefreshingComments = false;

	int commentBoxHeight;
	float commentBoxPositionX;
	float commentBoxPositionY;
	float commentBoxSizeX;
	float commentBoxSizeY;
	bool commentHelpText;

	std::set<String> swearWords;

	void displayComments();
	void commentBoxAutoHeight();
	void submitComment();
	void CheckCommentSubmitEnabled();
	bool CheckSwearing(const String& text);
	void CheckComment();
	void ShowMissingCustomElements();
	void ShowLoadError();
	void UpdateLoadStatus();

	std::unique_ptr<http::AddCommentRequest> addCommentRequest;
	std::unique_ptr<http::ReportSaveRequest> reportSaveRequest;

public:
	void AttachController(PreviewController * controller);
	PreviewView(std::unique_ptr<VideoBuffer> newSavePreviev);
	void NotifySaveChanged(PreviewModel * sender);
	void NotifyCommentsChanged(PreviewModel * sender);
	void NotifyCommentsPageChanged(PreviewModel * sender);
	void NotifyCommentBoxEnabledChanged(PreviewModel * sender);
	void SaveLoadingError(String errorMessage);
	void OnDraw() override;
	void DoDraw() override;
	void OnTick(float dt) override;
	void OnTryExit(ExitMethod method) override;
	void OnMouseWheel(int x, int y, int d) override;
	void OnMouseUp(int x, int y, unsigned int button) override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	virtual ~PreviewView();
};
