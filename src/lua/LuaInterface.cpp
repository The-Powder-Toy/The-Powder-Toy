#include "LuaScriptInterface.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/dialogues/TextPrompt.h"
#include "gui/game/Brush.h"
#include "gui/game/GameController.h"
#include "gui/game/GameModel.h"
#include "gui/game/GameView.h"
#include "gui/game/Tool.h"
#include "gui/interface/Engine.h"
#include "LuaButton.h"
#include "LuaCheckbox.h"
#include "LuaLabel.h"
#include "LuaLuna.h"
#include "LuaProgressBar.h"
#include "LuaSDLKeys.h"
#include "LuaSlider.h"
#include "LuaTextbox.h"
#include "LuaWindow.h"
#include "prefs/GlobalPrefs.h"
#include "simulation/SimulationData.h"

template<class Type>
struct PickIfTypeHelper;

template<>
struct PickIfTypeHelper<String>
{
	static constexpr auto LuaType = LUA_TSTRING;
	static String Get(lua_State *L, int index) { return tpt_lua_checkString(L, index); }
};

template<>
struct PickIfTypeHelper<bool>
{
	static constexpr auto LuaType = LUA_TBOOLEAN;
	static bool Get(lua_State *L, int index) { return lua_toboolean(L, index); }
};

template<class Type>
static Type PickIfType(lua_State *L, int index, Type defaultValue)
{
	return lua_type(L, index) == PickIfTypeHelper<Type>::LuaType ? PickIfTypeHelper<Type>::Get(L, index) : defaultValue;
}

static int beginMessageBox(lua_State *L)
{
	auto title = PickIfType(L, 1, String("Title"));
	auto message = PickIfType(L, 2, String("Message"));
	auto large = PickIfType(L, 3, false);
	auto cb = std::make_shared<LuaSmartRef>(); // * Bind to main lua state (might be different from L).
	if (lua_gettop(L))
	{
		cb->Assign(L, lua_gettop(L));
	}
	new InformationMessage(title, message, large, { [cb]() {
		auto *lsi = GetLSI();
		auto L = lsi->L;
		cb->Push(L);
		if (lua_isfunction(L, -1))
		{
			if (tpt_lua_pcall(L, 0, 0, 0, eventTraitNone))
			{
				lsi->Log(CommandInterface::LogError, LuaGetError());
			}
		}
		else
		{
			lua_pop(L, 1);
		}
	} });
	return 0;
}

static int beginThrowError(lua_State *L)
{
	auto errorMessage = PickIfType(L, 1, String("Error text"));
	auto cb = std::make_shared<LuaSmartRef>(); // * Bind to main lua state (might be different from L).
	if (lua_gettop(L))
	{
		cb->Assign(L, lua_gettop(L));
	}
	new ErrorMessage("Error", errorMessage, { [cb]() {
		auto *lsi = GetLSI();
		auto L = lsi->L;
		cb->Push(L);
		if (lua_isfunction(L, -1))
		{
			if (tpt_lua_pcall(L, 0, 0, 0, eventTraitNone))
			{
				lsi->Log(CommandInterface::LogError, LuaGetError());
			}
		}
		else
		{
			lua_pop(L, 1);
		}
	} });
	return 0;
}

static int beginInput(lua_State *L)
{
	auto title = PickIfType(L, 1, String("Title"));
	auto prompt = PickIfType(L, 2, String("Enter some text:"));
	auto text = PickIfType(L, 3, String(""));
	auto shadow = PickIfType(L, 4, String(""));
	auto cb = std::make_shared<LuaSmartRef>(); // * Bind to main lua state (might be different from L).
	if (lua_gettop(L))
	{
		cb->Assign(L, lua_gettop(L));
	}
	auto handle = [cb](std::optional<String> input) {
		auto *lsi = GetLSI();
		auto L = lsi->L;
		cb->Push(L);
		if (lua_isfunction(L, -1))
		{
			if (input)
			{
				tpt_lua_pushString(L, *input);
			}
			else
			{
				lua_pushnil(L);
			}
			if (tpt_lua_pcall(L, 1, 0, 0, eventTraitNone))
			{
				lsi->Log(CommandInterface::LogError, LuaGetError());
			}
		}
		else
		{
			lua_pop(L, 1);
		}
	};
	new TextPrompt(title, prompt, text, shadow, false, { [handle](const String &input) {
		handle(input);
	}, [handle]() {
		handle(std::nullopt);
	} });
	return 0;
}

