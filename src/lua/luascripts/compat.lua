bz2.compressLimit       = bz2.COMPRESS_LIMIT
bz2.compressNomem       = bz2.COMPRESS_NOMEM
bz2.decompressBad       = bz2.DECOMPRESS_BAD
bz2.decompressEof       = bz2.DECOMPRESS_EOF
bz2.decompressLimit     = bz2.DECOMPRESS_LIMIT
bz2.decompressNomem     = bz2.DECOMPRESS_NOMEM
bz2.decompressType      = bz2.DECOMPRESS_TYPE
elem.FLAG_MOVABLE       = sim.FLAG_MOVABLE
elem.FLAG_PHOTDECO      = sim.FLAG_PHOTDECO
elem.FLAG_SKIPMOVE      = sim.FLAG_SKIPMOVE
elem.FLAG_STAGNANT      = sim.FLAG_STAGNANT
elem.PROP_DRAWONCTYPE   = 0
elem.ST_GAS             = 0
elem.ST_LIQUID          = 0
elem.ST_NONE            = 0
elem.ST_SOLID           = 0
event.aftersimdraw      = event.AFTERSIMDRAW
event.aftersim          = event.AFTERSIM
event.beforesimdraw     = event.BEFORESIMDRAW
event.beforesim         = event.BEFORESIM
event.blur              = event.BLUR
event.close             = event.CLOSE
event.getmodifiers      = event.getModifiers
event.keypress          = event.KEYPRESS
event.keyrelease        = event.KEYRELEASE
event.mousedown         = event.MOUSEDOWN
event.mousemove         = event.MOUSEMOVE
event.mouseup           = event.MOUSEUP
event.mousewheel        = event.MOUSEWHEEL
event.textediting       = event.TEXTEDITING
event.textinput         = event.TEXTINPUT
event.tick              = event.TICK
ren.colourMode          = ren.colorMode
sim.can_move            = sim.canMove
sim.decoColour          = sim.decoColor
sim.framerender         = sim.frameRender
sim.gspeed              = sim.golSpeedRatio
sim.neighbours          = sim.neighbors
sim.partNeighbours      = sim.partNeighbors
sim.randomseed          = sim.randomSeed
sim.waterEqualisation   = sim.waterEqualization
tpt.active_menu         = ui.activeMenu
tpt.display_mode        = ren.useDisplayPreset
tpt.get_clipboard       = plat.clipboardCopy
tpt.get_name            = tpt.getUserName
tpt.menu_enabled        = ui.menuEnabled
tpt.num_menus           = ui.numMenus
tpt.perfectCircleBrush  = ui.perfectCircleBrush
tpt.reset_gravity_field = sim.resetGravityField
tpt.reset_spark         = sim.resetSpark
tpt.reset_velocity      = sim.resetVelocity
tpt.set_clipboard       = plat.clipboardPaste
tpt.setdrawcap          = tpt.drawCap
tpt.setfpscap           = tpt.fpsCap
ui.MOUSE_UP_BLUR        = ui.MOUSEUP_BLUR
ui.MOUSE_UP_DRAW_END    = ui.MOUSEUP_DRAWEND
ui.MOUSE_UP_NORMAL      = ui.MOUSEUP_NORMAL
if socket then
	socket.gettime = socket.getTime
end

local function fake_boolean_wrapper(func, true_is_1)
	return function(param)
		if not param then
			return func() and 1 or 0
		end
		local enable
		if true_is_1 then
			enable = param == 1
		else
			enable = param ~= 0
		end
		func(enable)
	end
end
tpt.set_console        = fake_boolean_wrapper(ui.console          , true )
tpt.ambient_heat       = fake_boolean_wrapper(sim.ambientHeatSim  , true )
tpt.heat               = fake_boolean_wrapper(sim.heatSim         , false)
tpt.set_pause          = fake_boolean_wrapper(sim.paused          , false)
tpt.decorations_enable = fake_boolean_wrapper(ren.decorations     , false)
tpt.hud                = fake_boolean_wrapper(ren.hud             , false)
tpt.newtonian_gravity  = fake_boolean_wrapper(sim.newtonianGravity, false)
ren.debugHUD           = fake_boolean_wrapper(ren.debugHud        , false)

function tpt.setdebug(flags)
	-- correct usage passed 0 to 1 arguments
	tpt.debug(flags or 0)
end

function sim.gravMap(x, y, ...)
	-- correct usage passed 2, 3, or 5 arguments
	if select("#", ...) == 0 then
		-- 2-argument calls are handled here
		local gx, gy = sim.gravityField(x, y)
		return math.sqrt(gx * gx + gy * gy)
	end
	-- sim.gravityMass is compatible with 3- and 5-argument calls
	sim.gravityMass(x, y, ...)
