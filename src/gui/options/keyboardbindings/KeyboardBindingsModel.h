#ifndef KEYBOARDBINDINGSMODEL_H
#define KEYBOARDBINDINGSMODEL_H

#include <vector>
#include <utility>
#include "common/String.h"

#define KEYBOARDBINDING_PREF "KeyboardBindings"
#define KEYBOARDBINDING_FUNCS_PREF "KeyboardBindingFunctions"

#define BINDING_MASK 0x07
#define BINDING_CTRL 0x01
#define BINDING_ALT 0x02
#define BINDING_SHIFT 0x04

struct BindingModel
{
	uint32_t modifier;
	uint32_t scan;
	int32_t functionId;
	String description;
	uint32_t index;
	bool isNew;
	bool noShortcut;

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

class KeyboardBindingsView;

class KeyboardBindingsModel
{
public:
	KeyboardBindingsModel(){}
	void WriteDefaultPrefs(bool force = false); // true if user clicks reset to defaults

	inline std::vector<BindingModel> GetBindingPrefs() const { return bindingPrefs; }
	void LoadBindingPrefs();
	void Save();
	void RemoveModelByIndex(uint32_t index);
	void AddModel(BindingModel model);
	void CreateModel(BindingModel model);
	String GetDisplayForModel(BindingModel model);
	void AddObserver(KeyboardBindingsView* observer);
	void NotifyBindingsChanged(bool hasConflict);
	bool HasConflictingCombo();
	void PopBindingByFunctionId(int32_t functionId);
	void WriteDefaultFuncArray(bool force = false);
	bool FunctionHasShortcut(int32_t functionId);
	int32_t GetFunctionForBinding(int scan, bool shift, bool ctrl, bool alt);

protected:
	void TurnOffFunctionShortcut(int32_t functionId);
	void TurnOnFunctionShortcut(int32_t functionId);

	std::vector<KeyboardBindingsView*> observers;
	std::vector<BindingModel> bindingPrefs;
	std::pair<uint32_t, uint32_t> GetModifierAndScanFromString(ByteString str);
};

#endif // KEYBOARDBINDINGSMODEL_H