static int beginConfirm(lua_State *L)
{
	auto title = PickIfType(L, 1, String("Title"));
	auto message = PickIfType(L, 2, String("Message"));
	auto buttonText = PickIfType(L, 3, String("Confirm"));
	auto cb = std::make_shared<LuaSmartRef>(); // * Bind to main lua state (might be different from L).
	if (lua_gettop(L))
	{
		cb->Assign(L, lua_gettop(L));
	}
	auto handle = [cb](int result) {
		auto *lsi = GetLSI();
		auto L = lsi->L;
		cb->Push(L);
		if (lua_isfunction(L, -1))
		{
			lua_pushboolean(L, result);
			if (tpt_lua_pcall(L, 1, 0, 0, eventTraitNone))
			{
				lsi->Log(CommandInterface::LogError, LuaGetError());
			}
		}
		else
		{
			lua_pop(L, 1);
		}
	};
	new ConfirmPrompt(title, message, { [handle]() {
		handle(1);
	}, [handle]() {
		handle(0);
	} }, buttonText);
	return 0;
}

static int console(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushboolean(L, lsi->window != ui::Engine::Ref().GetWindow());
		return 1;
	}
	if (lua_toboolean(L, 1))
		lsi->gameController->ShowConsole();
	else
		lsi->gameController->HideConsole();
	return 0;
}

static int brushID(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lua_gettop(L) < 1)
	{
		lua_pushnumber(L, lsi->gameModel->GetBrushID());
		return 1;
	}
	auto index = luaL_checkint(L, 1);
	if (index < 0 || index >= int(lsi->gameModel->BrushListSize()))
	{
		return luaL_error(L, "Invalid brush index %i", index);
	}
	lsi->gameModel->SetBrushID(index);
	return 0;
}

static int brushRadius(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lua_gettop(L) < 1)
	{
		auto radius = lsi->gameModel->GetBrush().GetRadius();
		lua_pushnumber(L, radius.X);
		lua_pushnumber(L, radius.Y);
		return 2;
	}
	lsi->gameModel->GetBrush().SetRadius({ luaL_checkint(L, 1), luaL_checkint(L, 2) });
	return 0;
}

static int mousePosition(lua_State *L)
{
	auto *lsi = GetLSI();
	auto pos = lsi->gameController->GetView()->GetMousePosition();
	lua_pushnumber(L, pos.X);
	lua_pushnumber(L, pos.Y);
	return 2;
}

static int activeTool(lua_State *L)
{	auto *lsi = GetLSI();
	auto index = luaL_checkint(L, 1);
	if (index < 0 || index >= NUM_TOOLINDICES)
	{
		return luaL_error(L, "Invalid tool index %i", index);
	}
	if (lua_gettop(L) < 2)
	{
		tpt_lua_pushByteString(L, lsi->gameModel->GetActiveTool(index)->Identifier);
		return 1;
	}
	auto identifier = tpt_lua_checkByteString(L, 2);
	auto *tool = lsi->gameModel->GetToolFromIdentifier(identifier);
	if (!tool)
	{
		return luaL_error(L, "Invalid tool identifier %s", identifier.c_str());
	}
	lsi->gameController->SetActiveTool(index, tool);
	return 0;
}

static int addComponent(lua_State *L)
{
	auto *lsi = GetLSI();
	void *opaque = nullptr;
	LuaComponent *luaComponent = nullptr;
	if ((opaque = Luna<LuaButton>::tryGet(L, 1)))
		luaComponent = Luna<LuaButton>::get(opaque);
	else if ((opaque = Luna<LuaLabel>::tryGet(L, 1)))
		luaComponent = Luna<LuaLabel>::get(opaque);
	else if ((opaque = Luna<LuaTextbox>::tryGet(L, 1)))
		luaComponent = Luna<LuaTextbox>::get(opaque);
	else if ((opaque = Luna<LuaCheckbox>::tryGet(L, 1)))
		luaComponent = Luna<LuaCheckbox>::get(opaque);
	else if ((opaque = Luna<LuaSlider>::tryGet(L, 1)))
		luaComponent = Luna<LuaSlider>::get(opaque);
	else if ((opaque = Luna<LuaProgressBar>::tryGet(L, 1)))
		luaComponent = Luna<LuaProgressBar>::get(opaque);
	else
		luaL_typerror(L, 1, "Component");
	if (lsi->window && luaComponent)
	{
		auto ok = lsi->grabbedComponents.insert(std::make_pair(luaComponent, LuaSmartRef()));
		if (ok.second)
		{
			auto it = ok.first;
			it->second.Assign(L, 1);
			it->first->owner_ref = it->second;
		}
		lsi->window->AddComponent(luaComponent->GetComponent());
	}
	return 0;
}

