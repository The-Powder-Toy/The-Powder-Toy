local char = "/"
if os.getenv("HOME") == nil then
	char = "\\"
end
local f = io.open("lua"..char.."mods.txt")
local table_size = 0
local mods = {}
local contents = f:read()
while true do
	if contents == nil then
		break
	end
	mods[table_size] = "lua"..char..contents
	dofile(mods[table_size])
	table_size = table_size + 1
	contents = f:read()
end
