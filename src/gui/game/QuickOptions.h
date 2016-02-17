#include "QuickOption.h"
#include "GameModel.h"
#include "Lang.h"

class SandEffectOption: public QuickOption
{
public:
	SandEffectOption(GameModel * m):
	QuickOption("P", TEXT_QUICK_OPT_SAND_DESC, m, Toggle)
	{

	}
	virtual bool GetToggle() 
	{
		return m->GetSimulation()->pretty_powder;
	}
	virtual void perform()
	{
		m->GetSimulation()->pretty_powder = !m->GetSimulation()->pretty_powder;
	}
};

class DrawGravOption: public QuickOption
{
public:
	DrawGravOption(GameModel * m):
	QuickOption("G", TEXT_QUICK_OPT_GGRID_DESC, m, Toggle)
	{

	}
	virtual bool GetToggle() 
	{
		return m->GetGravityGrid();
	}
	virtual void perform()
	{
		m->ShowGravityGrid(!m->GetGravityGrid());
	}
};

class DecorationsOption: public QuickOption
{
public:
	DecorationsOption(GameModel * m):
	QuickOption("D", TEXT_QUICK_OPT_DECO_DESC, m, Toggle)
	{

	}
	virtual bool GetToggle() 
	{
		return m->GetDecoration();
	}
	virtual void perform()
	{
		m->SetDecoration(!m->GetDecoration());
	}
};

class NGravityOption: public QuickOption
{
public:
	NGravityOption(GameModel * m):
	QuickOption("N", TEXT_QUICK_OPT_NEWTON_DESC, m, Toggle)
	{

	}
	virtual bool GetToggle() 
	{
		return m->GetSimulation()->grav->ngrav_enable;
	}
	virtual void perform()
	{
		if(m->GetSimulation()->grav->ngrav_enable)
		{
			m->GetSimulation()->grav->stop_grav_async();
			m->SetInfoTip(TEXT_INFOTIP_NEWTON_OFF);
		}
		else
		{
			m->GetSimulation()->grav->start_grav_async();
			m->SetInfoTip(TEXT_INFOTIP_NEWTON_ON);
		}
	}
};

class AHeatOption: public QuickOption
{
public:
	AHeatOption(GameModel * m):
	QuickOption("A", TEXT_QUICK_OPT_AHEAT_DESC, m, Toggle)
	{

	}
	virtual bool GetToggle() 
	{
		return m->GetAHeatEnable();
	}
	virtual void perform()
	{
		m->SetAHeatEnable(!m->GetAHeatEnable());
	}
};

class ConsoleShowOption: public QuickOption
{
	GameController * c;
public:
	ConsoleShowOption(GameModel * m, GameController * c_):
	QuickOption("C", TEXT_QUICK_OPT_CON_DESC, m, Toggle)
	{
		c = c_;
	}
	virtual bool GetToggle()
	{
		return 0;
	}
	virtual void perform()
	{
		c->ShowConsole();
	}
};