static int removeComponent(lua_State *L)
{
	auto *lsi = GetLSI();
	void *opaque = nullptr;
	LuaComponent *luaComponent = nullptr;
	if ((opaque = Luna<LuaButton>::tryGet(L, 1)))
		luaComponent = Luna<LuaButton>::get(opaque);
	else if ((opaque = Luna<LuaLabel>::tryGet(L, 1)))
		luaComponent = Luna<LuaLabel>::get(opaque);
	else if ((opaque = Luna<LuaTextbox>::tryGet(L, 1)))
		luaComponent = Luna<LuaTextbox>::get(opaque);
	else if ((opaque = Luna<LuaCheckbox>::tryGet(L, 1)))
		luaComponent = Luna<LuaCheckbox>::get(opaque);
	else if ((opaque = Luna<LuaSlider>::tryGet(L, 1)))
		luaComponent = Luna<LuaSlider>::get(opaque);
	else if ((opaque = Luna<LuaProgressBar>::tryGet(L, 1)))
		luaComponent = Luna<LuaProgressBar>::get(opaque);
	else
		luaL_typerror(L, 1, "Component");
	if(lsi->window && luaComponent)
	{
		ui::Component *component = luaComponent->GetComponent();
		lsi->window->RemoveComponent(component);
		auto it = lsi->grabbedComponents.find(luaComponent);
		if (it != lsi->grabbedComponents.end())
		{
			it->second.Clear();
			it->first->owner_ref = it->second;
			lsi->grabbedComponents.erase(it);
		}
	}
	return 0;
}

static int grabTextInput(lua_State *L)
{
	auto *lsi = GetLSI();
	lsi->textInputRefcount += 1;
	lsi->gameController->GetView()->DoesTextInput = lsi->textInputRefcount > 0;
	return 0;
}

static int dropTextInput(lua_State *L)
{
	auto *lsi = GetLSI();
	lsi->textInputRefcount -= 1;
	lsi->gameController->GetView()->DoesTextInput = lsi->textInputRefcount > 0;
	return 0;
}

static int textInputRect(lua_State *L)
{
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	int w = luaL_checkint(L, 3);
	int h = luaL_checkint(L, 4);
	ui::Engine::Ref().TextInputRect(ui::Point{ x, y }, ui::Point{ w, h });
	return 0;
}

static int showWindow(lua_State *L)
{
	LuaWindow * window = Luna<LuaWindow>::check(L, 1);

	if(window && ui::Engine::Ref().GetWindow()!=window->GetWindow())
		ui::Engine::Ref().ShowWindow(window->GetWindow());
	return 0;
}

static int closeWindow(lua_State *L)
{
	LuaWindow * window = Luna<LuaWindow>::check(L, 1);
	if (window)
		window->GetWindow()->CloseActiveWindow();
	return 0;
}

static int perfectCircleBrush(lua_State *L)
{
	auto *lsi = GetLSI();
	if (!lua_gettop(L))
	{
		lua_pushboolean(L, lsi->gameModel->GetPerfectCircle());
		return 1;
	}
	luaL_checktype(L, 1, LUA_TBOOLEAN);
	lsi->gameModel->SetPerfectCircle(lua_toboolean(L, 1));
	return 0;
}

static int activeMenu(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushinteger(L, lsi->gameModel->GetActiveMenu());
		return 1;
	}
	auto &sd = SimulationData::CRef();
	int menuid = luaL_checkint(L, 1);
	if (menuid >= 0 && menuid < int(sd.msections.size()))
		lsi->gameController->SetActiveMenu(menuid);
	else
		return luaL_error(L, "Invalid menu");
	return 0;
}

