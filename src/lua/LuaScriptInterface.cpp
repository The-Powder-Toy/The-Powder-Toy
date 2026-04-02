#include "LuaScriptInterface.h"
#include "client/http/Request.h"
#include "common/platform/Platform.h"
#include "common/tpt-rand.h"
#include "compat_lua.h"
#include "gui/game/GameController.h"
#include "gui/game/GameModel.h"
#include "gui/game/GameView.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Window.h"
#include "LuaBit.h"
#include "LuaComponent.h"
#include "prefs/GlobalPrefs.h"
#include "simulation/Simulation.h"
#include "simulation/SimulationData.h"

static int atPanic(lua_State *L)
{
	throw std::runtime_error("Unprotected lua panic: " + tpt_lua_toByteString(L, -1));
}

static int osExit(lua_State *L)
{
	Platform::Exit(luaL_optinteger(L, 1, 0));
	return 0;
}

static int mathRandom(lua_State *L)
{
	auto *lsi = GetLSI();
	// only thing that matters is that the rng not be sim->rng when !(eventTraits & eventTraitSimRng)
	auto &rng = (lsi->eventTraits & eventTraitSimRng) ? lsi->sim->rng : interfaceRng;
	int lower, upper;
	switch (lua_gettop(L))
	{
	case 0:
		lua_pushnumber(L, rng.uniform01());
		return 1;

	case 1:
		lower = 1;
		upper = luaL_checkinteger(L, 1);
		break;

	default:
		lower = luaL_checkinteger(L, 1);
		upper = luaL_checkinteger(L, 2);
		break;
	}
	if (upper < lower)
	{
		luaL_error(L, "interval is empty");
	}
	if ((unsigned int)(upper) - (unsigned int)(lower) + 1U)
	{
		lua_pushinteger(L, rng.between(lower, upper));
	}
	else
	{
		// The interval is *so* not empty that its size overflows 32-bit integers
		// (only possible if it's exactly 0x100000000); don't use between.
		lua_pushinteger(L, int(rng()));
	}
	return 1;
}

static int mathRandomseed(lua_State *L)
{
	interfaceRng.seed(luaL_checkinteger(L, 1));
	return 0;
}

static void hook(lua_State *L, lua_Debug * ar)
{
	auto *lsi = GetLSI();
	if (ar->event == LUA_HOOKCOUNT && int(Platform::GetTime() - lsi->luaExecutionStart) > lsi->luaHookTimeout)
	{
		luaL_error(L, "Error: Script not responding");
		lsi->luaExecutionStart = Platform::GetTime();
	}
}

int LuaToLoggableString(lua_State *L, int n)
{
	luaL_checkany(L, n);
	switch (lua_type(L, n))
	{
	case LUA_TNUMBER:
		lua_tostring(L, n);
		lua_pushvalue(L, n);
		break;
	case LUA_TSTRING:
		lua_pushvalue(L, n);
		break;
	case LUA_TBOOLEAN:
		lua_pushstring(L, (lua_toboolean(L, n) ? "true" : "false"));
		break;
	case LUA_TNIL:
		lua_pushliteral(L, "nil");
		break;
	default:
		lua_pushfstring(L, "%s: %p", luaL_typename(L, n), lua_topointer(L, n));
		break;
	}
	return 1;
}

String LuaGetError()
{
	auto *lsi = GetLSI();
	LuaToLoggableString(lsi->L, -1);
	String err = tpt_lua_optString(lsi->L, -1, "failed to execute");
	lua_pop(lsi->L, 1);
	return err;
}

