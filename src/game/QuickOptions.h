#include "QuickOption.h"
#include "GameModel.h"

class SandEffectOption: public QuickOption
{
public:
	SandEffectOption(GameModel * m):
	QuickOption("P", "Sand effect", m, Toggle)
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
	QuickOption("G", "Draw gravity field", m, Toggle)
	{

	}
	virtual bool GetToggle() 
	{
		return m->GetRenderer()->gravityFieldEnabled;
	}
	virtual void perform()
	{
		m->GetRenderer()->gravityFieldEnabled = !m->GetRenderer()->gravityFieldEnabled;
	}
};

class DecorationsOption: public QuickOption
{
public:
	DecorationsOption(GameModel * m):
	QuickOption("D", "Draw decorations", m, Toggle)
	{

	}
	virtual bool GetToggle() 
	{
		return m->GetRenderer()->decorations_enable;
	}
	virtual void perform()
	{
		m->GetRenderer()->decorations_enable = !m->GetRenderer()->decorations_enable;
	}
};

class NGravityOption: public QuickOption
{
public:
	NGravityOption(GameModel * m):
	QuickOption("N", "Newtonian Gravity", m, Toggle)
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
		}
		else
		{
			m->GetSimulation()->grav->start_grav_async();
		}
	}
};

class AHeatOption: public QuickOption
{
public:
	AHeatOption(GameModel * m):
	QuickOption("A", "Ambient heat", m, Toggle)
	{

	}
	virtual bool GetToggle() 
	{
		return m->GetSimulation()->aheat_enable;
	}
	virtual void perform()
	{
		m->GetSimulation()->aheat_enable = !m->GetSimulation()->aheat_enable;
	}
};

class DrawGravWallOption: public QuickOption
{
public:
	DrawGravWallOption(GameModel * m):
	QuickOption("Z", "Draw gravity zones", m, Toggle)
	{

	}
	virtual bool GetToggle() 
	{
		return m->GetRenderer()->gravityZonesEnabled;
	}
	virtual void perform()
	{
		m->GetRenderer()->gravityZonesEnabled = !m->GetRenderer()->gravityZonesEnabled;
	}
};