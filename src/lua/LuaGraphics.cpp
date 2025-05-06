#include "LuaScriptInterface.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"

void NonGraphicsContext::Die()
{
	luaL_error(GetLSI()->L, "this functionality is restricted to graphics events");
}

void NonGraphicsContext::BlendPixel(Vec2<int>, RGBA)
{
	Die();
}

Vec2<int> NonGraphicsContext::BlendText(Vec2<int>, const String &, RGBA)
{
	Die();
	return { 0, 0 };
}

void NonGraphicsContext::DrawLine(Vec2<int>, Vec2<int>, RGB)
{
	Die();
}

void NonGraphicsContext::BlendLine(Vec2<int>, Vec2<int>, RGBA)
{
	Die();
}

void NonGraphicsContext::DrawRect(Rect<int>, RGB)
{
	Die();
}

void NonGraphicsContext::BlendRect(Rect<int>, RGBA)
{
	Die();
}

void NonGraphicsContext::DrawFilledRect(Rect<int>, RGB)
{
	Die();
}

void NonGraphicsContext::BlendFilledRect(Rect<int>, RGBA)
{
	Die();
}

void NonGraphicsContext::BlendEllipse(Vec2<int>, Vec2<int>, RGBA)
{
	Die();
}

void NonGraphicsContext::BlendFilledEllipse(Vec2<int>, Vec2<int>, RGBA)
{
	Die();
}

static int32_t int32Truncate(double n)
{
	if (n >= 0x1p31)
	{
		n -= 0x1p32;
	}
	return int32_t(n);
}

static int textSize(lua_State *L)
{
	auto text = tpt_lua_optString(L, 1, "");
	auto size = Graphics::TextSize(text);
	lua_pushinteger(L, size.X);
	lua_pushinteger(L, size.Y);
	return 2;
}

static int drawText(lua_State *L)
{
	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);
	auto text = tpt_lua_optString(L, 3, "");
	int r = luaL_optint(L, 4, 255);
	int g = luaL_optint(L, 5, 255);
	int b = luaL_optint(L, 6, 255);
	int a = luaL_optint(L, 7, 255);

	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;

	std::visit([x, y, r, g, b, a, &text](auto p) {
		p->BlendText({ x, y }, text, RGBA(r, g, b, a));
	}, GetLSI()->GetGraphics());
	return 0;
}

static int drawPixel(lua_State *L)
{
	auto x = luaL_optint(L, 1, 0);
	auto y = luaL_optint(L, 2, 0);
	auto r = luaL_optint(L, 3, 255);
	auto g = luaL_optint(L, 4, 255);
	auto b = luaL_optint(L, 5, 255);
	auto a = luaL_optint(L, 6, 255);
	if      (r < 0  ) r = 0  ;
	else if (r > 255) r = 255;
	if      (g < 0  ) g = 0  ;
	else if (g > 255) g = 255;
	if      (b < 0  ) b = 0  ;
	else if (b > 255) b = 255;
	if      (a < 0  ) a = 0  ;
	else if (a > 255) a = 255;
	std::visit([x, y, r, g, b, a](auto p) {
		p->BlendPixel({ x, y }, RGBA(r, g, b, a));
	}, GetLSI()->GetGraphics());
	return 0;
}

static int drawLine(lua_State *L)
{
	int x1 = lua_tointeger(L, 1);
	int y1 = lua_tointeger(L, 2);
	int x2 = lua_tointeger(L, 3);
	int y2 = lua_tointeger(L, 4);
	int r = luaL_optint(L, 5, 255);
	int g = luaL_optint(L, 6, 255);
	int b = luaL_optint(L, 7, 255);
	int a = luaL_optint(L, 8, 255);

	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;

	std::visit([x1, y1, x2, y2, r, g, b, a](auto p) {
		if (a == 255)
		{
			p->DrawLine({ x1, y1 }, { x2, y2 }, RGB(r, g, b));
		}
		else
		{
			p->BlendLine({ x1, y1 }, { x2, y2 }, RGBA(r, g, b, a));
		}
	}, GetLSI()->GetGraphics());
	return 0;
}

static int drawRect(lua_State *L)
{
	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);
	int width = lua_tointeger(L, 3);
	int height = lua_tointeger(L, 4);
	int r = luaL_optint(L, 5, 255);
	int g = luaL_optint(L, 6, 255);
	int b = luaL_optint(L, 7, 255);
	int a = luaL_optint(L, 8, 255);

	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;

	std::visit([x, y, width, height, r, g, b, a](auto p) {
		if (a == 255)
		{
			p->DrawRect(RectSized(Vec2{ x, y }, Vec2{ width, height }), RGB(r, g, b));
		}
		else
		{
			p->BlendRect(RectSized(Vec2{ x, y }, Vec2{ width, height }), RGBA(r, g, b, a));
		}
	}, GetLSI()->GetGraphics());
	return 0;
}

