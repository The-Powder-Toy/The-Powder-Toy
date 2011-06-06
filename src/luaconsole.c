#ifdef LUACONSOLE
#include <powder.h>
#include <console.h>
#include <luaconsole.h>

lua_State *l;
int step_functions[6] = {0, 0, 0, 0, 0, 0};
void luacon_open(){
	const static struct luaL_reg tptluaapi [] = {
		{"test", &luatpt_test},
		{"drawtext", &luatpt_drawtext},
		{"create", &luatpt_create},
		{"set_pause", &luatpt_setpause},
		{"toggle_pause", &luatpt_togglepause},
		{"set_console", &luatpt_setconsole},
		{"log", &luatpt_log},
		{"set_pressure", &luatpt_set_pressure},
		{"set_gravity", &luatpt_set_gravity},
		{"reset_gravity_field", &luatpt_reset_gravity_field},
		{"reset_velocity", &luatpt_reset_velocity},
		{"reset_spark", &luatpt_reset_spark},
		{"set_property", &luatpt_set_property},
		{"get_property", &luatpt_get_property},
		{"drawpixel", &luatpt_drawpixel},
		{"drawrect", &luatpt_drawrect},
		{"fillrect", &luatpt_fillrect},
		{"textwidth", &luatpt_textwidth},
		{"get_name", &luatpt_get_name},
		{"set_shortcuts", &luatpt_set_shortcuts},
		{"delete", &luatpt_delete},
		{"register_step", &luatpt_register_step},
		{"unregister_step", &luatpt_unregister_step},
		{NULL,NULL}
	};

	l = lua_open();
	luaL_openlibs(l);
	luaL_openlib(l, "tpt", tptluaapi, 0);
}
int luacon_step(int mx, int my, int mb, int mbq, char key){
	int tempret = 0, tempb, i;
	if(step_functions[0]){
		//Set mouse globals
		lua_pushinteger(l, mbq);
		lua_pushinteger(l, mb);
		lua_pushinteger(l, my);
		lua_pushinteger(l, mx);
		lua_setfield(l, LUA_GLOBALSINDEX, "mousex");
		lua_setfield(l, LUA_GLOBALSINDEX, "mousey");
		lua_setfield(l, LUA_GLOBALSINDEX, "mouseb");
		lua_setfield(l, LUA_GLOBALSINDEX, "mousebq");
		for(i = 0; i<6; i++){
			if(step_functions[i]){
				lua_rawgeti(l, LUA_REGISTRYINDEX, step_functions[i]);
				lua_call(l, 0, 1);
				if(lua_isboolean(l, -1)){
					tempb = lua_toboolean(l, -1);
					lua_pop(l, 1);
					if(tempb){	//Mouse click has been handled, set the global for future calls
						lua_pushinteger(l, mb);
						lua_setfield(l, LUA_GLOBALSINDEX, "mouseb");
					}
					tempret |= tempb;
				}
			}
		}
		return tempret;
	}
	return 0;
}
int luacon_eval(char *command){
	return luaL_dostring (l, command);
}
char *luacon_geterror(){
	char *error = lua_tostring(l, -1);
	if(error==NULL || !error[0]){
		error = "failed to execute";
	}
	return error;
}
void luacon_close(){
	lua_close(l);
}
int process_command_lua(pixel *vid_buf, char *console, char *console_error)
{
	int commandret;
	char * tmp_error;
	char console2[15];
	char console3[15];
	char console4[15];
	char console5[15];
	//sprintf(console_error, "%s", console);
	if (console && strcmp(console, "")!=0 && strncmp(console, " ", 1)!=0)
	{
		sscanf(console,"%14s %14s %14s %14s", console2, console3, console4, console5);
		if (strcmp(console2, "quit")==0)
		{
			return -1;
		}
		else if(strncmp(console, "!", 1)==0)
		{
			return process_command_old(vid_buf, console+1, console_error);
		}
		else
		{
			commandret = luacon_eval(console);
			if (commandret){
				tmp_error = luacon_geterror();
				strncpy(console_error, tmp_error, 254);
				printf("%s\n", tmp_error);
			}
		}
	}
	return 1;
}
//Being TPT interface methods:
int luatpt_test(lua_State* l)
{
    int testint = 0;
	testint = luaL_optint(l, 1, 0);
	printf("Test successful, got %d\n", testint);
	return 0;
}
int luatpt_drawtext(lua_State* l)
{
    char *string;
	int textx, texty, textred, textgreen, textblue, textalpha;
	textx = luaL_optint(l, 1, 0);
	texty = luaL_optint(l, 2, 0);
	string = luaL_optstring(l, 3, "");
	textred = luaL_optint(l, 4, 255);
	textgreen = luaL_optint(l, 5, 255);
	textblue = luaL_optint(l, 6, 255);
	textalpha = luaL_optint(l, 7, 255);
	if(vid_buf!=NULL){
		drawtext(vid_buf, textx, texty, string, textred, textgreen, textblue, textalpha);
		return 0;
	}
	return -1;
}

