tpt.set_console(0)

function mouseGrav()
	tpt.newtonian_gravity(1)
	tpt.set_pause(0)
	tpt.set_console(0)
        local x = tpt.get_mousex()
	local y = tpt.get_mousey()
	tpt.set_gravity(x,y,1,1,0.1)
	return false
end
tpt.register_step(mouseGrav)