end

function tpt.set_wallmap(x, y, ...)
	-- correct usage passed 3, 5, or 7 arguments
	if select("#", ...) == 5 then
		-- 7-argument calls are handled here
		local w, h, fvx, fvy, v = ...
		sim.wallMap(x, y, w, h, v)
		sim.fanVelocityX(x, y, w, h, fvx)
		sim.fanVelocityY(x, y, w, h, fvy)
		return
	end
	-- sim.wallMap is compatible with 3- and 5-argument calls
	sim.wallMap(x, y, ...)
end

function tpt.get_wallmap(x, y)
	-- correct usage passed 0 to 2 arguments
	return sim.wallMap(x or 0, y or 0)
end

function tpt.set_elecmap(...)
	-- correct usage passed 0 to 5 arguments
	local x1, y1, width, height
	local argc = select("#", ...)
	if argc == 5 then
		-- only the 5-argument calls enabled area mode
		x1, y1, width, height = ...
	else
		x1, y1 = ...
		x1 = x1 or 0
		y1 = y1 or 0
		width, height = 1, 1
	end
	-- value was always the last argument
	local value = argc > 0 and select(argc, ...) or 0
	-- sim.elecMap is used with a 5-argument call
	sim.elecMap(x1, y1, width, height, value)
end

function tpt.get_elecmap(x, y)
	-- correct usage passed 0 to 2 arguments
	return sim.elecMap(x or 0, y or 0)
end

function tpt.set_pressure(x1, y1, width, height, value)
	-- correct usage passed 0 to 5 arguments
	return sim.pressure(x1 or 0, y1 or 0, width or sim.XCELLS, height or sim.YCELLS, value or 0)
end

function tpt.set_gravity(x1, y1, width, height, value)
	-- correct usage passed 0 to 5 arguments
	sim.gravityMass(x1 or 0, y1 or 0, width or sim.XCELLS, height or sim.YCELLS, value or 0)
end

function tpt.setwindowsize(scale, fullscreen)
	-- correct usage passed 0 to 2 arguments
	ui.windowSize(scale or 1, (fullscreen or 0) == 1)
end

function tpt.setfire(intensity)
	-- correct usage passed 0 to 1 arguments
	ren.fireSize(intensity or 1)
end

local old_ui_ctor
do
	local ctor_mt = {}
	function ctor_mt:__call(...)
		return self.new(...)
	end
	function ctor_mt:__newindex()
		error("table is read-only", 2)
	end
	function old_ui_ctor(func)
		return setmetatable({ new = function(_, ...)
			return func(...)
		end }, ctor_mt)
	end
end
Slider      = old_ui_ctor(ui.slider     )
Textbox     = old_ui_ctor(ui.textbox    )
ProgressBar = old_ui_ctor(ui.progressBar)
Window      = old_ui_ctor(ui.window     )
Button      = old_ui_ctor(ui.button     )
Label       = old_ui_ctor(ui.label      )
Checkbox    = old_ui_ctor(ui.checkbox   )

function tpt.register_step(f)
	event.register(event.tick, f)
end

function tpt.unregister_step(f)
	event.unregister(event.tick, f)
end

do
	local registered_mouseclicks = {}

	function tpt.register_mouseclick(f)
		if registered_mouseclicks[f] then return end
		local mousex = -1
		local mousey = -1
		local mousedown = -1
		local function mousedownfunc(x, y, button)
			--replicate hack in original function
			if button == 3 then
				button = 4
			end
			mousex = x
			mousey = y
			mousedown = button
			return f(x, y, button, 1, 0)
		end
		local function mouseupfunc(x, y, button, evt)
			--ignore automatic mouseup event sent when switching windows
			if mousedown == -1 and evt == 1 then
				return
			end
			--replicate hack in original function
			if button == 3 then
				button = 4
			end
			local evtType = 2
			if evt == 1 then
				evtType = 4
			elseif evt == 2 then
				evtType = 5
			end
			--zoom window cancel
			--Original function would have started returning 0 for mousetick events
			--(until the actual mousedown), but we don't replicate that here
			if evt ~= 2 then
				mousedown = -1
			end
			return f(x, y, button, evtType, 0)
		end
		local function mousemovefunc(x, y, dx, dy)
			mousex = x
			mousey = y
		end
		local function mousewheelfunc(x, y, d)
			return f(x, y, 0, 0, d)
		end
		local function tickfunc()
			if mousedown ~= -1 then
				return f(mousex, mousey, mousedown, 3, 0)
			end
		end
		event.register(event.mousedown, mousedownfunc)
		event.register(event.mouseup, mouseupfunc)
		event.register(event.mousemove, mousemovefunc)
		event.register(event.mousewheel, mousewheelfunc)
		event.register(event.tick, tickfunc)
		local funcs = {mousedownfunc, mouseupfunc, mousemovefunc, mousewheelfunc, tickfunc}
		registered_mouseclicks[f] = funcs
	end
	tpt.register_mouseevent = tpt.register_mouseclick

	function tpt.unregister_mouseclick(f)
		if not registered_mouseclicks[f] then return end

		local funcs = registered_mouseclicks[f]
		event.unregister(event.mousedown, funcs[1])
		event.unregister(event.mouseup, funcs[2])
		event.unregister(event.mousemove, funcs[3])
		event.unregister(event.mousewheel, funcs[4])
		event.unregister(event.tick, funcs[5])
		
		registered_mouseclicks[f] = nil
	end
	tpt.unregister_mouseevent = tpt.unregister_mouseclick