int luatpt_create(lua_State* l)
{
	int x, y, retid, t = 0;
	char * name;
	x = abs(luaL_optint(l, 1, 0));
	y = abs(luaL_optint(l, 2, 0));
	if(x < XRES && y < YRES){
		if(lua_isnumber(l, 3)){
			t = luaL_optint(l, 3, 0);
			if(t >= PT_NUM)
				return -1;
		} else {
			name = luaL_optstring(l, 3, "dust");
			if (name[0]!=0)
				console_parse_type(name, &t, console_error);
		}
		retid = create_part(-1, x, y, t);
		if(retid==-1)
			return -1;
		lua_pushinteger(l, retid);
		return 1;
	}
	return -1;
}

int luatpt_setpause(lua_State* l)
{
	int pausestate;
	pausestate = luaL_optint(l, 1, 0);
	sys_pause = (pausestate==0?0:1);
	return 0;
}

int luatpt_togglepause(lua_State* l)
{
	sys_pause=!sys_pause;
	return 0;
}

int luatpt_setconsole(lua_State* l)
{
	int consolestate;
	consolestate = luaL_optint(l, 1, 0);
	console_mode = (consolestate==0?0:1);
	return 0;
}

int luatpt_log(lua_State* l)
{
	char *buffer;
	buffer = luaL_optstring(l, 1, "");
	strcpy(console_error, buffer);
	return 0;
}