LuaScriptInterface::LuaScriptInterface(GameController *newGameController, GameModel *newGameModel) :
	CommandInterface(newGameController, newGameModel),
	ren(newGameModel->GetRenderer()),
	gameModel(newGameModel),
	gameController(newGameController),
	window(gameController->GetView()),
	sim(gameModel->GetSimulation()),
	g(ui::Engine::Ref().g),
	customElements(PT_NUM),
	gameControllerEventHandlers(std::variant_size_v<GameControllerEvent>)
{
	auto &prefs = GlobalPrefs::Ref();
	luaHookTimeout = prefs.Get("LuaHookTimeout", 3000);
	for (auto moving = 0; moving < PT_NUM; ++moving)
	{
		for (auto into = 0; into < PT_NUM; ++into)
		{
			customCanMove[moving][into] = 0;
		}
	}
	luaState = LuaStatePtr(luaL_newstate());
	L = luaState.get();
	lua_sethook(L, hook, LUA_MASKCOUNT, 200);
	lua_atpanic(L, atPanic);
	luaL_openlibs(L);
	{
		luaopen_bit(L);
		lua_pop(L, 1);
	}
	LuaBz2::Open(L);
	LuaElements::Open(L);
	LuaEvent::Open(L);
	LuaFileSystem::Open(L);
	LuaGraphics::Open(L);
	LuaHttp::Open(L);
	LuaInterface::Open(L);
	LuaMisc::Open(L);
	LuaPlatform::Open(L);
	LuaRenderer::Open(L);
	LuaSimulation::Open(L);
	LuaSocket::Open(L);
	LuaTools::Open(L);
	{
		lua_getglobal(L, "os");
		lua_pushcfunction(L, osExit);
		lua_setfield(L, -2, "exit");
		lua_pop(L, 1);
	}
	{
		lua_getglobal(L, "math");
		lua_pushcfunction(L, mathRandom);
		lua_setfield(L, -2, "random");
		lua_pushcfunction(L, mathRandomseed);
		lua_setfield(L, -2, "randomseed");
		lua_pop(L, 1);
	}
	for (auto &ref : gameControllerEventHandlers)
	{
		lua_newtable(L);
		ref.Assign(L, -1);
		lua_pop(L, 1);
	}
	auto compatSpan = compat_lua.AsCharSpan();
	if (luaL_loadbuffer(L, compatSpan.data(), compatSpan.size(), "@[built-in compat.lua]") || tpt_lua_pcall(L, 0, 0, 0, eventTraitNone))
	{
		throw std::runtime_error(ByteString("failed to load built-in compat: ") + tpt_lua_toByteString(L, -1));
	}
}

void LuaScriptInterface::InitCustomCanMove()
{
	auto &sd = SimulationData::Ref();
	sd.init_can_move();
	for (auto moving = 0; moving < PT_NUM; ++moving)
	{
		for (auto into = 0; into < PT_NUM; ++into)
		{
			if (customCanMove[moving][into] & 0x80)
			{
				sd.can_move[moving][into] = customCanMove[moving][into] & 0x7F;
			}
		}
	}
}

void CommandInterface::Init()
{
	auto *lsi = static_cast<LuaScriptInterface *>(this);
	auto *L = lsi->L;
	if (Platform::FileExists("autorun.lua"))
	{
		if(luaL_loadfile(L, "autorun.lua") || tpt_lua_pcall(L, 0, 0, 0, eventTraitNone))
			Log(CommandInterface::LogError, LuaGetError());
		else
			Log(CommandInterface::LogWarning, "Loaded autorun.lua");
	}
}

void CommandInterface::SetToolIndex(ByteString identifier, std::optional<int> index)
{
	auto *lsi = static_cast<LuaScriptInterface *>(this);
	LuaTools::SetToolIndex(lsi->L, identifier, index);
}

void CommandInterface::RemoveComponents()
{
	auto *lsi = static_cast<LuaScriptInterface *>(this);
	for (auto &[ component, ref ] : lsi->grabbedComponents)
	{
		lsi->window->RemoveComponent(component->GetComponent());
		ref.Clear();
		component->owner_ref = ref;
		component->SetParentWindow(nullptr);
	}
}

