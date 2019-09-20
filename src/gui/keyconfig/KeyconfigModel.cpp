#include "KeyconfigModel.h"
#include "client/Client.h"
#include "SDLCompat.h"
#include "KeyconfigMap.h"
#include <algorithm>
#include "KeyconfigView.h"

void KeyconfigModel::WriteDefaultFuncArray(bool force)
{
	if (force)
		Client::Ref().ClearPref(ByteString(KEYCONFIG_FUNCS_PREF));

	for (auto defaultBinding : defaultKeyconfigMapArray)
	{
		int functionId;
		String description;
		for (auto functions : keyboardBindingFunctionMap)
		{
			if (functions.id == defaultBinding.bindingId)
			{
				functionId = functions.functionId;
				description = functions.description;
			}
		}

		ByteString pref = ByteString(KEYCONFIG_FUNCS_PREF) + ByteString(".") + ByteString(functionId);
		bool functionExists = Client::Ref().GetPrefJson(pref, Json::nullValue) != Json::nullValue;

		if (!force && functionExists)
			continue;

		Json::Value prefValue;

		prefValue["hasShortcut"] = true;
		prefValue["functionId"] = functionId;
		Client::Ref().SetPref(pref, prefValue);
	}

}

void KeyconfigModel::WriteDefaultPrefs(bool force)
{
	// Load temporary bindings into memory
	// this is so we can add in any new axctions
	// from the KeyconfigMap into our prefs
	LoadBindingPrefs();

	if (force)
		Client::Ref().ClearPref(ByteString(KEYCONFIG_PREF));

	WriteDefaultFuncArray(force);

	for (auto defaultBinding : defaultKeyconfigMapArray)
	{
		int functionId;
		String description;
		for (auto functions : keyboardBindingFunctionMap)
		{
			if (functions.id == defaultBinding.bindingId)
			{
				functionId = functions.functionId;
				description = functions.description;
			}
		}

		ByteString pref = ByteString(KEYCONFIG_PREF) + ByteString(".") + defaultBinding.keyCombo;
		Json::Value prefValue;
		
		// if we not forcing then check if the function is already set up as a pref
		// if it is then bail the current iteration
		if (!force)
		{
			if (bindingPrefs.size() > 0)
			{
				for (auto prefBinding : bindingPrefs)
				{
					if (prefBinding.functionId == functionId)
						goto end; // evil but necessary
				}
			}
		}

		prefValue["description"] = description.ToUtf8();
		prefValue["functionId"] = functionId;
		Client::Ref().SetPref(pref, prefValue);

		end:;
	}

	// force is from a user action so don't write into store
	// until user hits OK
	if (!force)
		Client::Ref().WritePrefs();

	LoadBindingPrefs();
}

void KeyconfigModel::LoadBindingPrefs()
{
	Json::Value bindings = Client::Ref().GetPrefJson(KEYCONFIG_PREF);
	bindingPrefs.clear();

	if (bindings != Json::nullValue)
	{
		Json::Value::Members keyComboJson = bindings.getMemberNames();
		int index = 0;

		for (auto& member : keyComboJson)
		{
			ByteString keyCombo(member);
			ByteString pref = ByteString(KEYCONFIG_PREF) + "." + keyCombo;
			Json::Value result = Client::Ref().GetPrefJson(pref);

			if (result != Json::nullValue)
			{
				BindingModel model;
				std::pair<int, int> p = GetModifierAndScanFromString(keyCombo);
				model.modifier = p.first;
				model.scan = p.second;
				model.functionId = result["functionId"].asInt();
				model.description = ByteString(result["description"].asString()).FromUtf8();
				model.index = index;
				bindingPrefs.push_back(model);
			}

			index++;
		}
	}
}

std::pair<int, int> 
KeyconfigModel::GetModifierAndScanFromString(ByteString str)
{
	int modifier = 0;
	int scan = 0;

	if (str == "NULL")
	{
		scan = -1;
	}
	else
	{
		ByteString::Split split = str.SplitBy("+");

		// not the last int so its a modifier
		ByteString modString = split.Before();

		modifier |= (std::stoi(modString) & BINDING_MASK);
		scan = std::stoi(split.After());
	}

	return std::make_pair(modifier, scan);
}

void KeyconfigModel::TurnOffFunctionShortcut(int functionId)
{
	ByteString pref = ByteString(KEYCONFIG_FUNCS_PREF) + ByteString(".") + ByteString(functionId) 
		+ ByteString(".hasShortcut");

	Client::Ref().SetPref(pref, false);
}

void KeyconfigModel::TurnOnFunctionShortcut(int functionId)
{
	ByteString pref = ByteString(KEYCONFIG_FUNCS_PREF) + ByteString(".") + ByteString(functionId) 
		+ ByteString(".hasShortcut");

	Client::Ref().SetPref(pref, true);
}

