#ifndef CARDINPUT_H
#define CARDINPUT_H

#include "graphics/Pixel.h"
#include "graphics/Graphics.h"
#include "gui/interface/Window.h"

namespace ui
{
class Button;
class Textbox;
}
class CardInput : public ui::Window
{
	//pixel signature[50][250];
	VideoBuffer *signature;
	bool drawing = false;
	int step = 1;
	int purchaseChance;
	int amount;

	ui::Button *next;
	std::vector<ui::Textbox*> textboxChecks;
	bool signatureValid = false;

	void Next();
	void Step1();
	void Step2();
	void AttemptPurchase();
	void ClearComponents(int step);
public:
	CardInput(int chance, int amount);
	~CardInput();
	virtual void OnDraw();
	virtual void OnTick(float dt);

	virtual void OnMouseMove(int x, int y, int dx, int dy);
	virtual void OnMouseDown(int x, int y, unsigned button);
	virtual void OnMouseUp(int x, int y, unsigned button);

	virtual void OnTryExit(ExitMethod method);
};

#endif // CARDINPUT_H
