tpt.register_step("do_step")
numberthing = 0
increment = 2
function do_step()
	numberthing = numberthing + increment;
	if numberthing >= 400 then
		increment = -2
	elseif numberthing < 4 then
		increment = 2
	end
	tpt.drawtext(numberthing, 50, "Oh my god, this is amazing", 255, 255, 255, 255)
	tpt.drawtext(mousex, mousey, "Oh my god, this is amazing", 255, 255, 255, 255)
	tpt.reset_velocity(10, 10, 20, 20)
	tpt.reset_gravity_field(10, 10, 20, 20)
	tpt.set_pressure(10, 10, 20, 20)
	tpt.set_gravity(75, 45, 1, 1, 8)
	return false
end