static int menuEnabled(lua_State *L)
{
	int menusection = luaL_checkint(L, 1);
	{
		auto &sd = SimulationData::CRef();
		if (menusection < 0 || menusection >= int(sd.msections.size()))
			return luaL_error(L, "Invalid menu");
	}
	int acount = lua_gettop(L);
	if (acount == 1)
	{
		lua_pushboolean(L, SimulationData::CRef().msections[menusection].doshow);
		return 1;
	}
	luaL_checktype(L, 2, LUA_TBOOLEAN);
	int enabled = lua_toboolean(L, 2);
	{
		auto &sd = SimulationData::Ref();
		sd.msections[menusection].doshow = enabled;
	}
	auto *lsi = GetLSI();
	lsi->gameModel->BuildMenus();
	return 0;
}

static int numMenus(lua_State *L)
{
	int acount = lua_gettop(L);
	bool onlyEnabled = true;
	if (acount > 0)
	{
		luaL_checktype(L, 1, LUA_TBOOLEAN);
		onlyEnabled = lua_toboolean(L, 1);
	}
	auto *lsi = GetLSI();
	lua_pushinteger(L, lsi->gameController->GetNumMenus(onlyEnabled));
	return 1;
}

static int windowSize(lua_State *L)
{
	auto &g = ui::Engine::Ref();
	if (lua_gettop(L) < 1)
	{
		lua_pushinteger(L, g.GetScale());
		lua_pushboolean(L, g.GetFullscreen());
		return 2;
	}
	int scale = luaL_optint(L,1,1);
	auto kiosk = lua_toboolean(L,2);
	// TODO: handle this the same way as it's handled in PowderToySDL.cpp
	//   > maybe bind the maximum allowed scale to screen size somehow
	if (scale < 1 || scale > 10)
	{
		scale = 1;
	}
	{
		auto &prefs = GlobalPrefs::Ref();
		Prefs::DeferWrite dw(prefs);
		prefs.Set("Scale", scale);
		prefs.Set("Fullscreen", kiosk);
	}
	g.SetScale(scale);
	g.SetFullscreen(kiosk);
	return 0;
}

void LuaInterface::Open(lua_State *L)
{
	static const luaL_Reg reg[] = {
#define LFUNC(v) { #v, v }
		LFUNC(showWindow),
		LFUNC(closeWindow),
		LFUNC(addComponent),
		LFUNC(removeComponent),
		LFUNC(grabTextInput),
		LFUNC(dropTextInput),
		LFUNC(textInputRect),
		LFUNC(beginInput),
		LFUNC(beginMessageBox),
		LFUNC(beginConfirm),
		LFUNC(beginThrowError),
		LFUNC(activeMenu),
		LFUNC(menuEnabled),
		LFUNC(numMenus),
		LFUNC(perfectCircleBrush),
		LFUNC(console),
		LFUNC(windowSize),
		LFUNC(brushID),
		LFUNC(brushRadius),
		LFUNC(mousePosition),
		LFUNC(activeTool),
#undef LFUNC
		{ NULL, NULL }
	};
	lua_newtable(L);
	luaL_register(L, NULL, reg);
#define LCONSTAS(k, v) lua_pushinteger(L, int(v)); lua_setfield(L, -2, k)
	LCONSTAS("MOUSEUP_NORMAL" , GameController::mouseUpNormal);
	LCONSTAS("MOUSEUP_BLUR"   , GameController::mouseUpBlur);
	LCONSTAS("MOUSEUP_DRAWEND", GameController::mouseUpDrawEnd);
	LCONSTAS("NUM_TOOLINDICES", NUM_TOOLINDICES);
#undef LCONSTAS
	initLuaSDLKeys(L);
	lua_pushvalue(L, -1);
	lua_setglobal(L, "interface");
	lua_setglobal(L, "ui");
	Luna<LuaWindow     >::Register(L);
	Luna<LuaButton     >::Register(L);
	Luna<LuaLabel      >::Register(L);
	Luna<LuaTextbox    >::Register(L);
	Luna<LuaCheckbox   >::Register(L);
	Luna<LuaSlider     >::Register(L);
	Luna<LuaProgressBar>::Register(L);
}
