#include "QuickOption.h"
#include "GameModel.h"

class SandEffectOption: public QuickOption
{
public:
	SandEffectOption(GameModel * m):
	QuickOption("P", "Sand effect", m, Toggle)
	{

	}
	bool GetToggle() override
	{
		return m->GetSimulation()->pretty_powder;
	}
	void perform() override
	{
		m->GetSimulation()->pretty_powder = !m->GetSimulation()->pretty_powder;
	}
};

class DrawGravOption: public QuickOption
{
public:
	DrawGravOption(GameModel * m):
	QuickOption("G", "Draw gravity field \bg(ctrl+g)", m, Toggle)
	{

	}
	bool GetToggle() override
	{
		return m->GetGravityGrid();
	}
	void perform() override
	{
		m->ShowGravityGrid(!m->GetGravityGrid());
	}
};

class DecorationsOption: public QuickOption
{
public:
	DecorationsOption(GameModel * m):
	QuickOption("D", "Draw decorations \bg(ctrl+b)", m, Toggle)
	{

	}
	bool GetToggle() override
	{
		return m->GetDecoration();
	}
	void perform() override
	{
		m->SetDecoration(!m->GetDecoration());
	}
};

class NGravityOption: public QuickOption
{
public:
	NGravityOption(GameModel * m):
	QuickOption("N", "Newtonian Gravity \bg(n)", m, Toggle)
	{

	}
	bool GetToggle() override
	{
		return m->GetNewtonianGrvity();
	}
	void perform() override
	{
		m->SetNewtonianGravity(!m->GetNewtonianGrvity());
	}
};

class AHeatOption: public QuickOption
{
public:
	AHeatOption(GameModel * m):
	QuickOption("A", "Ambient heat \bg(u)", m, Toggle)
	{

	}
	bool GetToggle() override
	{
		return m->GetAHeatEnable();
	}
	void perform() override
	{
		m->SetAHeatEnable(!m->GetAHeatEnable());
	}
};

class ConsoleShowOption: public QuickOption
{
	GameController * c;
public:
	ConsoleShowOption(GameModel * m, GameController * c_):
	QuickOption("C", "Show Console \bg(~)", m, Toggle)
	{
		c = c_;
	}
	bool GetToggle() override
	{
		return 0;
	}
	void perform() override
	{
		c->ShowConsole();
	}
};