void LuaGetProperty(lua_State *L, StructProperty property, intptr_t propertyAddress)
{
	switch (property.Type)
	{
		case StructProperty::TransitionType:
		case StructProperty::ParticleType:
		case StructProperty::Integer:
			lua_pushnumber(L, *((int*)propertyAddress));
			break;
		case StructProperty::UInteger:
			lua_pushnumber(L, *((unsigned int*)propertyAddress));
			break;
		case StructProperty::Float:
			lua_pushnumber(L, *((float*)propertyAddress));
			break;
		case StructProperty::UChar:
			lua_pushnumber(L, *((unsigned char*)propertyAddress));
			break;
		case StructProperty::BString:
		{
			tpt_lua_pushByteString(L, *((ByteString*)propertyAddress));
			break;
		}
		case StructProperty::String:
		{
			tpt_lua_pushString(L, *((String*)propertyAddress));
			break;
		}
		case StructProperty::Colour:
			lua_pushinteger(L, *((unsigned int*)propertyAddress));
			break;
		case StructProperty::Removed:
			lua_pushnil(L);
	}
}

static int32_t int32_truncate(double n)
{
	if (n >= 0x1p31)
	{
		n -= 0x1p32;
	}
	return int32_t(n);
}

void LuaSetProperty(lua_State *L, StructProperty property, intptr_t propertyAddress, int stackPos)
{
	switch (property.Type)
	{
		case StructProperty::TransitionType:
		case StructProperty::ParticleType:
		case StructProperty::Integer:
			*((int*)propertyAddress) = int32_truncate(luaL_checknumber(L, stackPos));
			break;
		case StructProperty::UInteger:
			*((unsigned int*)propertyAddress) = int32_truncate(luaL_checknumber(L, stackPos));
			break;
		case StructProperty::Float:
			*((float*)propertyAddress) = luaL_checknumber(L, stackPos);
			break;
		case StructProperty::UChar:
			*((unsigned char*)propertyAddress) = int32_truncate(luaL_checknumber(L, stackPos));
			break;
		case StructProperty::BString:
			*((ByteString*)propertyAddress) = tpt_lua_checkByteString(L, stackPos);
			break;
		case StructProperty::String:
			*((String*)propertyAddress) = tpt_lua_checkString(L, stackPos);
			break;
		case StructProperty::Colour:
			*((unsigned int*)propertyAddress) = int32_truncate(luaL_checknumber(L, stackPos));
			break;
		case StructProperty::Removed:
			break;
	}
}

void LuaSetParticleProperty(lua_State *L, int particleID, StructProperty property, intptr_t propertyAddress, int stackPos)
{
	auto *lsi = GetLSI();
	auto *sim = lsi->sim;
	if (property.Name == "type")
	{
		sim->part_change_type(particleID, int(sim->parts[particleID].x+0.5f), int(sim->parts[particleID].y+0.5f), luaL_checkinteger(L, 3));
	}
	else if (property.Name == "x" || property.Name == "y")
	{
		float val = luaL_checknumber(L, 3);
		float x = sim->parts[particleID].x;
		float y = sim->parts[particleID].y;
		float nx = property.Name == "x" ? val : x;
		float ny = property.Name == "y" ? val : y;
		sim->move(particleID, (int)(x + 0.5f), (int)(y + 0.5f), nx, ny);
	}
	else
	{
		LuaSetProperty(L, property, propertyAddress, 3);
	}
}

