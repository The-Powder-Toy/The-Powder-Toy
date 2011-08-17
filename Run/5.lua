tpt.set_console(0)

function mouseGrav()
	tpt.newtonian_gravity(1)
	tpt.set_pause(0)
	tpt.set_console(0)
        local x = tpt.mousex
	local y = tpt.mousey
	x = x/4
	y = y/4
	tpt.set_gravity(x,y,1,1,10)
	return false
end
tpt.register_step(mouseGrav)