end

do
	local registered_keypresses = {}
	local keyMapping = {}
	-- lctrl, rctlr, lshift, rshift, lalt, ralt
	keyMapping[225] = 304
	keyMapping[229] = 303
	keyMapping[224] = 306
	keyMapping[228] = 305
	keyMapping[226] = 308
	keyMapping[230] = 307
	--up, down, right, left
	keyMapping[82] = 273
	keyMapping[81] = 274
	keyMapping[79] = 275
	keyMapping[80] = 276
	-- shift mapping for US keyboard layout
	local shiftMapping = {
		["`"] = "~",
		["1"] = "!",
		["2"] = "@",
		["3"] = "#",
		["4"] = "$",
		["5"] = "%",
		["6"] = "^",
		["7"] = "&",
		["8"] = "*",
		["9"] = "(",
		["0"] = ")",
		["-"] = "_",
		["="] = "+",

		["["] = "{",
		["]"] = "}",
		["\\"] = "|",
		[";"] = ":",
		["'"] = "\"",
		[","] = "<",
		["."] = ">",
		["/"] = "?"
	}

	function tpt.register_keypress(f)
		if registered_keypresses[f] then return end
		local function keypress(key, scan, rep, shift, ctrl, alt)
			if rep then return end
			local mod = event.getmodifiers()
			-- attempt to convert to string representation
			local err, keyStr = pcall(string.char, key)
			if not err then keyStr = "" end
			if keyStr ~= "" and shift then
				keyStr = shiftMapping[keyStr] and shiftMapping[keyStr] or string.upper(keyStr)
			end
			-- key mapping for common keys, extremely incomplete
			if keyMapping[scan] then key = keyMapping[scan] end
			return f(keyStr, key, mod, 1)
		end
		local function keyrelease(key, scan, rep, shift, ctrl, alt)
			local mod = event.getmodifiers()
			-- attempt to convert to string representation
			local err, keyStr = pcall(string.char, key)
			if not err then keyStr = "" end
			-- key mapping for common keys, extremely incomplete
			if keyMapping[scan] then key = keyMapping[scan] end
			return f(keyStr, key, mod, 2)
		end
		event.register(event.keypress, keypress)
		event.register(event.keyrelease, keyrelease)
		local funcs = { keypress, keyrelease }
		registered_keypresses[f] = funcs
	end
	tpt.register_keyevent = tpt.register_keypress

	function tpt.unregister_keypress(f)
		if not registered_keypresses[f] then return end
		local funcs = registered_keypresses[f]
		event.unregister(event.keypress, funcs[1])
		event.unregister(event.keyrelease, funcs[2])
		registered_keypresses[f] = nil
	end
	tpt.unregister_keyevent = tpt.unregister_keypress
end

function tpt.element_func(f, element, replace)
	if f == nil then f = false end
	elem.property(element, "Update", f, replace)
end

function tpt.graphics_func(f, element)
	if f == nil then f = false end
	elem.property(element, "Graphics", f)
end

function tpt.getscript(id, name, run)
	if not (id == 1 and name == "autorun.lua" and run == 1) then
		error("only use tpt.getscript to install the script manager")
	end
	tpt.installScriptManager()
end

tpt.drawpixel = gfx.drawPixel
function tpt.drawrect(x, y, w, h, r, g, b, a)
	gfx.drawRect(x, y, w + 1, h + 1, r, g, b, a)
end
function tpt.fillrect(x, y, w, h, r, g, b, a)
	gfx.fillRect(x + 1, y + 1, w - 1, h - 1, r, g, b, a)