static int pushGameControllerEvent(lua_State *L, const GameControllerEvent &event)
{
	if (auto *textInputEvent = std::get_if<TextInputEvent>(&event))
	{
		tpt_lua_pushString(L, textInputEvent->text);
		return 1;
	}
	else if (auto *textEditingEvent = std::get_if<TextEditingEvent>(&event))
	{
		tpt_lua_pushString(L, textEditingEvent->text);
		return 1;
	}
	else if (auto *keyPressEvent = std::get_if<KeyPressEvent>(&event))
	{
		lua_pushinteger(L, keyPressEvent->key);
		lua_pushinteger(L, keyPressEvent->scan);
		lua_pushboolean(L, keyPressEvent->repeat);
		lua_pushboolean(L, keyPressEvent->shift);
		lua_pushboolean(L, keyPressEvent->ctrl);
		lua_pushboolean(L, keyPressEvent->alt);
		return 6;
	}
	else if (auto *keyReleaseEvent = std::get_if<KeyReleaseEvent>(&event))
	{
		lua_pushinteger(L, keyReleaseEvent->key);
		lua_pushinteger(L, keyReleaseEvent->scan);
		lua_pushboolean(L, keyReleaseEvent->repeat);
		lua_pushboolean(L, keyReleaseEvent->shift);
		lua_pushboolean(L, keyReleaseEvent->ctrl);
		lua_pushboolean(L, keyReleaseEvent->alt);
		return 6;
	}
	else if (auto *mouseDownEvent = std::get_if<MouseDownEvent>(&event))
	{
		lua_pushinteger(L, mouseDownEvent->x);
		lua_pushinteger(L, mouseDownEvent->y);
		lua_pushinteger(L, mouseDownEvent->button);
		return 3;
	}
	else if (auto *mouseUpEvent = std::get_if<MouseUpEvent>(&event))
	{
		lua_pushinteger(L, mouseUpEvent->x);
		lua_pushinteger(L, mouseUpEvent->y);
		lua_pushinteger(L, mouseUpEvent->button);
		lua_pushinteger(L, mouseUpEvent->reason);
		return 4;
	}
	else if (auto *mouseMoveEvent = std::get_if<MouseMoveEvent>(&event))
	{
		lua_pushinteger(L, mouseMoveEvent->x);
		lua_pushinteger(L, mouseMoveEvent->y);
		lua_pushinteger(L, mouseMoveEvent->dx);
		lua_pushinteger(L, mouseMoveEvent->dy);
		return 4;
	}
	else if (auto *mouseWheelEvent = std::get_if<MouseWheelEvent>(&event))
	{
		lua_pushinteger(L, mouseWheelEvent->x);
		lua_pushinteger(L, mouseWheelEvent->y);
		lua_pushinteger(L, mouseWheelEvent->d);
		return 3;
	}
	return 0;
}

bool CommandInterface::HandleEvent(const GameControllerEvent &event)
{
	auto *lsi = static_cast<LuaScriptInterface *>(this);
	auto *L = lsi->L;
	bool cont = true;
	lsi->gameControllerEventHandlers[event.index()].Push(L);
	int len = lua_objlen(L, -1);
	for (int i = 1; i <= len && cont; i++)
	{
		lua_rawgeti(L, -1, i);
		int numArgs = pushGameControllerEvent(L, event);
		int callret = tpt_lua_pcall(L, numArgs, 1, 0, std::visit([](auto &event) {
			return event.traits;
		}, event));
		if (callret)
		{
			if (LuaGetError() == "Error: Script not responding")
			{
				for (int j = i; j <= len - 1; j++)
				{
					lua_rawgeti(L, -2, j + 1);
					lua_rawseti(L, -3, j);
				}
				lua_pushnil(L);
				lua_rawseti(L, -3, len);
				i--;
			}
			Log(CommandInterface::LogError, LuaGetError());
			lua_pop(L, 1);
		}
		else
		{
			if (!lua_isnoneornil(L, -1))
				cont = lua_toboolean(L, -1);
			lua_pop(L, 1);
		}
		len = lua_objlen(L, -1);
	}
	lua_pop(L, 1);
	return cont;
}

template<size_t Index>
std::enable_if_t<Index != std::variant_size_v<GameControllerEvent>, bool> HaveSimGraphicsEventHandlersHelper(lua_State *L, std::vector<LuaSmartRef> &gameControllerEventHandlers)
{
	if (std::variant_alternative_t<Index, GameControllerEvent>::traits & eventTraitHindersSrt)
	{
		gameControllerEventHandlers[Index].Push(L);
		auto have = lua_objlen(L, -1) > 0;
		lua_pop(L, 1);
		if (have)
		{
			return true;
		}
	}
	return HaveSimGraphicsEventHandlersHelper<Index + 1>(L, gameControllerEventHandlers);
}

