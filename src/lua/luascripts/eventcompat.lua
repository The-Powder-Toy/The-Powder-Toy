if not event then
	return
end

local deprecated_scripts = {}
local timer = nil
local function print_deprecation_warnings()
	if not timer or timer <= 0 then
		event.unregister(event.tick, print_deprecation_warnings)
	else
		timer = timer - 1
		return
	end

	local deprecated = {}
	for k,v in pairs(deprecated_scripts) do
		table.insert(deprecated, k)
	end
	local start_message = #deprecated == 1 and "This script is" or "These scripts are"
	print(start_message.." using a legacy event api and should be updated: ")
	print("\""..table.concat(deprecated, "\", \"").."\"")
end

local function deprecationwarning()
	-- no warning for now
	--[[local calling_file_info = debug.getinfo(3, "S")
	if calling_file_info then
		calling_file_info = calling_file_info["short_src"]

		if calling_file_info then
			deprecated_scripts[calling_file_info] = true
			if not timer then
				timer = 5
				event.register(event.tick, print_deprecation_warnings)
			end
		end
	end]]
end


function tpt.register_step(f)
	deprecationwarning()

	event.register(event.tick, f)
end

function tpt.unregister_step(f)
	deprecationwarning()

	event.unregister(event.tick, f)
end

local registered_mouseclicks = {}
function tpt.register_mouseclick(f)
	deprecationwarning()

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

local registered_keypresses = {}
function tpt.register_keypress(f)
	deprecationwarning()

	if registered_keypresses[f] then return end

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

	local function keypress(key, scan, rep, shift, ctrl, alt)
		if rep then return end
		local mod = event.getmodifiers()

		-- attempt to convert to string representation
		err, keyStr = pcall(string.char, key)
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
		err, keyStr = pcall(string.char, key)
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
