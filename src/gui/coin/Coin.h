#ifndef COIN_H
#define COIN_H

#include "gui/interface/Window.h"

class GameModel;
namespace ui
{
	class Label;
}
class Coin : public ui::Window
{
	GameModel *gameModel;
	ui::Label *powderCoins;
public:
	Coin(GameModel *gameModel);
	virtual void OnDraw();
	virtual void OnTick(float dt);
	virtual void OnTryExit(ExitMethod method);
};

#endif // COIN_H