end
tpt.drawline = gfx.drawLine
tpt.drawtext = gfx.drawText

function tpt.textwidth(str)
	local width = gfx.textSize(str)
	return width - 1
end

function tpt.toggle_pause()
	sim.paused(not sim.paused())
	return tpt.set_pause()
end

function tpt.watertest()
	sim.waterEqualization(not sim.waterEqualization())
	return sim.waterEqualization()
end

do
	local index = -1
	function tpt.start_getPartIndex()
		index = -1
	end
	function tpt.next_getPartIndex()
		while true do
			index = index + 1
			if index >= sim.MAX_PARTS then
				index = -1
				return false
			end
			if sim.partExists(index) then
				break
			end
		end
		return true
	end
	function tpt.getPartIndex()
		return index
	end
end

function tpt.get_numOfParts()
	return sim.NUM_PARTS
end

function tpt.element(thing)
	if type(thing) == "string" then
		local id = elem.getByName(thing)
		if id == -1 then
			error("Unrecognised element " .. thing, 2)
		end
		return id
	end
	return elem.property(thing, "Name")
end

function tpt.create(x, y, thing)
	if type(thing) ~= "number" then
		thing = tpt.element(thing or "dust")
	end
	return sim.partCreate(-1, x, y, thing)
end

function tpt.delete(x, y)
	if y then
		local id = sim.partID(x, y)
		if id then
			sim.partKill(id)
		end
		return
	end
	sim.partKill(x)
end

do
	local el_names = {
		[ "name"             ] = "Name"            ,
		[ "colour"           ] = "Colour"          ,
		[ "color"            ] = "Color"           ,
		[ "menu"             ] = "MenuVisible"     ,
		[ "menusection"      ] = "MenuSection"     ,
		[ "enabled"          ] = "Enabled"         ,
		[ "advection"        ] = "Advection"       ,
		[ "airdrag"          ] = "AirDrag"         ,
		[ "airloss"          ] = "AirLoss"         ,
		[ "loss"             ] = "Loss"            ,
		[ "collision"        ] = "Collision"       ,
		[ "gravity"          ] = "Gravity"         ,
		[ "newtoniangravity" ] = "NewtonianGravity",
		[ "diffusion"        ] = "Diffusion"       ,
		[ "hotair"           ] = "HotAir"          ,
		[ "falldown"         ] = "Falldown"        ,
		[ "flammable"        ] = "Flammable"       ,
		[ "explosive"        ] = "Explosive"       ,
		[ "meltable"         ] = "Meltable"        ,
		[ "hardness"         ] = "Hardness"        ,
		[ "weight"           ] = "Weight"          ,
		[ "heat"             ] = "Temperature"     ,
		[ "hconduct"         ] = "HeatConduct"     ,
		[ "description"      ] = "Description"     ,
		[ "state"            ] = "State"           ,
		[ "properties"       ] = "Properties"      ,
	}
	local el_mt = {}
	function el_mt:__index(key)
		return elem.property(self.id, el_names[key])
	end
	function el_mt:__newindex(key, value)
		elem.property(self.id, el_names[key], value)
	end

	local eltransition_names = {
		[ "presLowValue"  ] = "LowPressure"              ,
		[ "presLowType"   ] = "LowPressureTransition"    ,
		[ "presHighValue" ] = "HighPressure"             ,
		[ "presHighType"  ] = "HighPressureTransition"   ,
		[ "tempLowValue"  ] = "LowTemperature"           ,
		[ "tempLowType"   ] = "LowTemperatureTransition" ,
		[ "tempHighValue" ] = "HighTemperature"          ,
		[ "tempHighType"  ] = "HighTemperatureTransition",
	}
	local eltransition_mt = {}
	function eltransition_mt:__index(key)
		return elem.property(self.id, eltransition_names[key])
	end
	function eltransition_mt:__newindex(key, value)
		elem.property(self.id, eltransition_names[key], value)
	end

	tpt.el = {}
	tpt.eltransition = {}
	for i = 1, sim.PT_NUM do
		if elem.exists(i) then
			local name = elem.property(i, "Name"):lower()
			tpt.el          [name] = setmetatable({ id = i }, el_mt          )
			tpt.eltransition[name] = setmetatable({ id = i }, eltransition_mt)
		end
	end

	local part_mt = {}
	local last_id
	function part_mt:__index(key)
		if not sim.partExists(last_id) then
			error("dead particle", 2)
		end
		if key == "id" then
			return last_id
		end
		return sim.partProperty(last_id, key)
	end
	function part_mt:__newindex(key, value)
		if not sim.partExists(last_id) then
			error("dead particle", 2)
		end
		sim.partProperty(last_id, key, value)
	end
	local part_proxy = setmetatable({}, part_mt)

	local parts_mt = {}
	function parts_mt:__index(key)
		last_id = key
		return part_proxy
	end
	function parts_mt:__newindex()
		error("table is read-only", 2)
	end
	tpt.parts = setmetatable({}, parts_mt)