static int fillRect(lua_State *L)
{
	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);
	int width = lua_tointeger(L, 3);
	int height = lua_tointeger(L, 4);
	int r = luaL_optint(L, 5, 255);
	int g = luaL_optint(L, 6, 255);
	int b = luaL_optint(L, 7, 255);
	int a = luaL_optint(L, 8, 255);

	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;

	std::visit([x, y, width, height, r, g, b, a](auto p) {
		if (a == 255)
		{
			p->DrawFilledRect(RectSized(Vec2{ x, y }, Vec2{ width, height }), RGB(r, g, b));
		}
		else
		{
			p->BlendFilledRect(RectSized(Vec2{ x, y }, Vec2{ width, height }), RGBA(r, g, b, a));
		}
	}, GetLSI()->GetGraphics());
	return 0;
}

static int drawCircle(lua_State *L)
{
	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);
	int rx = lua_tointeger(L, 3);
	int ry = lua_tointeger(L, 4);
	int r = luaL_optint(L, 5, 255);
	int g = luaL_optint(L, 6, 255);
	int b = luaL_optint(L, 7, 255);
	int a = luaL_optint(L, 8, 255);

	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;

	std::visit([x, y, rx, ry, r, g, b, a](auto p) {
		p->BlendEllipse({ x, y }, { abs(rx), abs(ry) }, RGBA(r, g, b, a));
	}, GetLSI()->GetGraphics());
	return 0;
}

static int fillCircle(lua_State *L)
{
	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);
	int rx = lua_tointeger(L, 3);
	int ry = lua_tointeger(L, 4);
	int r = luaL_optint(L, 5, 255);
	int g = luaL_optint(L, 6, 255);
	int b = luaL_optint(L, 7, 255);
	int a = luaL_optint(L, 8, 255);

	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;

	std::visit([x, y, rx, ry, r, g, b, a](auto p) {
		p->BlendFilledEllipse({ x, y }, { abs(rx), abs(ry) }, RGBA(r, g, b, a));
	}, GetLSI()->GetGraphics());
	return 0;
}

static int getColors(lua_State *L)
{
	unsigned int color = int32Truncate(lua_tonumber(L, 1));

	int a = color >> 24;
	int r = (color >> 16)&0xFF;
	int g = (color >> 8)&0xFF;
	int b = color&0xFF;

	lua_pushinteger(L, r);
	lua_pushinteger(L, g);
	lua_pushinteger(L, b);
	lua_pushinteger(L, a);
	return 4;
}

static int getHexColor(lua_State *L)
{
	int r = lua_tointeger(L, 1);
	int g = lua_tointeger(L, 2);
	int b = lua_tointeger(L, 3);
	int a = 0;
	if (lua_gettop(L) >= 4)
		a = lua_tointeger(L, 4);
	unsigned int color = (a<<24) + (r<<16) + (g<<8) + b;

	lua_pushinteger(L, color);
	return 1;
}

static int setClipRect(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lsi->eventTraits & eventTraitSimGraphics)
	{
		return luaL_error(L, "simulation graphics do not support clip rects");
	}
	if (!(lsi->eventTraits & eventTraitInterfaceGraphics))
	{
		NonGraphicsContext::Die();
	}
	int x = luaL_optinteger(L, 1, 0);
	int y = luaL_optinteger(L, 2, 0);
	int w = luaL_optinteger(L, 3, WINDOWW);
	int h = luaL_optinteger(L, 4, WINDOWH);
	auto rect = RectSized(Vec2(x, y), Vec2(w, h));
	lsi->g->SwapClipRect(rect);
	lua_pushinteger(L, rect.pos.X);
	lua_pushinteger(L, rect.pos.Y);
	lua_pushinteger(L, rect.size.X);
	lua_pushinteger(L, rect.size.Y);
	return 4;
}

void LuaGraphics::Open(lua_State *L)
{
	static const luaL_Reg reg[] = {
#define LFUNC(v) { #v, v }
		LFUNC(textSize),
		LFUNC(drawText),
		LFUNC(drawPixel),
		LFUNC(drawLine),
		LFUNC(drawRect),
		LFUNC(fillRect),
		LFUNC(drawCircle),
		LFUNC(fillCircle),
		LFUNC(getColors),
		LFUNC(getHexColor),
		LFUNC(setClipRect),
#undef LFUNC
		{ nullptr, nullptr }
	};
	lua_newtable(L);
	luaL_register(L, nullptr, reg);
#define LCONSTAS(k, v) lua_pushinteger(L, int(v)); lua_setfield(L, -2, k)
	LCONSTAS("WIDTH",  WINDOWW);
	LCONSTAS("HEIGHT", WINDOWH);
#undef LCONSTAS
	lua_pushvalue(L, -1);
	lua_setglobal(L, "graphics");
	lua_setglobal(L, "gfx");
}
