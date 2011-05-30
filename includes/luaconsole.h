#ifndef LUACONSOLEH
#define LUACONSOLEH
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
#include <defines.h>

void luacon_open();
int luacon_step();
int luacon_keypress(char key);
int luacon_eval(char *command);
void luacon_close();
int process_command_lua(pixel *vid_buf, char *console, char *console_error);

//TPT Interface
int luatpt_test(lua_State* l);
int luatpt_drawtext(lua_State* l);
#endif