end

function tpt.set_property(prop, value, ...)
	if type(value) == "string" then
		value = tpt.element(value)
	end
	local argc = select("#", ...)
	local filter = argc > 0 and select(argc, ...)
	local have_filter = type(filter) == "string"
	if not have_filter then
		-- fast path for the common cases; the slow path covers these too though
		if argc == 1 then
			sim.partProperty(..., prop, value)
			return
		end
		if argc == 2 then
			local i = sim.partID(...)
			if i then
				sim.partProperty(i, prop, value)
			end
			return
		end
	end
	filter = have_filter and tpt.element(filter)
	do
		-- is this a region?
		local x, y, w, h = ...
		if type(x) ~= "number" or argc >= 4 then
			if argc < 4 then -- we're here because type(x) ~= "number", use default x, y, w, h
				x, y, w, h = 0, 0, sim.XRES, sim.YRES
			end
			for i in sim.parts() do
				local ix, iy = sim.partPosition(i)
				ix = math.floor(ix + 0.5)
				iy = math.floor(iy + 0.5)
				if ix >= x and iy >= y and ix < x + w and iy < y + h and (not filter or sim.partProperty(i, "type") == filter) then
					sim.partProperty(i, prop, value)
				end
			end
			return
		end
	end
	local x, y = ...
	local i
	if type(y) == "number" then
		i = sim.pmap(x, y)
		if i and filter and sim.partProperty(i, "type") ~= filter then
			i = nil
		end
		if not i then
			i = sim.photons(x, y)
			if i and filter and sim.partProperty(i, "type") ~= filter then
				i = nil
			end
		end
	else
		i = x
	end
	if i and filter and sim.partProperty(i, "type") ~= filter then
		i = nil
	end
	if i then
		sim.partProperty(i, prop, value)
	end
end

function tpt.get_property(prop, x, y)
	local i
	if type(y) == "number" then
		i = sim.partID(x, y)
	else
		i = sim.partExists(x) and x
	end
	if i then
		if prop == "id" then
			return i
		end
	else
		if prop == "type" then
			return 0
		end
	end
	return sim.partProperty(i, prop)
end

local sim_mt = {}
function sim_mt:__index(key)
	if key == "NUM_PARTS" then
		return sim.partCount()
	end
end
function sim_mt:__newindex(key)
	if key == "NUM_PARTS" then
		error("property is read-only", 2)
	end
	rawset(self, key, value)
end
setmetatable(sim, sim_mt)

local tpt_mt = {}
function tpt_mt:__index(key)
	if key == "selectedl" then
		return ui.activeTool(0)
	elseif key == "mousex" then
		local x, y = ui.mousePosition()
		return x
	elseif key == "mousey" then
		local x, y = ui.mousePosition()
		return y
	elseif key == "selectedr" then
		return ui.activeTool(1)
	elseif key == "selecteda" then
		return ui.activeTool(2)
	elseif key == "selectedreplace" then
		return ui.activeTool(3)
	elseif key == "brushID" then
		return ui.brushID()
	elseif key == "brushx" then
		local w, h = ui.brushRadius()
		return w
	elseif key == "brushy" then
		local w, h = ui.brushRadius()
		return h
	elseif key == "decoSpace" then
		return sim.decoSpace()
	end
end
function tpt_mt:__newindex(key, value)
	if key == "selectedl" then
		return ui.activeTool(0, value)
	elseif key == "mousex" then
		error("property is read-only", 2)
	elseif key == "mousey" then
		error("property is read-only", 2)
	elseif key == "selectedr" then
		return ui.activeTool(1, value)
	elseif key == "selecteda" then
		return ui.activeTool(2, value)
	elseif key == "selectedreplace" then
		return ui.activeTool(3, value)
	elseif key == "brushID" then
		return ui.brushID(value)
	elseif key == "brushx" then
		local w, h = ui.brushRadius()
		ui.brushRadius(value, h)
	elseif key == "brushy" then
		local w, h = ui.brushRadius()
		ui.brushRadius(w, value)
	elseif key == "decoSpace" then
		return sim.decoSpace(value)
	end
	rawset(self, key, value)
end
setmetatable(tpt, tpt_mt)