template<size_t Index>
std::enable_if_t<Index == std::variant_size_v<GameControllerEvent>, bool> HaveSimGraphicsEventHandlersHelper(lua_State *L, std::vector<LuaSmartRef> &gameControllerEventHandlers)
{
	return false;
}

bool CommandInterface::HaveSimGraphicsEventHandlers()
{
	auto &sd = SimulationData::CRef();
	auto *lsi = static_cast<LuaScriptInterface *>(this);
	for (int i = 0; i < int(lsi->customElements.size()); ++i)
	{
		if (lsi->customElements[i].graphics && !sd.graphicscache[i].isready && lsi->sim->elementCount[i])
		{
			return true;
		}
	}
	return HaveSimGraphicsEventHandlersHelper<0>(lsi->L, lsi->gameControllerEventHandlers);
}

void CommandInterface::OnTick()
{
	auto *lsi = static_cast<LuaScriptInterface *>(this);
	LuaMisc::Tick(lsi->L);
	HandleEvent(TickEvent{});
}

int CommandInterface::Command(String command)
{
	auto *lsi = static_cast<LuaScriptInterface *>(this);
	auto *L = lsi->L;
	lastError = "";
	lsi->luacon_hasLastError = false;
	if (command[0] == '!')
	{
		int ret = PlainCommand(command.Substr(1));
		lastError = GetLastError();
		return ret;
	}
	else
	{
		int level = lua_gettop(L), ret = -1;
		lsi->currentCommand = true;
		if (lsi->lastCode.length())
			lsi->lastCode += "\n";
		lsi->lastCode += command;
		ByteString tmp = ("return " + lsi->lastCode).ToUtf8();
		luaL_loadbuffer(L, tmp.data(), tmp.size(), "@console");
		if (lua_type(L, -1) != LUA_TFUNCTION)
		{
			lua_pop(L, 1);
			ByteString lastCodeUtf8 = lsi->lastCode.ToUtf8();
			luaL_loadbuffer(L, lastCodeUtf8.data(), lastCodeUtf8.size(), "@console");
		}
		if (lua_type(L, -1) != LUA_TFUNCTION)
		{
			lastError = LuaGetError();
			String err = lastError;
			if (err.Contains("near '<eof>'")) //the idea stolen from lua-5.1.5/lua.c
				lastError = "...";
			else
				lsi->lastCode = "";
		}
		else
		{
			lsi->lastCode = "";
			ret = tpt_lua_pcall(L, 0, LUA_MULTRET, 0, eventTraitNone);
			if (ret)
			{
				lastError = LuaGetError();
			}
			else
			{
				String text = "";
				bool hasText = false;
				for (level++; level <= lua_gettop(L); level++)
				{
					LuaToLoggableString(L, level);
					if (hasText)
					{
						text += ", " + tpt_lua_optString(L, -1, "");
					}
					else
					{
						text = tpt_lua_optString(L, -1, "");
						hasText = true;
					}
					lua_pop(L, 1);
				}
				if (text.length())
				{
					if (lastError.length())
						lastError += "; " + text;
					else
						lastError = text;
				}

			}
		}
		lsi->currentCommand = false;
		return ret;
	}
}