int luatpt_set_pressure(lua_State* l)
{
	int nx, ny;
	int x1, y1, width, height;
	float value;
	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	value = (float)luaL_optint(l, 5, 0.0f);
	if(value > 256.0f)
		value = 256.0f;
	else if(value < -256.0f)
		value = -256.0f;

	if(x1 > (XRES/CELL)-1)
		x1 = (XRES/CELL)-1;
	if(y1 > (YRES/CELL)-1)
		y1 = (YRES/CELL)-1;
	if(x1+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x1;
	if(y1+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y1;
	for (nx = x1; nx<x1+width; nx++)
		for (ny = y1; ny<y1+height; ny++)
		{
			pv[ny][nx] = value;
		}
	return 0;
}

int luatpt_set_gravity(lua_State* l)
{
	int nx, ny;
	int x1, y1, width, height;
	float value;
	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	value = (float)luaL_optint(l, 5, 0.0f);
	if(value > 256.0f)
		value = 256.0f;
	else if(value < -256.0f)
		value = -256.0f;

	if(x1 > (XRES/CELL)-1)
		x1 = (XRES/CELL)-1;
	if(y1 > (YRES/CELL)-1)
		y1 = (YRES/CELL)-1;
	if(x1+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x1;
	if(y1+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y1;
	for (nx = x1; nx<x1+width; nx++)
		for (ny = y1; ny<y1+height; ny++)
		{
			gravmap[ny][nx] = value;
		}
	return 0;
}

int luatpt_reset_gravity_field(lua_State* l)
{
	int nx, ny;
	int x1, y1, width, height;
	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	if(x1 > (XRES/CELL)-1)
		x1 = (XRES/CELL)-1;
	if(y1 > (YRES/CELL)-1)
		y1 = (YRES/CELL)-1;
	if(x1+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x1;
	if(y1+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y1;
	for (nx = x1; nx<x1+width; nx++)
		for (ny = y1; ny<y1+height; ny++)
		{
			gravx[ny][nx] = 0;
			gravy[ny][nx] = 0;
		}
	return 0;
}

int luatpt_reset_velocity(lua_State* l)
{
	int nx, ny;
	int x1, y1, width, height;
	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	if(x1 > (XRES/CELL)-1)
		x1 = (XRES/CELL)-1;
	if(y1 > (YRES/CELL)-1)
		y1 = (YRES/CELL)-1;
	if(x1+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x1;
	if(y1+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y1;
	for (nx = x1; nx<x1+width; nx++)
		for (ny = y1; ny<y1+height; ny++)
		{
			vx[ny][nx] = 0;
			vy[ny][nx] = 0;
		}
	return 0;
}

int luatpt_reset_spark(lua_State* l)
{
	int i;
	for (i=0; i<NPART; i++)
	{
		if (parts[i].type==PT_SPRK)
		{
			parts[i].type = parts[i].ctype;
			parts[i].life = 4;
		}
	}
	return 0;
}

int luatpt_set_property(lua_State* l)
{
	char *prop, *name;
	int i, x, y, w, h, t, format, nx, ny, partsel = 0, acount;
	float f;
	size_t offset;
	acount = lua_gettop(l);
	prop = luaL_optstring(l, 1, "");
	if(lua_isnumber(l, 3))
		i = luaL_optint(l, 3, -1);
	else
		i = -1;
	if(lua_isnumber(l, 4))
		y = luaL_optint(l, 4, -1);
	else
		y = -1;
	if(lua_isnumber(l, 5))
		w = luaL_optint(l, 5, -1);
	else
		w = -1;
	if(lua_isnumber(l, 6))
		h = luaL_optint(l, 6, -1);
	else
		h = -1;
	if (strcmp(prop,"type")==0){
		offset = offsetof(particle, type);
		format = 3;
	} else if (strcmp(prop,"life")==0){
		offset = offsetof(particle, life);
		format = 1;
	} else if (strcmp(prop,"ctype")==0){
		offset = offsetof(particle, ctype);
		format = 3;
	} else if (strcmp(prop,"temp")==0){
		offset = offsetof(particle, temp);
		format = 2;
	} else if (strcmp(prop,"tmp")==0){
		offset = offsetof(particle, tmp);
		format = 1;
	} else if (strcmp(prop,"vy")==0){
		offset = offsetof(particle, vy);
		format = 2;
	} else if (strcmp(prop,"vx")==0){
		offset = offsetof(particle, vx);
		format = 2;
	} else if (strcmp(prop,"x")==0){
		offset = offsetof(particle, x);
		format = 2;
	} else if (strcmp(prop,"y")==0){
		offset = offsetof(particle, y);
		format = 2;
	} else {
		lua_pushstring(l, "invalid property");
		lua_error(l);
	}
	if(acount>2){
		if(!lua_isnumber(l, acount) && lua_isstring(l, acount)){
			name = luaL_optstring(l, acount, "none");
			if (name[0]!=0)
				console_parse_type(name, &partsel, console_error);
		}
	}
	if(lua_isnumber(l, 2)){
		if(format==2){
			f = luaL_optnumber(l, 2, 0);
		} else {
			t = luaL_optint(l, 2, 0);
		}
		if(t >= PT_NUM && format == 3)
			return -1;
	} else {
		name = luaL_optstring(l, 2, "dust");
		if (name[0]!=0)
			console_parse_type(name, &t, console_error);
	}
	if(i == -1 || (w != -1 && h != -1)){
		// Got a region
		if(i == -1){
			i = 0;
			y = 0;
			w = XRES;
			h = YRES;
		}
		x = i;
		for (nx = x; nx<x+w; nx++)
			for (ny = y; ny<y+h; ny++){
				i = pmap[ny][nx]>>PS;
				if (i < 0 || i >= NPART || (partsel && partsel != parts[i].type))
					continue;
				if(format==2){
					*((float*)(((void*)&parts[i])+offset)) = f;
				} else {
					*((int*)(((void*)&parts[i])+offset)) = t;
				}
			}
	} else {
		// Got coords or particle index
		if(i != -1 && y != -1){
			i = pmap[y][i]>>PS;
		}
		if (i < 0 || i >= NPART || (partsel && partsel != parts[i].type))
			return -1;
		if(format==2){
			*((float*)(((void*)&parts[i])+offset)) = f;
		} else {
			*((int*)(((void*)&parts[i])+offset)) = t;
		}
	}
	return -1;
}

int luatpt_get_property(lua_State* l)
{
	int i, y;
	char *prop;
	prop = luaL_optstring(l, 1, "");
	i = luaL_optint(l, 2, 0);
	y = luaL_optint(l, 3, -1);
	if(y!=-1 && y < YRES && y > 0 && i < XRES && i > 0){
		i = pmap[y][i]>>PS;
	}
	if (i < 0 || i >= NPART)
		return -1;
	if (parts[i].type)
	{
		if (strcmp(prop,"type")==0){
			lua_pushinteger(l, parts[i].type);
			return 1;
		}
		if (strcmp(prop,"life")==0){
			lua_pushinteger(l, parts[i].life);
			return 1;
		}
		if (strcmp(prop,"ctype")==0){
			lua_pushinteger(l, parts[i].ctype);
			return 1;
		}
		if (strcmp(prop,"temp")==0){
			lua_pushnumber(l, parts[i].temp);
			return 1;
		}
		if (strcmp(prop,"tmp")==0){
			lua_pushinteger(l, parts[i].tmp);
			return 1;
		}
		if (strcmp(prop,"vy")==0){
			lua_pushnumber(l, (double)parts[i].vy);
			return 1;
		}
		if (strcmp(prop,"vx")==0){
			lua_pushnumber(l, (double)parts[i].vx);
			return 1;
		}
		if (strcmp(prop,"x")==0){
			lua_pushnumber(l, parts[i].x);
			return 1;
		}
		if (strcmp(prop,"y")==0){
			lua_pushnumber(l, parts[i].y);
			return 1;
		}
	}

	return -1;
}

int luatpt_drawpixel(lua_State* l)
{
	int x, y, r, g, b, a;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	r = luaL_optint(l, 3, 255);
	g = luaL_optint(l, 4, 255);
	b = luaL_optint(l, 5, 255);
	a = luaL_optint(l, 6, 255);

	if(x > XRES || y > YRES || x < 0 || y < 0)
		return -1;
	if (vid_buf!=NULL)
	{
		drawpixel(vid_buf, x, y, r, g, b, a);
		return 0;
	}
	return -1;

}

int luatpt_drawrect(lua_State* l)
{
	int x, y, w, h, r, g, b, a;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	w = luaL_optint(l, 3, 10);
	h = luaL_optint(l, 4, 10);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	if(x > XRES || y > YRES || x < 0 || y < 0)
		return -1;
	if (vid_buf!=NULL)
	{
		drawrect(vid_buf, x, y, w, h, r, g, b, a);
		return 0;
	}
	return -1;
}

int luatpt_fillrect(lua_State* l)
{
	int x,y,w,h,r,g,b,a;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	w = luaL_optint(l, 3, 10);
	h = luaL_optint(l, 4, 10);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	if(x > XRES || y > YRES || x < 0 || y < 0)
		return -1;
	if (vid_buf!=NULL)
	{
		fillrect(vid_buf, x, y, w, h, r, g, b, a);
		return 0;
	}
	return -1;
}

int luatpt_textwidth(lua_State* l)
{
	char * string;
	int strwidth = 0;
	string = luaL_optstring(l, 1, "");
	strwidth = textwidth(string);
	lua_pushinteger(l, strwidth);
	return 1;
}

int luatpt_get_name(lua_State* l)
{
	if (svf_login){
		lua_pushstring(l, "");
		return 1;
	}
	lua_pushstring(l, "");
	return 1;
}

int luatpt_set_shortcuts(lua_State* l)
{
	int state;
	state = luaL_optint(l, 1, 0);
	sys_shortcuts = (state==0?0:1);
	return 0;
}

int luatpt_delete(lua_State* l)
{
	int arg1, arg2;
	arg1 = abs(luaL_optint(l, 1, 0));
	arg2 = luaL_optint(l, 2, -1);
	if(arg2 == -1 && arg1 < NPART){
		kill_part(arg1);
		return 0;
	}
	arg2 = abs(arg2);
	if(arg2 < YRES && arg1 < XRES){
		delete_part(arg1, arg2);
		return 0;
	}
	return -1;
}

int luatpt_register_step(lua_State* l)
{
	int ref, i;
	if(lua_isfunction(l, 1)){
		for(i = 0; i<6; i++){
			if(!step_functions[i]){
				ref = luaL_ref(l, LUA_REGISTRYINDEX);
				step_functions[i] = ref;
				break;
			} else { //Supposed to prevent the registration of 2 functions, but this isn't working TODO: FIX!
				lua_rawgeti(l, LUA_REGISTRYINDEX, step_functions[i]);
				if(lua_equal(l, 1, lua_gettop(l))){
					lua_pushstring(l, "function already registered");
					lua_error(l);
					return -1;
				}
			}
		}
	}
	return 0;
}
int luatpt_unregister_step(lua_State* l)
{
	//step_function = luaL_optstring(l, 1, "");
	return 0;
}
#endif
