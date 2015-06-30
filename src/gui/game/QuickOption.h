#pragma once

#include <string>
#include <vector>

class GameModel;
class QuickOption;
class QuickOptionListener
{
protected:
	QuickOptionListener() {}
public:
	virtual ~QuickOptionListener() {}
	virtual void OnValueChanged(QuickOption * sender) {}
};
class QuickOption
{
public:
	enum Type {
		Toggle, Multi
	};
protected:
	std::vector<QuickOptionListener*> listeners;
	GameModel * m;
	Type type;
	std::string icon;
	std::string description;
	QuickOption(std::string icon, std::string description, GameModel * m, Type type) :
		m(m),
		type(type),
		icon(icon),
		description(description)
	{

	}
	virtual void perform() {}
public:
	virtual ~QuickOption()
	{
		for(std::vector<QuickOptionListener*>::iterator iter = listeners.begin(), end = listeners.end(); iter != end; ++iter)
			delete *iter;
	}

	std::vector<QuickOptionListener*> GetListeners()
	{
		return listeners;
	}

	void AddListener(QuickOptionListener * listener)
	{
		listeners.push_back(listener);
	}

	Type GetType() { return type; }

	virtual bool GetToggle() { return true;}
	virtual int GetMutli() { return 0;}
	virtual int GetMultiCount() { return 0;}

	std::string GetIcon() { return icon; }
	void SetIcon(std::string icon) { this->icon = icon; }
	std::string GetDescription() { return description; }
	void SetDescription(std::string description) { this->description = description; }
	void Perform()
	{
		perform();
		for(std::vector<QuickOptionListener*>::iterator iter = listeners.begin(), end = listeners.end(); iter != end; ++iter)
			(*iter)->OnValueChanged(this);
	}
	void Update()
	{
		for(std::vector<QuickOptionListener*>::iterator iter = listeners.begin(), end = listeners.end(); iter != end; ++iter)
			(*iter)->OnValueChanged(this);
	}
};

