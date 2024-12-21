#include "LuaScriptInterface.h"
#include "client/http/Request.h"
#include "common/platform/Platform.h"
#include "compat_lua.h"
#include "Config.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/game/GameController.h"
#include "gui/game/GameModel.h"
#include "gui/game/GameView.h"
#include "gui/interface/Engine.h"

static int getUserName(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lsi->gameModel->GetUser().UserID)
	{
		tpt_lua_pushByteString(L, lsi->gameModel->GetUser().Username);
		return 1;
	}
	lua_pushliteral(L, "");
	return 1;
}

static int installScriptManager(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lsi->scriptManagerDownload)
	{
		new ErrorMessage("Script download", "A script download is already pending");
		return 0;
	}
	lsi->gameController->HideConsole();
	if (ui::Engine::Ref().GetWindow() != lsi->gameController->GetView())
	{
		new ErrorMessage("Script download", "You must run this function from the console");
		return 0;
	}
	lsi->scriptManagerDownload = std::make_unique<http::Request>(ByteString::Build("https://starcatcher.us/scripts/main.lua?get=1"));
	lsi->scriptManagerDownload->Start();
	return 0;
}

void LuaMisc::Tick(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lsi->scriptManagerDownload && lsi->scriptManagerDownload->CheckDone())
	{
		auto scriptManagerDownload = std::move(lsi->scriptManagerDownload);
		struct Status
		{
			struct Ok
			{
			};
			struct GetFailed
			{
				String error;
			};
			struct RunFailed
			{
				String error;
			};
			using Value = std::variant<
				Ok,
				GetFailed,
				RunFailed
			>;
			Value value;
		};
		auto complete = [](Status status) {
			if (std::get_if<Status::Ok>(&status.value))
			{
				new InformationMessage("Install script manager", "Script manager successfully installed", false);
			}
			if (auto *requestFailed = std::get_if<Status::GetFailed>(&status.value))
			{
				new ErrorMessage("Install script manager", "Failed to get script manager: " + requestFailed->error);
			}
			if (auto *runFailed = std::get_if<Status::RunFailed>(&status.value))
			{
				new ErrorMessage("Install script manager", "Failed to run script manager: " + runFailed->error);
			}
		};
		try
		{
			auto ret = scriptManagerDownload->StatusCode();
			auto scriptData = scriptManagerDownload->Finish().second;
			if (!scriptData.size())
			{
				complete({ Status::GetFailed{ "Server did not return data" } });
				return;
			}
			if (ret != 200)
			{
				complete({ Status::GetFailed{ ByteString(http::StatusText(ret)).FromUtf8() } });
				return;
			}
			ByteString filename = "autorun.lua";
			if (!Platform::WriteFile(scriptData, filename))
			{
				complete({ Status::GetFailed{ String::Build("Unable to write to ", filename.FromUtf8()) } });
				return;
			}
			if (tpt_lua_dostring(L, ByteString::Build("dofile('", filename, "')")))
			{
				complete({ Status::RunFailed{ LuaGetError() } });
				return;
			}
			complete({ Status::Ok{} });
		}
		catch (const http::RequestError &ex)
		{
			complete({ Status::GetFailed{ ByteString(ex.what()).FromUtf8() } });
		}
	}
}

static int flog(lua_State *L)
{
	auto *lsi = GetLSI();
	int args = lua_gettop(L);
	String text;
	bool hasText = false;
	for(int i = 1; i <= args; i++)
	{
		LuaToLoggableString(L, -1);
		if (hasText)
		{
			text = tpt_lua_optString(L, -1, "") + ", " + text;
		}
		else
		{
			text = tpt_lua_optString(L, -1, "");
			hasText = true;
		}
		lua_pop(L, 2);
	}
	if (lsi->currentCommand)
	{
		auto lastError = lsi->GetLastError();
		if (lsi->luacon_hasLastError)
			lastError += "; ";
		lastError += text;
		lsi->SetLastError(lastError);
		lsi->luacon_hasLastError = true;
	}
	else
		lsi->Log(CommandInterface::LogNotice, text);
	return 0;
}

static int screenshot(lua_State *L)
{
	int captureUI = luaL_optint(L, 1, 0);
	int fileType = luaL_optint(L, 2, 0);

	auto *lsi = GetLSI();
	ByteString filename = lsi->gameController->TakeScreenshot(captureUI, fileType);
	if (filename.size())
	{
		tpt_lua_pushByteString(L, filename);
		return 1;
	}
	return 0;
}

static int record(lua_State *L)
{
	if (!lua_isboolean(L, -1))
		return luaL_typerror(L, 1, lua_typename(L, LUA_TBOOLEAN));
	bool record = lua_toboolean(L, -1);
	auto *lsi = GetLSI();
	int recordingFolder = lsi->gameController->Record(record);
	lua_pushinteger(L, recordingFolder);
	return 1;
}

