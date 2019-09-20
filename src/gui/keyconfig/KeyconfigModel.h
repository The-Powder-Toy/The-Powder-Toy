#ifndef KEYCONFIGSMODEL_H
#define KEYCONFIGSMODEL_H

#include <vector>
#include <utility>
#include "common/String.h"

#define KEYCONFIG_PREF "Keyconfig"
#define KEYCONFIG_FUNCS_PREF "KeyconfigFunctions"

#define BINDING_MASK 0x07
#define BINDING_CTRL 0x01
#define BINDING_ALT 0x02
#define BINDING_SHIFT 0x04

struct BindingModel
{
	int modifier;
	int scan;
	int functionId;
	String description;
	int index;
	bool noShortcut;
	bool isNew;

	BindingModel() : noShortcut(false), isNew(false){};

	bool operator==(const BindingModel& other) const
	{
		return modifier == other.modifier && scan == other.scan;
	}
	
	bool operator< (const BindingModel &other) const
	{
		if (description == other.description)
			return index < other.index;

		return description < other.description;
	}
};

class KeyconfigView;

class KeyconfigModel
{
public:
	KeyconfigModel(){}
	void WriteDefaultPrefs(bool force = false); // true if user clicks reset to defaults

	inline std::vector<BindingModel> GetBindingPrefs() const { return bindingPrefs; }
	void LoadBindingPrefs();
	void Save();
	void RemoveModelByIndex(int index);
	void AddModel(BindingModel model);
	void CreateModel(BindingModel model);
	String GetDisplayForModel(BindingModel model);
	void AddObserver(KeyconfigView* observer);
	void NotifyBindingsChanged(bool hasConflict);
	bool HasConflictingCombo();
	void PopBindingByFunctionId(int functionId);
	void WriteDefaultFuncArray(bool force = false);
	bool FunctionHasShortcut(int functionId);
	int GetFunctionForBinding(int scan, bool shift, bool ctrl, bool alt);

protected:
	void TurnOffFunctionShortcut(int functionId);
	void TurnOnFunctionShortcut(int functionId);

	std::vector<KeyconfigView*> observers;
	std::vector<BindingModel> bindingPrefs;
	std::pair<int, int> GetModifierAndScanFromString(ByteString str);
};

#endif // KEYCONFIGSMODEL_H
