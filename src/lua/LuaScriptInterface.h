#pragma once
#include "LuaCompat.h"
#include "LuaSmartRef.h"
#include "CommandInterface.h"
#include "gui/game/GameControllerEvents.h"
#include "simulation/StructProperty.h"
#include "simulation/ElementDefs.h"
#include <cstdint>
#include <map>
#include <memory>

namespace http
{
	class Request;
}

namespace ui
{
	class Window;
}

class Graphics;
class Renderer;
class Simulation;
class LuaComponent;

int LuaToLoggableString(lua_State *L, int n);
String LuaGetError();
void LuaGetProperty(lua_State *L, StructProperty property, intptr_t propertyAddress);
void LuaSetProperty(lua_State *L, StructProperty property, intptr_t propertyAddress, int stackPos);
void LuaSetParticleProperty(lua_State *L, int particleID, StructProperty property, intptr_t propertyAddress, int stackPos);

struct LuaStateDeleter
{
	void operator ()(lua_State *L) const
	{
		lua_close(L);
	}
};
using LuaStatePtr = std::unique_ptr<lua_State, LuaStateDeleter>;

enum UpdateMode
{
	UPDATE_AFTER,
	UPDATE_REPLACE,
	UPDATE_BEFORE,
	NUM_UPDATEMODES,
};

struct CustomElement
{
	UpdateMode updateMode = UPDATE_AFTER;
	LuaSmartRef update;
	LuaSmartRef graphics;
	LuaSmartRef ctypeDraw;
	LuaSmartRef create;
	LuaSmartRef createAllowed;
	LuaSmartRef changeType;
};

struct CustomTool
{
	LuaSmartRef perform;
	LuaSmartRef click;
	LuaSmartRef drag;
	LuaSmartRef draw;
	LuaSmartRef drawLine;
	LuaSmartRef drawRect;
	LuaSmartRef drawFill;
};

class LuaScriptInterface : public CommandInterface
{
	LuaStatePtr luaState;

	Renderer *ren;

public:
	lua_State *L{};

	GameModel *gameModel;
	GameController *gameController;
	ui::Window *window;
	Simulation *sim;
	Graphics *g;

	std::variant<Graphics *, Renderer *> GetGraphics()
	{
		if (eventTraits & eventTraitSimGraphics)
		{
			// This is ok without calling gameModel->view->PauseRendererThread() because
			// the renderer thread gets paused anyway if there are handlers
			// installed for eventTraitSimGraphics and *SimDraw events.
			return ren;
		}
		return g;
	}

	std::vector<CustomElement> customElements; // must come after luaState
	std::vector<CustomTool> customTools;

	EventTraits eventTraits = eventTraitNone;

	bool luacon_hasLastError = false;
	String lastCode;

	bool currentCommand = false;
	int textInputRefcount = 0;
	long unsigned int luaExecutionStart = 0;

	std::vector<LuaSmartRef> gameControllerEventHandlers; // must come after luaState
	std::unique_ptr<http::Request> scriptManagerDownload;
	int luaHookTimeout;

	std::map<LuaComponent *, LuaSmartRef> grabbedComponents; // must come after luaState

	LuaScriptInterface(GameController *newGameController, GameModel *newGameModel);
	~LuaScriptInterface();

	char customCanMove[PT_NUM][PT_NUM];
	void InitCustomCanMove();
};

void tpt_lua_pushByteString(lua_State *L, const ByteString &str);
void tpt_lua_pushString(lua_State *L, const String &str);

// TODO: toByteStringView once we have a ByteStringView (or std::string_view, if we get rid of ByteString)
ByteString tpt_lua_toByteString(lua_State *L, int index);
String tpt_lua_toString(lua_State *L, int index, bool ignoreError = true);

// TODO: toByteStringView once we have a ByteStringView (or std::string_view, if we get rid of ByteString)
ByteString tpt_lua_checkByteString(lua_State *L, int index);
String tpt_lua_checkString(lua_State *L, int index, bool ignoreError = true);

// TODO: toByteStringView once we have a ByteStringView (or std::string_view, if we get rid of ByteString)
ByteString tpt_lua_optByteString(lua_State *L, int index, ByteString defaultValue = {});
String tpt_lua_optString(lua_State *L, int index, String defaultValue = {}, bool ignoreError = true);

int tpt_lua_loadstring(lua_State *L, const ByteString &str);
int tpt_lua_dostring(lua_State *L, const ByteString &str);

bool tpt_lua_equalsString(lua_State *L, int index, const char *data, size_t size);

// TODO: use std::literals::string_literals::operator""s if we get rid of ByteString
template<size_t N>
bool tpt_lua_equalsLiteral(lua_State *L, int index, const char (&lit)[N])
{
	return tpt_lua_equalsString(L, index, lit, N - 1U);
}

int tpt_lua_pcall(lua_State *L, int numArgs, int numResults, int errorFunc, EventTraits eventTraits);

namespace LuaHttp
{
	void Open(lua_State *L);
}

namespace LuaBz2
{
	void Open(lua_State *L);
}

namespace LuaElements
{
	void Open(lua_State *L);
}

namespace LuaEvent
{
	void Open(lua_State *L);
}

namespace LuaFileSystem
{
	void Open(lua_State *L);
}

namespace LuaGraphics
{
	void Open(lua_State *L);
}

namespace LuaInterface
{
	void Open(lua_State *L);
}

namespace LuaMisc
{
	void Open(lua_State *L);
	void Tick(lua_State *L);
}

namespace LuaPlatform
{
	void Open(lua_State *L);
}

namespace LuaRenderer
{
	void Open(lua_State *L);
}

namespace LuaSimulation
{
	void Open(lua_State *L);
}

namespace LuaSocket
{
	int GetTime(lua_State *L);
	int Sleep(lua_State *L);
	double Now();
	void Timeout(double timeout);
	void Open(lua_State *L);
	void OpenTCP(lua_State *L);
}

namespace LuaTools
{
	void Open(lua_State *L);
	void SetToolIndex(lua_State *L, ByteString identifier, std::optional<int> index);
}

inline LuaScriptInterface *GetLSI()
{
	return static_cast<LuaScriptInterface *>(&CommandInterface::Ref());
}