static String highlight(String command)
{
	StringBuilder result;
	int pos = 0;
	String::value_type const*raw = command.c_str();
	String::value_type c;
	while ((c = raw[pos]))
	{
		if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
		{
			int len = 0;
			String::value_type w;
			String::value_type const* wstart = raw+pos;
			while((w = wstart[len]) && ((w >= 'A' && w <= 'Z') || (w >= 'a' && w <= 'z') || (w >= '0' && w <= '9') || w == '_'))
				len++;
#define CMP(X) (String(wstart, len) == X)
			if(CMP("and") || CMP("break") || CMP("do") || CMP("else") || CMP("elseif") || CMP("end") || CMP("for") || CMP("function") || CMP("if") || CMP("in") || CMP("local") || CMP("not") || CMP("or") || CMP("repeat") || CMP("return") || CMP("then") || CMP("until") || CMP("while"))
				result << "\x0F\xB5\x89\x01" << String(wstart, len) << "\bw";
			else if(CMP("false") || CMP("nil") || CMP("true"))
				result << "\x0F\xCB\x4B\x16" << String(wstart, len) << "\bw";
			else
				result << "\x0F\x2A\xA1\x98" << String(wstart, len) << "\bw";
#undef CMP
			pos += len;
		}
		else if((c >= '0' && c <= '9') || (c == '.' && raw[pos + 1] >= '0' && raw[pos + 1] <= '9'))
		{
			if(c == '0' && raw[pos + 1] == 'x')
			{
				int len = 2;
				String::value_type w;
				String::value_type const* wstart = raw+pos;
				while((w = wstart[len]) && ((w >= '0' && w <= '9') || (w >= 'A' && w <= 'F') || (w >= 'a' && w <= 'f')))
					len++;
				result << "\x0F\xD3\x36\x82" << String(wstart, len) << "\bw";
				pos += len;
			}
			else
			{
				int len = 0;
				String::value_type w;
				String::value_type const* wstart = raw+pos;
				bool seendot = false;
				while((w = wstart[len]) && ((w >= '0' && w <= '9') || w == '.'))
				{
					if(w == '.')
					{
						if(seendot)
							break;
						else
							seendot = true;
					}
					len++;
				}
				if(w == 'e')
				{
					len++;
					w = wstart[len];
					if(w == '+' || w == '-')
						len++;
					while((w = wstart[len]) && (w >= '0' && w <= '9'))
						len++;
				}
				result << "\x0F\xD3\x36\x82" << String(wstart, len) << "\bw";
				pos += len;
			}
		}
		else if(c == '\'' || c == '"' || (c == '[' && (raw[pos + 1] == '[' || raw[pos + 1] == '=')))
		{
			if(c == '[')
			{
				int len = 1, eqs=0;
				String::value_type w;
				String::value_type const* wstart = raw + pos;
				while((w = wstart[len]) && (w == '='))
				{
					eqs++;
					len++;
				}
				while((w = wstart[len]))
				{
					if(w == ']')
					{
						int nlen = 1;
						String::value_type const* cstart = wstart + len;
						while((w = cstart[nlen]) && (w == '='))
							nlen++;
						if(w == ']' && nlen == eqs+1)
						{
							len += nlen+1;
							break;
						}
					}
					len++;
				}
				result << "\x0F\xDC\x32\x2F" << String(wstart, len) << "\bw";
				pos += len;
			}
			else
			{
				int len = 1;
				String::value_type w;
				String::value_type const* wstart = raw+pos;
				while((w = wstart[len]) && (w != c))
				{
					if(w == '\\' && wstart[len + 1])
						len++;
					len++;
				}
				if(w == c)
					len++;
				result << "\x0F\xDC\x32\x2F" << String(wstart, len) << "\bw";
				pos += len;
			}
		}
		else if(c == '-' && raw[pos + 1] == '-')
		{
			if(raw[pos + 2] == '[')
			{
				int len = 3, eqs = 0;
				String::value_type w;
				String::value_type const* wstart = raw + pos;
				while((w = wstart[len]) && (w == '='))
				{
					eqs++;
					len++;
				}
				while((w = wstart[len]))
				{
					if(w == ']')
					{
						int nlen = 1;
						String::value_type const* cstart = wstart + len;
						while((w = cstart[nlen]) && (w == '='))
							nlen++;
						if(w == ']' && nlen == eqs + 1)
						{
							len += nlen+1;
							break;
						}
					}
					len++;
				}
				result << "\x0F\x85\x99\x01" << String(wstart, len) << "\bw";
				pos += len;
			}
			else
			{
				int len = 2;
				String::value_type w;
				String::value_type const* wstart = raw + pos;
				while((w = wstart[len]) && (w != '\n'))
					len++;
				result << "\x0F\x85\x99\x01" << String(wstart, len) << "\bw";
				pos += len;
			}
		}
		else if(c == '{' || c == '}')
		{
			result << "\x0F\xCB\x4B\x16" << c << "\bw";
			pos++;
		}
		else if(c == '.' && raw[pos + 1] == '.' && raw[pos + 2] == '.')
		{
			result << "\x0F\x2A\xA1\x98...\bw";
			pos += 3;
		}
		else
		{
			result << c;
			pos++;
		}
	}
	return result.Build();
}

