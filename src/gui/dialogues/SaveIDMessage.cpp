#include "SaveIDMessage.h"
#include "gui/Style.h"
#include "graphics/Graphics.h"
#include "gui/interface/Button.h"
#include "gui/interface/CopyTextButton.h"
#include "gui/interface/Label.h"
#include "Format.h"
#include "SimulationConfig.h"

SaveIDMessage::SaveIDMessage(int id):
	ui::Window(ui::Point((XRES-244)/2, (YRES-90)/2), ui::Point(244, 90))
{
	int textWidth = Graphics::TextSize("Save ID").X - 1;
	ui::Label * titleLabel = new ui::Label(ui::Point(4, 5), ui::Point(textWidth+20, 16), "Save ID");
	titleLabel->SetTextColour(style::Colour::InformationTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);

	textWidth = Graphics::TextSize("Saved Successfully!").X - 1;
	ui::Label * messageLabel = new ui::Label(ui::Point(4, 24), ui::Point(textWidth+20, 16), "Saved Successfully!");
	messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	AddComponent(messageLabel);

	textWidth = Graphics::TextSize("Click the box below to copy the save ID").X - 1;
	ui::Label * copyTextLabel = new ui::Label(ui::Point((Size.X-textWidth-20)/2, 35), ui::Point(textWidth+20, 16), "Click the box below to copy the save id");
	copyTextLabel->SetTextColour(ui::Colour(150, 150, 150));
	copyTextLabel->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	AddComponent(copyTextLabel);

	textWidth = Graphics::TextSize(String::Build(id)).X - 1;
	ui::CopyTextButton * copyTextButton = new ui::CopyTextButton(ui::Point((Size.X-textWidth-10)/2, 50), ui::Point(textWidth+10, 18), String::Build(id), copyTextLabel);
	AddComponent(copyTextButton);

	ui::Button * okayButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "OK");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->SetActionCallback({ [this] {
		CloseActiveWindow();
		SelfDestruct();
	} });
	AddComponent(okayButton);
	// This button has multiple personalities
	SetOkayButton(okayButton);
	SetCancelButton(okayButton);

	MakeActiveWindow();
}

void SaveIDMessage::OnDraw()
{
	Graphics * g = GetGraphics();

	g->DrawFilledRect(RectSized(Position - Vec2{ 1, 1 }, Size + Vec2{ 2, 2 }), 0x000000_rgb);
	g->DrawRect(RectSized(Position, Size), 0xC8C8C8_rgb);
}

void SaveIDMessage::OnTryExit(ExitMethod method)
{
	CloseActiveWindow();
	SelfDestruct();
}
