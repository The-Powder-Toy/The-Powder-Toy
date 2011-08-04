tpt.set_console(0)

function mouseGrav()
	tpt.newtonian_gravity(1)
	tpt.set_pause(0)
	tpt.set_console(0)
        local x = mousex
	local y = mousey
	x = x/4
	y = y/4
	if key == ' ' then
		tpt.set_gravity(x,y,1,1,10)
	end

	return false
end
tpt.register_step(mouseGrav)