#ifndef LUACONSOLEH
#define LUACONSOLEH
#ifdef LUA_R_INCL
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#else
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
#endif
#include <defines.h>

void luacon_open();
int luacon_step(int mx, int my, int mb, int mbq, char key);
int luacon_keypress(char key);
int luacon_eval(char *command);
char *luacon_geterror();
void luacon_close();
int process_command_lua(pixel *vid_buf, char *console, char *console_error);

int getPartIndex_curIdx;

//TPT Interface
int luatpt_test(lua_State* l);
int luatpt_drawtext(lua_State* l);
int luatpt_create(lua_State* l);
int luatpt_setpause(lua_State* l);
int luatpt_togglepause(lua_State* l);
int luatpt_setconsole(lua_State* l);
int luatpt_log(lua_State* l);
int luatpt_set_pressure(lua_State* l);
int luatpt_set_gravity(lua_State* l);
int luatpt_reset_gravity_field(lua_State* l);
int luatpt_reset_velocity(lua_State* l);
int luatpt_reset_spark(lua_State* l);
int luatpt_set_property(lua_State* l);
int luatpt_get_property(lua_State* l);
int luatpt_drawpixel(lua_State* l);
int luatpt_drawrect(lua_State* l);
int luatpt_fillrect(lua_State* l);
int luatpt_textwidth(lua_State* l);
int luatpt_get_name(lua_State* l);
int luatpt_set_shortcuts(lua_State* l);
int luatpt_delete(lua_State* l);
int luatpt_register_step(lua_State* l);
int luatpt_unregister_step(lua_State* l);
int luatpt_input(lua_State* l);
int luatpt_message_box(lua_State* l);
int luatpt_get_numOfParts(lua_State* l);
int luatpt_start_getPartIndex(lua_State* l);
int luatpt_getPartIndex(lua_State* l);
int luatpt_next_getPartIndex(lua_State* l);
int luatpt_hud(lua_State* l);
int luatpt_gravity(lua_State* l);
int luatpt_airheat(lua_State* l);
int luatpt_active_menu(lua_State* l);
int luatpt_decorations_enable(lua_State* l);
int luatpt_cmode_set(lua_State* l);
#endif