static int compatChunk(lua_State *L)
{
	auto data = compat_lua.AsCharSpan();
	lua_pushlstring(L, data.data(), data.size());
	return 1;
}
static int debug(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushinteger(L, lsi->gameController->GetDebugFlags());
		return 1;
	}
	int debugFlags = luaL_optint(L, 1, 0);
	lsi->gameController->SetDebugFlags(debugFlags);
	return 0;
}

static int fpsCap(lua_State *L)
{
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		auto fpsLimit = ui::Engine::Ref().GetFpsLimit();
		if (std::holds_alternative<FpsLimitVsync>(fpsLimit))
		{
			lua_pushliteral(L, "vsync");
		}
		else if (std::holds_alternative<FpsLimitNone>(fpsLimit))
		{
			lua_pushnumber(L, 2);
		}
		else
		{
			lua_pushnumber(L, std::get<FpsLimitExplicit>(fpsLimit).value);
		}
		return 1;
	}
	if (lua_isstring(L, 1) && byteStringEqualsLiteral(tpt_lua_toByteString(L, 1), "vsync"))
	{
		ui::Engine::Ref().SetFpsLimit(FpsLimitVsync{});
		return 0;
	}
	float fpscap = luaL_checknumber(L, 1);
	if (fpscap < 2)
	{
		return luaL_error(L, "fps cap too small");
	}
	if (fpscap == 2)
	{
		ui::Engine::Ref().SetFpsLimit(FpsLimitNone{});
		return 0;
	}
	ui::Engine::Ref().SetFpsLimit(FpsLimitExplicit{ fpscap });
	return 0;
}

static int drawCap(lua_State *L)
{
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		auto drawLimit = ui::Engine::Ref().GetDrawingFrequencyLimit();
		if (std::holds_alternative<DrawLimitDisplay>(drawLimit))
		{
			lua_pushliteral(L, "display");
		}
		else if (std::holds_alternative<DrawLimitNone>(drawLimit))
		{
			lua_pushinteger(L, 0);
		}
		else
		{
			lua_pushinteger(L, std::get<DrawLimitExplicit>(drawLimit).value);
		}
		return 1;
	}
	if (lua_isstring(L, 1) && byteStringEqualsLiteral(tpt_lua_toByteString(L, 1), "display"))
	{
		ui::Engine::Ref().SetDrawingFrequencyLimit(DrawLimitDisplay{});
		return 0;
	}
	int drawcap = luaL_checkinteger(L, 1);
	if(drawcap < 0)
		return luaL_error(L, "draw cap too small");
	if (drawcap == 0)
	{
		ui::Engine::Ref().SetDrawingFrequencyLimit(DrawLimitNone{});
		return 0;
	}
	ui::Engine::Ref().SetDrawingFrequencyLimit(DrawLimitExplicit{ drawcap });
	return 0;
}

void LuaMisc::Open(lua_State *L)
{
	static const luaL_Reg reg[] = {
#define LFUNC(v) { #v, v }
		LFUNC(getUserName),
		LFUNC(installScriptManager),
		LFUNC(screenshot),
		LFUNC(record),
		LFUNC(debug),
		LFUNC(fpsCap),
		LFUNC(drawCap),
		LFUNC(compatChunk),
#undef LFUNC
		{ "log", flog },
		{ nullptr, nullptr }
	};
	lua_newtable(L);
	luaL_register(L, nullptr, reg);
#define LCONST(v) lua_pushinteger(L, int(v)); lua_setfield(L, -2, #v)
	LCONST(DEBUG_PARTS);
	LCONST(DEBUG_ELEMENTPOP);
	LCONST(DEBUG_LINES);
	LCONST(DEBUG_PARTICLE);
	LCONST(DEBUG_SURFNORM);
	LCONST(DEBUG_SIMHUD);
	LCONST(DEBUG_RENHUD);
#undef LCONST
	{
		lua_newtable(L);
#define LCONSTAS(k, v) lua_pushinteger(L, int(v)); lua_setfield(L, -2, k)
		LCONSTAS("major"        , DISPLAY_VERSION[0]);
		LCONSTAS("minor"        , DISPLAY_VERSION[1]);
		LCONSTAS("build"        , APP_VERSION.build);
		LCONSTAS("upstreamMajor", UPSTREAM_VERSION.displayVersion[0]);
		LCONSTAS("upstreamMinor", UPSTREAM_VERSION.displayVersion[1]);
		LCONSTAS("upstreamBuild", UPSTREAM_VERSION.build);
		LCONSTAS("modid"        , MOD_ID);
#undef LCONSTAS
		lua_pushboolean(L, SNAPSHOT);
		lua_setfield(L, -2, "snapshot");
		lua_pushboolean(L, BETA);
		lua_setfield(L, -2, "beta");
		auto vcsTag = ByteString(VCS_TAG);
		if (vcsTag.size())
		{
			tpt_lua_pushByteString(L, vcsTag);
			lua_setfield(L, -2, "vcstag");
		}
		lua_pushstring(L, PACKAGE_MODE);
		lua_setfield(L, -2, "packagemode");
		lua_setfield(L, -2, "version");
	}
	lua_setglobal(L, "tpt");
}
