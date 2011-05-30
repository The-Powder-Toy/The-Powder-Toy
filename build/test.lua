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
end