void KeyconfigModel::RemoveModelByIndex(int index)
{
	std::vector<BindingModel>::iterator it = bindingPrefs.begin();

	while(it != bindingPrefs.end())
	{
		auto& pref = *it;
		if (pref.index == index)
		{
			bindingPrefs.erase(it);
			return;
		}
		
		it++;
	}
}

void KeyconfigModel::CreateModel(BindingModel model)
{
	// if the function has no shortcut then just turn it on
	if (!FunctionHasShortcut(model.functionId))
	{
		TurnOnFunctionShortcut(model.functionId);
		return;
	}

	// index is just an session based id that we use
	// to identify removals/changes
	// so whenever a new model is created we just set it to the 
	// size of the container
	model.index = bindingPrefs.size();
	bindingPrefs.push_back(model);
}

void KeyconfigModel::AddModel(BindingModel model)
{
	bindingPrefs.push_back(model);
	TurnOnFunctionShortcut(model.functionId);
	bool hasConflict = HasConflictingCombo();
	NotifyBindingsChanged(hasConflict);
}

bool KeyconfigModel::FunctionHasShortcut(int functionId)
{
	ByteString pref = ByteString(KEYCONFIG_FUNCS_PREF) + ByteString(".") + ByteString(functionId) 
		+ ByteString(".hasShortcut");

	return Client::Ref().GetPrefBool(pref, false);
}

void KeyconfigModel::Save()
{
	Client::Ref().ClearPref(KEYCONFIG_PREF);

	for (auto& binding : bindingPrefs)
	{
		ByteString mod(std::to_string(binding.modifier));
		ByteString scan(std::to_string(binding.scan));
		ByteString pref = ByteString(KEYCONFIG_PREF) + ByteString(".") + mod + ByteString("+") + scan;

		Json::Value val;
		val["functionId"] = binding.functionId;
		val["description"] = binding.description.ToUtf8();
		Client::Ref().SetPref(pref, val);
	}

	Client::Ref().WritePrefs();
}

int KeyconfigModel::GetFunctionForBinding(int scan, bool shift, bool ctrl, bool alt)
{
	int modifier = 0;

	if (ctrl)
		modifier |= BINDING_CTRL;

	if (alt)
		modifier |= BINDING_ALT;

	if (shift)
		modifier |= BINDING_SHIFT;

	auto it = std::find_if(bindingPrefs.begin(), bindingPrefs.end(), [modifier, scan](BindingModel m)
	{
		return m.modifier == modifier && m.scan == scan;
	});

	if (it != bindingPrefs.end())
	{
		BindingModel binding = *it;
		
		if (FunctionHasShortcut(binding.functionId))
			return binding.functionId;
	}

	return -1;
}

/**
 * Here we pop off a hotkey if the user clicks delete
 * however if we are on the last remaining hotkey
 * then we turn off hasShortcut for the associated function
 * so it renders as *No Shortcut* on the view
 */ 
void KeyconfigModel::PopBindingByFunctionId(int functionId)
{
	std::sort(bindingPrefs.begin(), bindingPrefs.end(), [](BindingModel a, BindingModel b)
	{
		return a.index > b.index;
	});

	std::vector<BindingModel> v;
	for (auto b : bindingPrefs)
	{
		if (b.functionId == functionId)
			v.push_back(b);
	}

	if (v.size() == 1)
	{
		auto it = std::find(bindingPrefs.begin(), bindingPrefs.end(), v[0]);
		TurnOffFunctionShortcut((*it).functionId);
	}
	else
	{
		auto it = bindingPrefs.begin();
		while (it != bindingPrefs.end())
		{
			if ((*it).functionId == functionId)
			{
				bindingPrefs.erase(it);
				break;
			}
			it++;
		}
	}
}

String KeyconfigModel::GetDisplayForModel(BindingModel model)
{
	return model.description;
}

bool KeyconfigModel::HasConflictingCombo()
{
	for (auto& binding : bindingPrefs)
	{
		// if we have any new bindings then we 
		// need to return a conflict until
		// the user types out a binding
		if (binding.isNew)
			return true;

		// if the current binding has no shortcut then skip
		if (!FunctionHasShortcut(binding.functionId))
			continue;

		// if key combo appears twice then there is a conflicting combo
		auto iter = std::find(bindingPrefs.begin(), bindingPrefs.end(), binding);
		if (iter != bindingPrefs.end())
		{
			// if this time round we don't have a shortcut either
			// then we can safely continue because this means
			// we don't have a conflict for the current binding
			if (!FunctionHasShortcut((*iter).functionId))
				continue;

			iter++;
			iter = std::find(iter, bindingPrefs.end(), binding);
			if (iter != bindingPrefs.end())
				return true;
		}
	}

	return false;
}

void KeyconfigModel::AddObserver(KeyconfigView* observer)
{
	observers.push_back(observer);
}

void KeyconfigModel::NotifyBindingsChanged(bool hasConflict)
{
	for (auto& observer : observers)
	{
		observer->OnKeyCombinationChanged(hasConflict);
	}
}