String CommandInterface::FormatCommand(String command)
{
	if(command.size() && command[0] == '!')
	{
		return "!" + PlainFormatCommand(command.Substr(1));
	}
	else
		return highlight(command);
}

LuaScriptInterface::~LuaScriptInterface() = default;

void tpt_lua_pushByteString(lua_State *L, const ByteString &str)
{
	lua_pushlstring(L, str.data(), str.size());
}

void tpt_lua_pushString(lua_State *L, const String &str)
{
	tpt_lua_pushByteString(L, str.ToUtf8());
}

ByteString tpt_lua_toByteString(lua_State *L, int index)
{
	size_t size;
	if (auto *data = lua_tolstring(L, index, &size))
	{
		return ByteString(data, size);
	}
	return {};
}

String tpt_lua_toString(lua_State *L, int index, bool ignoreError)
{
	return tpt_lua_toByteString(L, index).FromUtf8(ignoreError);
}

ByteString tpt_lua_checkByteString(lua_State *L, int index)
{
	size_t size;
	if (auto *data = luaL_checklstring(L, index, &size))
	{
		return ByteString(data, size);
	}
	return {};
}

String tpt_lua_checkString(lua_State *L, int index, bool ignoreError)
{
	return tpt_lua_checkByteString(L, index).FromUtf8(ignoreError);
}

ByteString tpt_lua_optByteString(lua_State *L, int index, ByteString defaultValue)
{
	if (lua_isnoneornil(L, index))
	{
		return defaultValue;
	}
	return tpt_lua_checkByteString(L, index);
}

String tpt_lua_optString(lua_State *L, int index, String defaultValue, bool ignoreError)
{
	if (lua_isnoneornil(L, index))
	{
		return defaultValue;
	}
	return tpt_lua_checkString(L, index, ignoreError);
}

int tpt_lua_loadstring(lua_State *L, const ByteString &str)
{
	return luaL_loadbuffer(L, str.data(), str.size(), str.data());
}

int tpt_lua_dostring(lua_State *L, const ByteString &str)
{
	return tpt_lua_loadstring(L, str) || tpt_lua_pcall(L, 0, LUA_MULTRET, 0, eventTraitNone);
}

bool tpt_lua_equalsString(lua_State *L, int index, const char *data, size_t size)
{
	return lua_isstring(L, index) && lua_objlen(L, index) == size && !memcmp(lua_tostring(L, index), data, size);
}

int tpt_lua_pcall(lua_State *L, int numArgs, int numResults, int errorFunc, EventTraits newEventTraits)
{
	auto *lsi = GetLSI();
	lsi->luaExecutionStart = Platform::GetTime();
	struct AtReturn
	{
		EventTraits oldEventTraits;

		AtReturn(EventTraits newEventTraits)
		{
			auto *lsi = GetLSI();
			oldEventTraits = lsi->eventTraits;
			lsi->eventTraits = newEventTraits;
		}

		~AtReturn()
		{
			auto *lsi = GetLSI();
			lsi->eventTraits = oldEventTraits;
		}
	} atReturn(newEventTraits);
	return lua_pcall(L, numArgs, numResults, errorFunc);
}

CommandInterfacePtr CommandInterface::Create(GameController *newGameController, GameModel *newGameModel)
{
	return CommandInterfacePtr(new LuaScriptInterface(newGameController, newGameModel));
}

void CommandInterfaceDeleter::operator ()(CommandInterface *ptr) const
{
	delete static_cast<LuaScriptInterface *>(ptr);
}

